// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include "common.hpp"

const float pi = std::atan(1) * 4;

// unsigned int level_state;

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const Motion& motion1, const Motion& motion2)
{
	// calc 2 opposite corners of each object using position as center pt and adding half of the scale
	vec2 m1_bounding_box = get_bounding_box(motion1);
	vec2 m2_bounding_box = get_bounding_box(motion2);

	vec2 m1_top_left = motion1.position - m1_bounding_box / 2.f;
	vec2 m1_bot_right = motion1.position + m1_bounding_box / 2.f;
	vec2 m2_top_left = motion2.position - m2_bounding_box / 2.f;
	vec2 m2_bot_right = motion2.position + m2_bounding_box / 2.f;

	//check horizontal distance between them
	if (m1_top_left.x > m2_bot_right.x || m2_top_left.x > m1_bot_right.x) {
		return false;
	}

	//check vertical distance between them
	if (m1_bot_right.y < m2_top_left.y || m2_bot_right.y < m1_top_left.y) {
		return false;
	}
	return true;
}

void PhysicsSystem::step(float elapsed_ms, float window_width_px, float window_height_px)
{
	// Move entities based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
    float step_seconds = 1.0f * (elapsed_ms / 1000.f);
	if (level_state == LEVEL_STATE_SELECTOR) {
		auto& motion_registry = registry.motions;
		for (uint i = 0; i < motion_registry.size(); i++)
		{
			// !!! TODO PHYSICS: update motion.position based on step_seconds and motion.velocity
			Motion& motion = motion_registry.components[i];
			Entity entity = motion_registry.entities[i];


			//Including gravity
			if (registry.gravities.has(entity)) {
				motion.velocity += step_seconds * vec2(0.f, GRAVITY_ACCEL);
			}
			motion.velocity += step_seconds * motion.acceleration;
			motion.position += step_seconds * motion.velocity;
		}
	}
	else if (level_state == LEVEL_STATE_COMBAT) {
		auto& motion_registry = registry.motions;
		for (uint i = 0; i < motion_registry.size(); i++)
		{
			// !!! TODO PHYSICS: update motion.position based on step_seconds and motion.velocity
			Motion& motion = motion_registry.components[i];
			Entity entity = motion_registry.entities[i];

			motion.position = motion.position;

		}
	}

    /*for (Entity emitter_entity : registry.emitters.entities) {
        auto& particle_motions = registry.particleMotions.get(emitter_entity);
        auto& emitter = registry.emitters.get(emitter_entity);

        if (emitter.lifetime > 0) {
            for (uint j = 0; j < emitter.num_particles; j++) {
                particle_motions.pos_x[j] += 100.f * std::cos(2 * pi / emitter.num_particles * j) * elapsed_ms / 1000;
                particle_motions.pos_y[j] += 100.f * std::sin(2 * pi / emitter.num_particles * j) * elapsed_ms / 1000;
            }

            emitter.lifetime -= elapsed_ms / 1000;
        }
        else {
            registry.remove_all_components_of(emitter_entity);
        }
    }*/


	// Check for collisions between all moving entities
    ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i<motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		for(uint j = 0; j<motion_container.components.size(); j++) // i+1
		{
			if (i == j)
				continue;

			Motion& motion_j = motion_container.components[j];
			Entity entity_j = motion_container.entities[j];

			if (collides(motion_i, motion_j) && !(registry.mouses.has(entity_j) || registry.mouses.has(entity_i)))
			{

				//Notify both of the entities of the collision

				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);

			}
		}
	}
	//TODO Wall Collisions

	// you may need the following quantities to compute wall positions
	(float)window_width_px; (float)window_height_px;

	/***************************************************************************************
	TEMP COLLISION CODE FOR TESTING MOVEMENT BELOW
	****************************************************************************************/
    ComponentContainer<Player> &player_container = registry.players;
    Entity player = player_container.entities[0];
    Motion& player_motion = registry.motions.get(player);

    Transform transform;
    transform.translate(vec3(player_motion.position.x, player_motion.position.y, 1.0));
    transform.rotate(player_motion.angle);
    transform.scale(player_motion.scale);

    auto& meshes = registry.meshPtrs.get(player);
    uint player_vertices = meshes->vertices.size();
    vec2 bounding_box = get_bounding_box(player_motion);


    auto& collisionsRegistry = registry.collisions;
    for (Entity entity : registry.solid_platforms.entities) {
        Motion& solid_motion = registry.motions.get(entity);

        vec2 right_line_pos = {solid_motion.position.x + solid_motion.scale.x/2, solid_motion.position.y};
        vec2 left_line_pos = {solid_motion.position.x - solid_motion.scale.x/2, solid_motion.position.y};
        vec2 top_line_pos = {solid_motion.position.x, solid_motion.position.y - solid_motion.scale.y/2};
        vec2 bottom_line_pos = {solid_motion.position.x, solid_motion.position.y + solid_motion.scale.y/2};

        for (uint i=0; i< player_vertices; i = i+48) {
            vec3 meshPosition = meshes->vertices.at(i).position;
            vec3 worldPosition = transform.mat * meshPosition;
            vec2 worldPos2D = {worldPosition.x + player_motion.position.x,
                               worldPosition.y + player_motion.position.y};

            bool right = worldPos2D.x > solid_motion.position.x + solid_motion.scale.x/2;
            bool left = worldPos2D.x < solid_motion.position.x - solid_motion.scale.x/2;
            bool above = worldPos2D.y < solid_motion.position.y - solid_motion.scale.y/2;
            bool below = worldPos2D.y > solid_motion.position.y + solid_motion.scale.y/2;
            bool within = !right && !left && !below && !above;


            float distanceB = abs(worldPos2D.x - solid_motion.position.x)
                                + abs(worldPos2D.y - (solid_motion.position.y + solid_motion.scale.y/2));
            float distanceT = abs(worldPos2D.x - solid_motion.position.x)
                               + abs(worldPos2D.y - (solid_motion.position.y - solid_motion.scale.y/2));
            float distanceL = abs(worldPos2D.x - (solid_motion.position.x - solid_motion.scale.x/2))
                               + abs(worldPos2D.y - solid_motion.position.y);
            float distanceR = abs(worldPos2D.x - (solid_motion.position.x + solid_motion.scale.x/2))
                               + abs(worldPos2D.y - solid_motion.position.y);

            float shortestD = min(distanceT, distanceL);
            shortestD = min(shortestD, distanceR);
            shortestD = min(shortestD, distanceB);

            if (solid_motion.angle != 0.f) {
                if (worldPos2D.x < 0) {
                    player_motion.position.x = -player_motion.scale.x / 2;
                } else if (player_motion.position.y > window_height_px + player_motion.scale.y/2) {
                    player_motion.velocity.y = 0;
                    player_motion.position.y = window_height_px + player_motion.scale.y/2;
                }
                else if (within && shortestD == distanceT &&
                        player_motion.position.x >= solid_motion.position.x - solid_motion.scale.x/2 - 16 &&
                        player_motion.position.x <= solid_motion.position.x + solid_motion.scale.x/2 + 12.5) {
                    player_motion.velocity.y = 0;
                    player_motion.position.y = solid_motion.position.y - solid_motion.scale.y/2 + player_motion.scale.y / 2;
                }
                else if (within && shortestD == distanceB) {
                    player_motion.velocity.y = -player_motion.velocity.y/2;
                    player_motion.position.y += 1.0f;
                }
                else if (within && shortestD == distanceL &&
                        player_motion.position.y <= solid_motion.position.y + solid_motion.scale.y/2 - player_motion.scale.y / 2) {
                    player_motion.velocity.x = 0;
                    player_motion.position.x -= 1.f;
                }
                else if (within && shortestD == distanceR &&
                        player_motion.position.y <= solid_motion.position.y + solid_motion.scale.y/2 - player_motion.scale.y / 2) {
                    player_motion.velocity.x = 0;
                    player_motion.position.x += 1.f;
                }
            }

        }
    }


	/***************************************************************************************
	TEMP COLLISION CODE FOR TESTING MOVEMENT ABOVE
	****************************************************************************************/

	//TODO MESH Collision Debug

    ComponentContainer<DebugComponent> &debug_container = registry.debugComponents;

	// debugging of bounding boxes
	if (debugging.in_debug_mode)
	{
		Entity player_entity = registry.players.entities[0];
		Motion& player_motion = registry.motions.get(player_entity);

        Transform transform;
        transform.translate(vec3(player_motion.position.x, player_motion.position.y, 1.0));
        transform.rotate(player_motion.angle);
        transform.scale(player_motion.scale);

        auto& meshes = registry.meshPtrs.get(player_entity);
        uint player_vertices = meshes -> vertices.size();
        for (uint i = 0; i < player_vertices; i = i+500) {
            vec3 meshPosition = meshes->vertices.at(i).position;
            vec3 worldPosition = transform.mat *  meshPosition;
            vec2 worldPos2D = {worldPosition.x + player_motion.position.x, worldPosition.y + player_motion.position.y};

            vec2 line_scale = {5, 5};
            createLine(worldPos2D, line_scale);
        }
        
		uint size_before_adding_new = (uint)motion_container.components.size();
		for (uint i = 0; i < size_before_adding_new; i++)
		{
			Motion& motion_i = motion_container.components[i];
			Entity entity_i = motion_container.entities[i];

            if (!debug_container.has(entity_i)) {
                if (!registry.backgrounds.has(entity_i)) {

                    // visualize the radius with two axis-aligned lines
                    const vec2 bonding_box = get_bounding_box(motion_i);
                    vec2 line_scale1 = {bonding_box.x, 5};
                    createLine({motion_i.position.x, motion_i.position.y + bonding_box.y / 2 }, line_scale1);
                    createLine({motion_i.position.x, motion_i.position.y - bonding_box.y / 2 }, line_scale1);
                    vec2 line_scale2 = {5, bonding_box.y};
                    createLine({motion_i.position.x + bonding_box.x / 2 , motion_i.position.y}, line_scale2);
                    createLine({motion_i.position.x - bonding_box.x / 2 , motion_i.position.y}, line_scale2);
                }
            }
		}
	}

}
