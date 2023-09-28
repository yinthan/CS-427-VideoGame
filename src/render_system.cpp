// internal
#include "render_system.hpp"
#include <SDL.h>

#include "tiny_ecs_registry.hpp"
#include "tiny_ecs.hpp"
#include "common.hpp"



void RenderSystem::drawTexturedMesh(Entity entity,
									const mat3 &projection)
{
	Motion &motion = registry.motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position);
	transform.rotate(motion.angle);
	transform.scale(motion.scale);


	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED || render_request.used_effect == EFFECT_ASSET_ID::ANIMATION || render_request.used_effect == EFFECT_ASSET_ID::VACCINE || render_request.used_effect == EFFECT_ASSET_ID::SICKMAN || render_request.used_effect == EFFECT_ASSET_ID::FIREBALL)
	{
		if (render_request.used_effect == EFFECT_ASSET_ID::ANIMATION) {
			//Set frame rate
			GLuint frame_uloc = glGetUniformLocation(program, "frame");
			int frameTimer = (glfwGetTime() * 10.0f);
			int number_of_frames = render_request.num_frames;
			initAnimation(number_of_frames, 1);
			//printf("%f\n", frameTimer % 2 / 2.0f);
			glUniform1f(frame_uloc, frameTimer % number_of_frames / (float)number_of_frames);
			gl_has_errors();
		}
		


		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position
		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();

        GLint time_uloc = glGetUniformLocation(program, "time");
            glUniform1f(time_uloc, (float) (glfwGetTime()));
            gl_has_errors();



        if (render_request.used_effect == EFFECT_ASSET_ID::SICKMAN) {
            ComponentContainer<Sickman> sickmen = registry.sickmen;
            GLint move_uloc = glGetUniformLocation(program, "move");
            int i = 0;
            if (sickmen.components.size() > 0) {
                if (!sickmen.components[0].sick) {
                    i = 2;
                } else if (sickmen.components[0].kick) {
                    i = 1;
                } else if (sickmen.components[0].fire) {
					i = 3;
				} else if (sickmen.components[0].dead) {
					i = 4;
				}
            }
            glUniform1i(move_uloc, i);
            gl_has_errors();
			float width = 1200.f;
			float height = 800.f;
			GLint size_uloc = glGetUniformLocation(program, "widtheight");
			glUniform2f(size_uloc, width, height);
            gl_has_errors();
        }
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::LINE || render_request.used_effect == EFFECT_ASSET_ID::PLAYER)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)sizeof(vec3));
		gl_has_errors();

        GLint time_uloc = glGetUniformLocation(program, "time");
        glUniform1f(time_uloc, (float) (glfwGetTime()));
        gl_has_errors();

	}
	//TODO GRAPHICS
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

// draw the intermediate texture to the screen, with some distortion to simulate
// water
//TODO Remove water and add SKY
void RenderSystem::drawToScreen()
{
	// Setting shaders
	// get the water texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::SCREEN]);
	gl_has_errors();
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	

	const GLuint screen_program = effects[(GLuint)EFFECT_ASSET_ID::SCREEN];
	// Set clock
	GLuint time_uloc = glGetUniformLocation(screen_program, "time");
	GLuint dead_timer_uloc = glGetUniformLocation(screen_program, "darken_screen_factor");
	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	ScreenState &screen = registry.screenStates.get(screen_state_entity);
	glUniform1f(dead_timer_uloc, screen.darken_screen_factor);
	gl_has_errors();
	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(screen_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();


	
	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();
	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();

	// Clearing backbuffer
	/*if (level_state == LEVEL_STATE_SELECTOR)
		glViewport(x, 0, w, h);
	else*/
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	glClearColor(0, 0, 1, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();
	mat3 projection_2D = createProjectionMatrix();
    // Render the backgrounds
    for (Entity entity: registry.backgrounds.entities) {
        drawTexturedMesh(entity, projection_2D);
    }

	// Draw all textured meshes that have a position and size component
	for (Entity entity : registry.renderRequests.entities)
	{
		if (!registry.motions.has(entity) || registry.backgrounds.has(entity)
			|| registry.helpComponent.has(entity) || registry.storyComponents.has(entity))
			continue;
		// Note, its not very efficient to access elements indirectly via the entity
		// albeit iterating through all Sprites in sequence. A good point to optimize
		drawTexturedMesh(entity, projection_2D);
	}

    for (Entity entity : registry.renderRequests.entities) {
        if (registry.storyComponents.has(entity)) {
            drawTexturedMesh(entity, projection_2D);
        }
    }

    for (Entity entity : registry.renderRequests.entities) {
        if (registry.helpComponent.has(entity)) {
            drawTexturedMesh(entity, projection_2D);
        }
    }

	/*for (Entity entity : registry.emitters.entities) {
		drawParticles(entity, projection_2D);
	}*/

	// Truely render to the screen
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

mat3 RenderSystem::createProjectionMatrix()
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	gl_has_errors();

	float right = (float)w / screen_scale;
	float bottom = (float)h / screen_scale;

	// Get player position in world coordinates
	assert(registry.players.entities.size() > 0);
	Entity& player = registry.players.entities[0];
	Motion& motion = registry.motions.get(player);
	vec2 pos = motion.position;

	float tx, ty;
	float slope = (-3.f - (-1.f)) / (1.5f * right - 0.5f * right);

	// Find coordinates for viewport
	if (pos.x > (right / 2) && pos.x < (2 * right) - (right / 2)) {
		tx = slope * pos.x;
	}
	else if (pos.x <= (right / 2)) {
		tx = -1.f;
	}
	else if (pos.x >= (2 * right) - (right / 2)) {
		tx = -3.f;
	}

	ty = -(top + bottom) / (top - bottom);
	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

void RenderSystem::addBackground(
        int layer, RenderSystem *renderer, TEXTURE_ASSET_ID textureAssetId, int game_w,
        int game_h, int pos_x, int pos_y, vec2 scale) {

    // Reserve en entity
    auto entity = Entity();

    // Store a reference to the potentially re-used mesh object
    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    registry.meshPtrs.emplace(entity, &mesh);

    // Initialize the position, scale, and physics components
    auto& motion = registry.motions.emplace(entity);
    motion.angle = 0.f;
    motion.velocity = { 0, 0 };
    motion.position = {pos_x, pos_y};
    motion.scale = scale;

    // Setting initial values, scale is negative to make it face the opposite way
    auto& background = registry.backgrounds.emplace(entity);
//    background.bg_x = game_w/2;
//    background.bg_y = game_h/2;
    background.layer = layer;
    registry.renderRequests.insert(
            entity,
            { textureAssetId,
              EFFECT_ASSET_ID::TEXTURED,
              GEOMETRY_BUFFER_ID::SPRITE });
}

void RenderSystem::updateBackgrounds(float time_ms, int game_w, int game_h) {
//    Motion& motion = registry.motions.get(bg_entity);
//    motion.position.x -= scroll_speed;
//    if (motion.position.x < -1000) {
//        motion.position.x = 0;
//    }
    Entity& player = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player);

    // TODO: Add condition to check for boundaries (do after initial implementation is working)
    for (auto& bg : registry.backgrounds.entities) {
		if (level_state == LEVEL_STATE_SELECTOR) {
			auto& motion = registry.motions.get(bg);
			auto& bg_component = registry.backgrounds.get(bg);
			if (bg_component.layer >= 2 && player_motion.position.x > (game_w / 2) && player_motion.position.x < (2 * game_w) - (game_w / 2)) {
				motion.position.x += player_motion.velocity.x * time_ms / 1000 / (bg_component.layer * 2);
			}
		}
    }
}

void RenderSystem::removeBackgrounds() {
    while (registry.backgrounds.entities.size() > 0) {
        registry.remove_all_components_of(registry.backgrounds.entities.back());
    }
}

//void RenderSystem::drawParticles(Entity entity, const mat3& projection) {
//	auto& emitter = registry.emitters.get(entity);
//	auto& motions = registry.particleMotions.get(entity);
//
//	Transform transform;
//	transform.translate(emitter.pos);
//	transform.rotate(0.f);
//	transform.scale(vec2(motions.scales[0], motions.scales[0]));
//
//	const GLuint used_effect_enum = (GLuint)EFFECT_ASSET_ID::PARTICLE;
//	const GLuint program = (GLuint)effects[used_effect_enum];
//
//	// Setting shaders
//	glUseProgram(program);
//	gl_has_errors();
//
//	const GLuint vbo = vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::PARTICLE];
//	const GLuint ibo = index_buffers[(GLuint)GEOMETRY_BUFFER_ID::PARTICLE];
//
//	// Setting vertex and index buffers
//	glBindBuffer(GL_ARRAY_BUFFER, vbo);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
//	gl_has_errors();
//
//	GLint in_position_loc = glGetAttribLocation(program, "in_position");
//	//GLint in_color_loc = glGetAttribLocation(program, "color");
//	gl_has_errors();
//
//	glEnableVertexAttribArray(in_position_loc);
//	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
//		sizeof(ColoredVertex), (void*)0);
//	gl_has_errors();
//
//	/*glEnableVertexAttribArray(in_color_loc);
//	glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
//		sizeof(ColoredVertex), (void*)sizeof(vec3));
//	gl_has_errors();*/
//
//	GLint color_uloc = glGetUniformLocation(program, "color");
//	const vec3 color = vec3(1, 0, 0);
//	glUniform3fv(color_uloc, 1, (float*)&color);
//	gl_has_errors();
//
//	//// Get number of indices from index buffer, which has elements uint16_t
//	//GLint size = 0;
//	//glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
//	//gl_has_errors();
//
//	//GLsizei num_indices = size / sizeof(uint16_t);
//	//// GLsizei num_triangles = num_indices / 3;
//
//	GLint currProgram;
//	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
//	// Setting uniform values to the currently bound program
//	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
//	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.mat);
//	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
//	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
//	GLuint pos_x_loc = glGetUniformLocation(currProgram, "pos_x");
//	glUniform1fv(pos_x_loc, 100, (float*)motions.pos_x.data());
//	GLuint pos_y_loc = glGetUniformLocation(currProgram, "pos_y");
//	glUniform1fv(pos_y_loc, 100, (float*)motions.pos_y.data());
//	GLuint scale_loc = glGetUniformLocation(currProgram, "scale");
//	glUniform1fv(scale_loc, 100, (float*)motions.scales.data());
//	gl_has_errors();
//
//	//glVertexAttribDivisor(0, 0); // base mesh
//	//glVertexAttribDivisor(1, 1); // position and size
//	//glVertexAttribDivisor(2, 1); // color
//
//	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 100);
//}

//Text rendering

void RenderSystem::RenderText(std::string text, float x, float y, float scale, glm::vec3 color)
{
	/*
	*/
}
