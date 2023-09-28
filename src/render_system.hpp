// Source for freetype https://freetype.org/freetype2/docs/tutorial/index.html
// Source for implementing free type https://learnopengl.com/In-Practice/Text-Rendering

#pragma once

#include <array>
#include <utility>
#include <iostream>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

#include <ft2build.h>
#include FT_FREETYPE_H  

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	// TODO GRAPHICS
	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		   std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::PLAYER, mesh_path("medic.obj"))
		  // specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {

            textures_path("enemy/virus-idle-sprite-sheet.png"),
			textures_path("enemy/bacteria-idle-sprite-sheet.png"),
			textures_path("enemy/fungus-idle-sprite-sheet.png"),
			textures_path("item.png"),
            textures_path("1.png"),
            textures_path("backgrounds/city.png"),
            textures_path("backgrounds/mountain.png"),
            textures_path("backgrounds/sky.jpg"),
	        textures_path("backgrounds/combat_background.jpg"),
    
            textures_path("platform.png"),
            textures_path("help.png"),
            textures_path("vaccine.png"),
			textures_path("fireball.png"),
	        textures_path("buttons/battle_menu_main.png"),
	        textures_path("buttons/fight.png"),
	        textures_path("buttons/allies.png"),
	        textures_path("buttons/items.png"),
	        textures_path("buttons/run.png"),
	        textures_path("buttons/fight_menu_main.png"),
	        textures_path("buttons/punch.png"),
            textures_path("buttons/shoot.png"),
			textures_path("buttons/heat.png"),
	        textures_path("buttons/back.png"),
            textures_path("story/story_bl.png"),
            textures_path("story/story_br.png"),
            textures_path("story/story_far_right.png"),
            textures_path("story/story_start.png"),
            textures_path("story/story_tl.png"),
            textures_path("story/story_tr.png")
  };


	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
        shader_path("player"),
		shader_path("line"),
		shader_path("textured"),
		shader_path("animation"),
		shader_path("screen"),
        shader_path("vaccine"),
        shader_path("sickman"),
		shader_path("fireball"),
		shader_path("particle")};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(int width, int height, GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();
	void initAnimation(int cols, int rows);

	
	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the water
	// shader
	bool initScreenTexture();
	void initParticles();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	mat3 createProjectionMatrix();

    // parallax scrolling methods
    void addBackground(int layer, RenderSystem* renderer,
                       TEXTURE_ASSET_ID textureAssetId, int game_w,
                       int game_h, int pos_x, int pos_y, vec2 scale);
    void removeBackgrounds();
    void updateBackgrounds(float time_ms, int game_w, int game_h);

	//Freetype text methods

	struct Character {
		unsigned int TextureID;  // ID handle of the glyph texture
		glm::ivec2   Size;       // Size of glyph
		glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
		unsigned int Advance;    // Offset to advance to next glyph
	};

	std::unordered_map<GLchar, Character> Characters;
	void initializeFTCharacters();
	void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);

private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawToScreen();
	void drawParticles(Entity entity, const mat3& projection);

	// Window handle
	GLFWwindow* window;
	float screen_scale;  // Screen to pixel coordinates scale factor (for apple
						 // retina display?)

	bool render_particles;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
