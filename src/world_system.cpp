// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <chrono>

#include "physics_system.hpp"
#include "common.hpp"
#include "components.hpp"

// Game configuration
// TODO game configuration
unsigned int level_state;
unsigned int current_level = 1;
unsigned int save_state;
uint global_num =  0;

const size_t MAX_VIRUS = 5;
const size_t DELAY_MS = 20;

using Clock = std::chrono::high_resolution_clock;
Story story;
auto wait_start = Clock::now();

bool story_start_closed = 0;
bool tutorial_closed = 0;
WorldSystem::WorldSystem()
	: points(0)
	, next_virus_spawn(0.f){
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
    virus_positions_cache = {};
    pathogen_types = {};
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (player_dead_sound != nullptr)
		Mix_FreeChunk(player_dead_sound);
	if (player_get_item_sound != nullptr)
		Mix_FreeChunk(player_get_item_sound);
    if (player_attack_sound != nullptr)
        Mix_FreeChunk(player_attack_sound);
    if (enemy_attack_sound != nullptr)
        Mix_FreeChunk(enemy_attack_sound);
    if (defend_sound != nullptr)
        Mix_FreeChunk(defend_sound);
    if (heal_sound != nullptr)
        Mix_FreeChunk(heal_sound);
    if (jump_sound != nullptr)
        Mix_FreeChunk(jump_sound);
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char* desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window(int width, int height) {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(width, height, "Avoid The Virus", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_input(_0, _1, _2); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	player_dead_sound = Mix_LoadWAV(audio_path("player_dead.wav").c_str());
	player_get_item_sound = Mix_LoadWAV(audio_path("item_pickup1.wav").c_str());
    player_attack_sound = Mix_LoadWAV(audio_path("player_attack.wav").c_str());
    enemy_attack_sound = Mix_LoadWAV(audio_path("enemy_attack.wav").c_str());
    defend_sound = Mix_LoadWAV(audio_path("defend.wav").c_str());
    heal_sound = Mix_LoadWAV(audio_path("heal.wav").c_str());
    jump_sound = Mix_LoadWAV(audio_path("jump.wav").c_str());

	if (background_music == nullptr || player_dead_sound == nullptr || player_get_item_sound == nullptr || player_attack_sound == nullptr
        || enemy_attack_sound == nullptr || defend_sound == nullptr || heal_sound == nullptr || jump_sound == nullptr) {

		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n %s\n %s\n %s\n %s\n %s\n make sure the data directory is present\n",
			audio_path("music.wav").c_str(),
			audio_path("player_dead.wav").c_str(),
			audio_path("item_pickup1.wav").c_str(),
            audio_path("player_attack.wav").c_str(),
            audio_path("enemy_attack.wav").c_str(),
            audio_path("defend.wav").c_str(),
            audio_path("heal.wav").c_str(),
            audio_path("jump.wav").c_str());
		return nullptr;
	}


	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg, int window_width_px, int window_height_px) {
	this->renderer = renderer_arg;

	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);

	this->game_w = window_width_px;
	this->game_h = window_height_px;

    fprintf(stderr, "Loaded music\n"); 
	// Set all states to default
	restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Get the screen dimensions
	int screen_width, screen_height;
	glfwGetFramebufferSize(window, &screen_width, &screen_height);

	// Updating window title with points
	std::stringstream title_ss;
	title_ss << "Points: " << points;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	//TODO screen state
	assert(registry.screenStates.components.size() <= 1);
	ScreenState& screen = registry.screenStates.components[0];

	float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < min_counter_ms) {
			min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.deathTimers.remove(entity);
			screen.darken_screen_factor = 0;
			restart_game();
			return true;
		}
	}

    resolveCombat();
    updateCombatState();

    return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;

	// TODO Remove all entities that we created

	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

	std::ifstream i(std::string(PROJECT_SOURCE_DIR) + "/src/levels/level" + std::to_string(current_level) + ".json");
	j = json::parse(i);
    printf("save_state: %i\n", save_state);
    if (save_state == 0) {
        player_position = getPlayerPosition(j, game_w, game_h);
    } else {
        std::ifstream i2(std::string(PROJECT_SOURCE_DIR) + "src/save_states/state" + std::to_string(save_state) + ".json");
        auto save_j = json::parse(i2);
        player_position = getPlayerPosition(save_j, game_w, game_h);
        printf("State %i loaded\n", save_state);
        printf("Player position: x: %f, y: %f\n", player_position.x, player_position.y);
    }

	// player_position = { 100, game_h - abs(PLAYER_BB_HEIGHT) / 2 };
	createPlayer(renderer, player_position);


	UI = createUI();

	mouse = createMouse();

    // saveState(1,1);

	// TODO create players and enemies when the world resets.
	// std::vector<vec2> virus_positions = getVirusPositions(j, game_w, game_h);
	// std::vector<vec2>::iterator iter;
    // printf("virus positions size: %i\n", virus_positions.size());
	// for (iter = virus_positions.begin(); iter != virus_positions.end(); iter++) {
	// 	createVirus(renderer, *iter);
	// }
	//createVirus(renderer, { 300, 600 });

	//registry.colors.insert(player, { 1, 0.8f, 0.8f });
    std::vector<vec2> virus_positions = getVirusPositions(j, game_w, game_h);
    std::vector<vec2>::iterator iter_v;
    for (iter_v = virus_positions.begin(); iter_v != virus_positions.end(); iter_v++) {
        //createVirus(renderer, *iter_v, TEXTURE_ASSET_ID::VIRUS, false);
        virus_positions_cache.push_back(*iter_v);
        pathogen_types.push_back(TEXTURE_ASSET_ID::VIRUS);
    }
    std::vector<vec2> bacteria_positions = getBacteriaPositions(j, game_w, game_h);
    std::vector<vec2>::iterator iter_b;
    for (iter_b = bacteria_positions.begin(); iter_b != bacteria_positions.end(); iter_b++) {
        //createVirus(renderer, *iter_b, TEXTURE_ASSET_ID::BACTERIA, false);
        virus_positions_cache.push_back(*iter_b);
        pathogen_types.push_back(TEXTURE_ASSET_ID::BACTERIA);
    }
    std::vector<vec2> fungus_positions = getFungusPositions(j, game_w, game_h);
    std::vector<vec2>::iterator iter_f;
    for (iter_f = fungus_positions.begin(); iter_f != fungus_positions.end(); iter_f++) {
        //createVirus(renderer, *iter_f, TEXTURE_ASSET_ID::FUNGUS, false);
        virus_positions_cache.push_back(*iter_f);
        pathogen_types.push_back(TEXTURE_ASSET_ID::FUNGUS);
    }

    changeState(LEVEL_STATE_SELECTOR, false, Sickman::PATHOGEN_TYPE::NA, Entity());
//    changeState(LEVEL_STATE_COMBAT, false, Sickman::PATHOGEN_TYPE::VIRUS, Entity());
//    changeState(LEVEL_STATE_SELECTOR, false, Sickman::PATHOGEN_TYPE::NA, Entity());

    //toggle_help();
    createStoryBox(vec2(game_w / 2, game_h / 2), vec2{game_w, game_h}, TEXTURE_ASSET_ID::STORY_START);
    
}

void WorldSystem::updateUI()
{
	//Delete all menuButtons
	while (registry.menuButtons.size() > 0) {
		registry.remove_all_components_of(registry.menuButtons.entities.back());
	}
	

	for (uint i = 0; i < registry.uis.size(); i++) {
		auto state = registry.uis.components[i].current_state;
		float remaining_width = this->game_w - (BUTTON_MAIN_BB_WIDTH + 2 * BUTTON_SELECT_BB_WIDTH);
		remaining_width = std::max(0.f, remaining_width);
		remaining_width /= 2;


		vec2 pos0 = vec2(0 + BUTTON_MAIN_BB_WIDTH / 2.f, 0 + BUTTON_MAIN_BB_HEIGHT / 2.f) + vec2(remaining_width,0);
		vec2 pos1 = vec2(BUTTON_MAIN_BB_WIDTH + BUTTON_SELECT_BB_WIDTH / 2, BUTTON_SELECT_BB_HEIGHT / 2.f) + vec2(remaining_width, 0);
		vec2 pos2 = vec2(BUTTON_MAIN_BB_WIDTH + 3 * BUTTON_SELECT_BB_WIDTH / 2, BUTTON_SELECT_BB_HEIGHT / 2.f) + vec2(remaining_width, 0);
		vec2 pos3 = vec2(BUTTON_MAIN_BB_WIDTH + BUTTON_SELECT_BB_WIDTH / 2, 3 * BUTTON_SELECT_BB_HEIGHT / 2.f) + vec2(remaining_width, 0);
		vec2 pos4 = vec2(BUTTON_MAIN_BB_WIDTH + 3 * BUTTON_SELECT_BB_WIDTH / 2, 3 * BUTTON_SELECT_BB_HEIGHT / 2.f) + vec2(remaining_width, 0);

		switch (state) {
		case Ui::State::NONE:
			break;
		case Ui::State::MAIN:
			createButton(renderer, pos0, MenuButton::Type::MAIN);
			createButton(renderer, pos1, MenuButton::Type::FIGHT);
			createButton(renderer, pos2, MenuButton::Type::ALLIES);
			createButton(renderer, pos3, MenuButton::Type::ITEMS);
			createButton(renderer, pos4, MenuButton::Type::RUN);
			break;
		case Ui::State::FIGHT:
			createButton(renderer, pos0, MenuButton::Type::MAIN_ATTACK);
			createButton(renderer, pos1, MenuButton::Type::PUNCH);
			createButton(renderer, pos2, MenuButton::Type::SHOOT);
			createButton(renderer, pos3, MenuButton::Type::HEAT);
			createButton(renderer, pos4, MenuButton::Type::BACK);
			break;
		default:
			createButton(renderer, pos0, MenuButton::Type::MAIN);
			createButton(renderer, pos1, MenuButton::Type::BACK);
			createButton(renderer, pos2, MenuButton::Type::BACK);
			createButton(renderer, pos3, MenuButton::Type::BACK);
			createButton(renderer, pos4, MenuButton::Type::BACK);
            break;
		}

	}

}
void WorldSystem::removePlatforms()
{
	while (registry.solid_platforms.entities.size() > 0)
		registry.remove_all_components_of(registry.solid_platforms.entities.back());
}


// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

/****************************************************************************************
* Below Code contains Observer Patterns
*****************************************************************************************/
void WorldSystem::onNotify(Event event) {
    if (event.type == Event::COLLISION) {
        //handle_collisions(event.entity, event.entity_other);
    }
}
void WorldSystem::animate()
{
	auto& motion_registry = registry.motions;

	//Iterate through all players
	for (uint i = 0; i < registry.players.size(); i++) {
		// !!! TODO: Decouple the motion component into smaller pieces
		Entity entity_i = registry.players.entities[i];
		Motion& motion_i = motion_registry.get(entity_i);
				
		switch (motion_i.current_state) {
			case Motion::State::Idle:
				break;
			case Motion::State::Crouch:
				break;
			case Motion::State::Jump:
				break;
			case Motion::State::Climb:
				break;
		}
		
	}
	
}
	// Compute collisions between entities
        void WorldSystem::handle_collisions() {
        // Loop over all collisions detected by the physics system
        auto &collisionsRegistry = registry.collisions;
        for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
            // The entity and its collider

            Entity entity = collisionsRegistry.entities[i];
            Entity entity_other = collisionsRegistry.components[i].other;

            // For now, we are only interested in collisions that involve the player
            if (registry.players.has(entity) && !registry.deathTimers.has(entity)) {
                //Player& player = registry.players.get(entity);

                // Checking Player - HardShell collisions
                // Location of player and virus collision behaviour implementation
                if (registry.viruses.has(entity_other)) {
                    // initiate death unless already dying
                    switch (registry.viruses.get(entity_other).pathogen) {
                    case TEXTURE_ASSET_ID::VIRUS:
                        changeState(LEVEL_STATE_COMBAT, false, Sickman::PATHOGEN_TYPE::VIRUS, entity_other);
                        break;
                    case TEXTURE_ASSET_ID::BACTERIA:
                        changeState(LEVEL_STATE_COMBAT, false, Sickman::PATHOGEN_TYPE::BACTERIA, entity_other);
                        break;
                    case TEXTURE_ASSET_ID::FUNGUS:
                        changeState(LEVEL_STATE_COMBAT, false, Sickman::PATHOGEN_TYPE::FUNGUS, entity_other);
                        break;
                    }

                }
                    // Checking Player - item collisions
                else if (registry.items.has(entity_other)) {

                    registry.remove_all_components_of(entity_other);
                    Mix_PlayChannel(-1, player_get_item_sound, 0);
                    ++points;

                }

            }
            //Collisions with platforms
            if (registry.solid_platforms.has(entity) && registry.gravities.has(entity_other) &&
                !registry.players.has(entity_other)) {

                Motion &motion_platform = registry.motions.get(entity);
                Motion &motion_other = registry.motions.get(entity_other);

                vec2 platform_pos = motion_platform.position;
                vec2 platform_scale = get_bounding_box(motion_platform) / 2.f;

                vec2 other_pos = motion_other.position;
                vec2 other_scale = get_bounding_box(motion_other) / 2.f;

                //A platform is below an object when the ypos of object is less than the platform posy

                bool platform_below = other_pos.y < platform_pos.y;
                bool platform_left = other_pos.x > platform_pos.x;

                vec2 shortest_path_out_collision = {0.f, 0.f};
                vec2 relative_distance;


                //The asserts are to make sure that the values are positive and are within bounds

                relative_distance.y =
                        std::min(abs((other_pos.y + other_scale.y) - (platform_pos.y - platform_scale.y)),
                                 abs((other_pos.y - other_scale.y) - (platform_pos.y + platform_scale.y)));
                relative_distance.x =
                        std::min(abs((other_pos.x - other_scale.x) - (platform_pos.x + platform_scale.x)),
                                 abs((other_pos.x + other_scale.x) - (platform_pos.x - platform_scale.x)));

                if (abs(relative_distance.x) < abs(relative_distance.y)) {
                    if (platform_left) {
                        shortest_path_out_collision.x += relative_distance.x;
                    } else {
                        shortest_path_out_collision.x -= relative_distance.x;
                    }


                } else {
                    motion_other.velocity.y = 0;
                    if (platform_below) {
                        //registry.gravities.remove(entity_other);

                        shortest_path_out_collision.y -= relative_distance.y;
                    } else {
                        shortest_path_out_collision.y += relative_distance.y;
                    }

                }

                motion_other.position += shortest_path_out_collision;
            }
                //Check mouse collisions
                if (registry.mouses.has(entity)) {
                    if (registry.menuButtons.has(entity_other)) {
                        auto button_type = registry.menuButtons.get(entity_other).type;
                        printf("%d\n", button_type);
                        if (registry.battles.get(registry.players.entities[0]).current_state ==
                            COMBAT_STATE::WAIT) {
                            Battle& battle = registry.battles.get(registry.players.entities[0]);

                            switch (button_type) {
                                case MenuButton::Type::FIGHT:
                                    registry.uis.get(UI).current_state = Ui::State::FIGHT;
                                    break;
                                case MenuButton::Type::BACK:
                                    registry.uis.get(UI).current_state = Ui::State::MAIN;
                                    break;
                                case MenuButton::Type::RUN:
                                    battle.action = PLAYER_ACTION::RUN;
                                    break;
                                case MenuButton::Type::ALLIES:
                                    registry.uis.get(UI).current_state = Ui::State::ALLIES;
                                    break;
                                case MenuButton::Type::ITEMS:
                                    registry.uis.get(UI).current_state = Ui::State::ITEMS;
                                    break;
                                case MenuButton::Type::PUNCH:
                                     if (battle.action == PLAYER_ACTION::IDLE) {
                                        registry.uis.get(UI).current_state = Ui::State::MAIN;
//                                        registry.menuButtons.get(entity_other).type = MenuButton::Type::MAIN;
                                        battle.action = PLAYER_ACTION::ATTACK;
                                        curr_player_attack.attack_type = Attack::ATTACK_TYPE::OFFENCE;
                                        curr_player_attack.name = "PUNCH";
                                        curr_player_attack.base_dmg = 25;
                                     }
                                    break;
                                case MenuButton::Type::SHOOT:
                                     if (battle.action == PLAYER_ACTION::IDLE) {
                                        registry.uis.get(UI).current_state = Ui::State::MAIN;
//                                        registry.menuButtons.get(entity_other).type = MenuButton::Type::MAIN;
                                        battle.action = PLAYER_ACTION::ATTACK;
                                        curr_player_attack.attack_type = Attack::ATTACK_TYPE::OFFENCE;
                                        curr_player_attack.name = "SHOOT";
                                        curr_player_attack.base_dmg = 35;
                                     }
                                    break;
                                case MenuButton::Type::HEAT:
                                     if (battle.action == PLAYER_ACTION::IDLE) {
                                        registry.uis.get(UI).current_state = Ui::State::MAIN;
//                                        registry.menuButtons.get(entity_other).type = MenuButton::Type::MAIN;
                                        battle.action = PLAYER_ACTION::ATTACK;
                                        curr_player_attack.attack_type = Attack::ATTACK_TYPE::OFFENCE;
                                        curr_player_attack.name = "HEAT";
                                        curr_player_attack.base_dmg = 30;
                                     }
                                    break;
                            }
                            updateUI();
                        }
                    }

                }


            }
            registry.collisions.clear();



    }

void WorldSystem::toggle_help() {
    if (help.in_help_mode) {
        help.in_help_mode = false;
        Entity help_entity = registry.helpComponent.entities[0];
        registry.renderRequests.remove(help_entity);
        registry.helpComponent.remove(help_entity);
        registry.remove_all_components_of(help_entity);

    }
    else {
        help.in_help_mode = true;
        createHelpBox(vec2(game_w / 2, game_h / 2), vec2{ game_w / 2, game_h / 2 });
    }
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // action can be GLFW_PRESS GLFW_RELEASE GLFW_REPEAT
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


    // Controls for platformer
    if (level_state == LEVEL_STATE_SELECTOR) {

        // Movement with WASD
        Entity& player = registry.players.entities[0];
        Motion& motion = registry.motions.get(player);
        float player_speed = registry.players.get(player).player_speed;
        bool ladder = false;

        if (!registry.deathTimers.has(player)) {
            switch (key) {
            case GLFW_KEY_W:	// Move up or jump
                if ((action == GLFW_PRESS || action == GLFW_REPEAT) && ladder) {
                    motion.velocity.y = -player_speed;

                }
                else if ((action == GLFW_PRESS || action == GLFW_REPEAT) && motion.velocity.y == 0) {
                    motion.velocity.y = -2.5 * player_speed;
                    Mix_PlayChannel(-1, jump_sound, 0);
                }
                else if (action == GLFW_RELEASE) {
                    motion.velocity.y = motion.velocity.y;
                }
                break;
            case GLFW_KEY_A:	// Move left
                if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                    motion.velocity.x = -player_speed;
                    if (motion.scale.x < 0)
                        motion.scale.x *= -1;
                }
                else if (action == GLFW_RELEASE) {
                    motion.velocity.x = 0;
                }
                break;
            case GLFW_KEY_S:	// Move down
                if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                    if (ladder) {
                        motion.velocity.y = player_speed;
                    }
                }
                else if (action == GLFW_RELEASE) {
                    motion.velocity.y = 0;
                }
                break;
            case GLFW_KEY_D:	// Move right
                if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                    motion.velocity.x = player_speed;
                    if (motion.scale.x > 0)
                        motion.scale.x *= -1;
                }
                else if (action == GLFW_RELEASE) {
                    motion.velocity.x = 0;
                }
                break;
            }
        }
    }
    else if (level_state == LEVEL_STATE_COMBAT) {
        if (key == GLFW_MOUSE_BUTTON_LEFT) {
            switch (action) {
            case GLFW_PRESS:
                break;
            case GLFW_REPEAT:
                break;
            case GLFW_RELEASE:
                break;
            }
        }
    }

    // Resetting game
    if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
        int w, h;
        glfwGetWindowSize(window, &w, &h);

        restart_game();
    }

    // Debugging
    if (key == GLFW_KEY_P) {
        if (action == GLFW_RELEASE)
            debugging.in_debug_mode = false;
        else
            debugging.in_debug_mode = true;
    }

    // Help
    if (key == GLFW_KEY_H) {
        if (action == GLFW_RELEASE) {
            toggle_help();
        }
    }

    if (key == GLFW_KEY_X) {
        if (action == GLFW_RELEASE) {
            if (help.in_help_mode && !registry.storyComponents.entities.size()) {
                toggle_help();
                tutorial_closed = 1;
            }
            for (uint i = 0; i < registry.storyComponents.entities.size(); i++) {
                story_start_closed = 1;
                Entity story_entity = registry.storyComponents.entities[i];
                registry.renderRequests.remove(story_entity);
                registry.storyComponents.remove(story_entity);
                registry.remove_all_components_of(story_entity);
            }
            if (story_start_closed && !tutorial_closed) toggle_help();
        }
    }

    /*if (key == GLFW_KEY_L) {
        if (action == GLFW_RELEASE)
            if (level_state == LEVEL_STATE_COMBAT)
                changeState(LEVEL_STATE_SELECTOR, false, Sickman::PATHOGEN_TYPE::NA);
            else if (level_state == LEVEL_STATE_SELECTOR) {
                changeState(LEVEL_STATE_COMBAT, false, Sickman::PATHOGEN_TYPE::BACTERIA);
            }
    }*/

    if (key == GLFW_KEY_LEFT_BRACKET) {
        if (action == GLFW_RELEASE)
            if (level_state == LEVEL_STATE_SELECTOR) {
                saveState(1,1);
            }
    }

    if (key == GLFW_KEY_RIGHT_BRACKET) {
        if (action == GLFW_RELEASE)
            if (level_state == LEVEL_STATE_SELECTOR) {
                if (loadState(1)) {
                    save_state = 1;
                    level_state = LEVEL_STATE_COMBAT; // ugly fix to fix save load
                    restart_game();
                }
                
            }
    }

    // Control the current speed with `<` `>`
    if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
        current_speed -= 0.1f;
        printf("Current speed = %f\n", current_speed);
    }
    if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
        current_speed += 0.1f;
        printf("Current speed = %f\n", current_speed);
    }
    current_speed = fmax(0.f, current_speed);
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {

    this->mouse_position = mouse_position;
    if (registry.mouses.has(mouse)) {
        registry.motions.get(mouse).position = mouse_position;       
    }


}


void WorldSystem::on_mouse_input(int button, int action, int mods)
{

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

      Motion& motion_mouse = registry.motions.get(mouse);
      Entity entity_j = mouse;
      //Iterate through all buttons and check for collision
      if (registry.menuButtons.size() > 0) {
        for (uint i = 0; i < registry.menuButtons.size(); i++) {
          Entity entity_i = registry.menuButtons.entities[i];
          Motion& motion_button = registry.motions.get(entity_i);

          if (collides(motion_mouse, motion_button) && i != 0) {

            registry.collisions.emplace_with_duplicates(entity_i, entity_j);
            registry.collisions.emplace_with_duplicates(entity_j, entity_i);

          }
        }
      }

    }

  // Mouse Gesture in combat mode
  if (level_state == LEVEL_STATE_COMBAT) {
      if (button == GLFW_MOUSE_BUTTON_LEFT) {
          if(GLFW_PRESS == action) {
              l_button_down = true;
              prev_mouse_position = mouse_position;
          }
          else if(GLFW_RELEASE == action && l_button_down) {
              l_button_down = false;
              Battle& battle = registry.battles.get(registry.players.entities[0]);

              // BACK BUTTON, drag left-down
              if (prev_mouse_position.x > mouse_position.x && prev_mouse_position.y < mouse_position.y) {
                  // call back button
                  registry.uis.get(UI).current_state = Ui::State::MAIN;

              }

              // PUNCH BUTTON, drag right-up
              else if (prev_mouse_position.x < mouse_position.x && prev_mouse_position.y > mouse_position.y) {
                  battle.action = PLAYER_ACTION::ATTACK;
                  curr_player_attack.attack_type = Attack::ATTACK_TYPE::OFFENCE;
                  curr_player_attack.name = "PUNCH";
                  curr_player_attack.base_dmg = 25;
              }

              // HEAT BUTTON, drag left left-up
              if (prev_mouse_position.x > mouse_position.x && prev_mouse_position.y > mouse_position.y) {
                  battle.action = PLAYER_ACTION::ATTACK;
                  curr_player_attack.attack_type = Attack::ATTACK_TYPE::OFFENCE;
                  curr_player_attack.name = "HEAT";
                  curr_player_attack.base_dmg = 30;
              }

              // SHOOT BUTTON, drag right-down
              if (prev_mouse_position.x < mouse_position.x && prev_mouse_position.y < mouse_position.y) {
                  battle.action = PLAYER_ACTION::ATTACK;
                  curr_player_attack.attack_type = Attack::ATTACK_TYPE::OFFENCE;
                  curr_player_attack.name = "SHOOT";
                  curr_player_attack.base_dmg = 35;
              }
              updateUI();
          }
      }


  }

}


void WorldSystem::changeState(unsigned int new_state, bool win, Sickman::PATHOGEN_TYPE pathogen, Entity overworld_enemy) {


    // Change state flag
    if (level_state != new_state) {
        level_state = new_state;
        renderer->removeBackgrounds();

        //Delete all virus entities
        for (int i = 0; i < registry.viruses.size(); i++) {
            if (registry.viruses.components[i].in_combat)
                registry.remove_all_components_of(registry.viruses.entities[i]);
            else
                registry.renderRequests.remove(registry.viruses.entities[i]);
        }


        Entity player = registry.players.entities[0];
        Motion &player_motion = registry.motions.get(player);

        if (new_state == LEVEL_STATE_SELECTOR) {
            // TODO: Find reason for returning to level selector: win or death

            if (!registry.items.entities.empty()) {
                for (Entity entity : registry.items.entities) {
                    registry.remove_all_components_of(entity);
                }
            }

            if (registry.battles.components.size() > 0) {
                if (win) {
                    registry.remove_all_components_of(registry.battles.components[0].enemy);
                    registry.remove_all_components_of(overworld_enemy);
                }
            }
            registry.battles.clear();

            for (int i = 0; i < registry.viruses.size(); i++) {
                if (!registry.renderRequests.has(registry.viruses.entities[i])) {
                    registry.renderRequests.insert(
                        registry.viruses.entities[i],
                        { registry.viruses.components[i].pathogen,
                         EFFECT_ASSET_ID::TEXTURED,
                         GEOMETRY_BUFFER_ID::SPRITE });
                }
            }

			// Add backgrounds for level selector
			this->renderer->addBackground(10, renderer, TEXTURE_ASSET_ID::SKY, this->game_w,
				this->game_h, this->game_w, this->game_h / 20,
				vec2({ this->game_w * 3, this->game_h }));
			this->renderer->addBackground(2, renderer, TEXTURE_ASSET_ID::MOUNTAIN, this->game_w, this->game_h,
				this->game_w, this->game_h / 2,
				vec2({ this->game_w * 3, this->game_h / 1.5 }));
			this->renderer->addBackground(1, renderer, TEXTURE_ASSET_ID::CITY, this->game_w, this->game_h,
				this->game_w, 3 * this->game_h / 5,
				vec2({ this->game_w * 2, this->game_h }));

            createWall(DOWN_WALL, this->game_w, this->game_h);
            createWall(LEFT_WALL, this->game_w, this->game_h);

            std::vector<std::vector<float>> platforms = getPlatformRows(j, game_w, game_h);
            std::vector<int> platform_numbers = getPlatformNumbers(j);
            std::vector<std::vector<float>>::iterator iter2;
            // for (iter2 = platforms.begin(); iter2 != platforms.end(); iter2++) {
            //     std::vector<float> curr = *iter2;
            //     createPlatformRow(curr[0] * game_w, curr[1] * game_h, {curr[2] * game_w, curr[3] * game_w});
            // }
            for (int i = 0; i < platforms.size(); i++) {
                createPlatformRow(platforms[i][0] * game_w, platforms[i][1] * game_h, {platforms[i][2] * game_w, platforms[i][3] * game_w}, platform_numbers[i]);
            }

            // Move player to correct coordinates
            if (win) {
                player_motion.position = player_position;
                std::vector<vec2>::iterator iter_v;
                for (int i = 0; i < virus_positions_cache.size(); i++) {
                    if (virus_positions_cache[i].x == defeated_virus_position.x && virus_positions_cache[i].y == defeated_virus_position.y) {
                        virus_positions_cache.erase(virus_positions_cache.begin()+i);
                        pathogen_types.erase(pathogen_types.begin()+i);
                        break;
                    }
                }
            }
            else {
                // player_motion.position = getPlayerPosition(j, game_w, game_h);
                if (save_state == 0) {
                    player_motion.position = getPlayerPosition(j, game_w, game_h);
                } else {
                    std::ifstream i2(std::string(PROJECT_SOURCE_DIR) + "src/save_states/state" + std::to_string(save_state) + ".json");
                    auto save_j = json::parse(i2);
                    player_motion.position = getPlayerPosition(save_j, game_w, game_h);
                    }
                //std::vector<vec2> virus_positions = getVirusPositions(j, game_w, game_h);
                //std::vector<vec2>::iterator iter_v;
                //// printf("virus positions size: %i\n", virus_positions.size());
                //for (iter_v = virus_positions.begin(); iter_v != virus_positions.end(); iter_v++) {
                //    createVirus(renderer, *iter_v, TEXTURE_ASSET_ID::VIRUS, false);
                //}
                //std::vector<vec2> bacteria_positions = getBacteriaPositions(j, game_w, game_h);
                //std::vector<vec2>::iterator iter_b;
                //// printf("virus positions size: %i\n", virus_positions.size());
                //for (iter_b = bacteria_positions.begin(); iter_b != bacteria_positions.end(); iter_b++) {
                //    createVirus(renderer, *iter_b, TEXTURE_ASSET_ID::BACTERIA, false);
                //}
                //// createBacteria(renderer, { 230, 600 });
                //std::vector<vec2> fungus_positions = getFungusPositions(j, game_w, game_h);
                //std::vector<vec2>::iterator iter_f;
                //// printf("virus positions size: %i\n", virus_positions.size());
                //for (iter_f = fungus_positions.begin(); iter_f != fungus_positions.end(); iter_f++) {
                //    createVirus(renderer, *iter_f, TEXTURE_ASSET_ID::FUNGUS, false);
                //}
            }
            if (!registry.gravities.has(player))
                registry.gravities.emplace(player);
            registry.uis.get(UI).current_state = Ui::State::NONE;


            for (Entity entity: registry.sickmen.entities) {
                registry.remove_all_components_of(entity);
            }

            for (Entity entity: registry.hpbars.entities) {
                registry.remove_all_components_of(entity);
            }

            for (Entity entity : registry.viruses.entities) {
                registry.remove_all_components_of(entity);
            }

            for (int i = 0; i < virus_positions_cache.size(); i++) {
                createVirus(renderer, virus_positions_cache[i], pathogen_types[i], false);
            }
            

        } else if (new_state == LEVEL_STATE_COMBAT) {
            removePlatforms();

            // Add background for combat scene
            this->renderer->addBackground(1, renderer, TEXTURE_ASSET_ID::COMBAT, this->game_w, this->game_h,
                                          this->game_w / 2, this->game_h / 2, vec2({this->game_w, this->game_h}));


            // Save player coordinates in overworld
            player_position = {player_motion.position.x, player_motion.position.y};
            // Move player to correct coordinates
            player_motion.position = {game_w / 5, game_h - abs(player_motion.scale.y) / 2};
            player_motion.velocity = {0, 0};
            player_motion.acceleration = {0, 0};
            registry.gravities.remove(player);

            // Spawn virus
            // Create render request for player controls

            registry.uis.get(UI).current_state = Ui::State::MAIN;

            Entity hp_p = createHP({game_w / 5, game_h - 150});
            Entity hp_e = createHP({game_w - 250, (game_h - 300)});

//            createVirus(renderer, { game_w - 50, game_h - abs(VIRUS_BB_HEIGHT) / 2 });
            Entity enemy = createSickman(renderer, {game_w - 250, (game_h - abs(SICKMAN_BB_HEIGHT) / 2)}, pathogen);

            Motion& overworld_enemy_motion = registry.motions.get(overworld_enemy);
            defeated_virus_position = overworld_enemy_motion.position;
            startBattle(enemy, overworld_enemy);

            for (Entity entity : registry.viruses.entities) {
                registry.remove_all_components_of(entity);
            }

            switch (pathogen) {
            case Sickman::PATHOGEN_TYPE::VIRUS:
                createVirus(renderer, { game_w - 250, (game_h - abs(VIRUS_BB_HEIGHT) - 30) }, TEXTURE_ASSET_ID::VIRUS, true);
                createVirus(renderer, { game_w - 220, (game_h - abs(VIRUS_BB_HEIGHT) - 120) }, TEXTURE_ASSET_ID::VIRUS, true);
                createVirus(renderer, { game_w - 240, (game_h - abs(VIRUS_BB_HEIGHT) - 150) }, TEXTURE_ASSET_ID::VIRUS, true);
                createVirus(renderer, { game_w - 275, (game_h - abs(VIRUS_BB_HEIGHT) - 200) }, TEXTURE_ASSET_ID::VIRUS, true);
                createVirus(renderer, { game_w - 300, (game_h - abs(VIRUS_BB_HEIGHT) - 70) }, TEXTURE_ASSET_ID::VIRUS, true);
                break;
            case Sickman::PATHOGEN_TYPE::BACTERIA:
                createVirus(renderer, { game_w - 250, (game_h - abs(VIRUS_BB_HEIGHT) - 30) }, TEXTURE_ASSET_ID::BACTERIA, true);
                createVirus(renderer, { game_w - 220, (game_h - abs(VIRUS_BB_HEIGHT) - 120) }, TEXTURE_ASSET_ID::BACTERIA, true);
                createVirus(renderer, { game_w - 240, (game_h - abs(VIRUS_BB_HEIGHT) - 150) }, TEXTURE_ASSET_ID::BACTERIA, true);
                createVirus(renderer, { game_w - 275, (game_h - abs(VIRUS_BB_HEIGHT) - 200) }, TEXTURE_ASSET_ID::BACTERIA, true);
                createVirus(renderer, { game_w - 300, (game_h - abs(VIRUS_BB_HEIGHT) - 70) }, TEXTURE_ASSET_ID::BACTERIA, true);
                break;
            case Sickman::PATHOGEN_TYPE::FUNGUS:
                createVirus(renderer, { game_w - 250, (game_h - abs(VIRUS_BB_HEIGHT) - 30) }, TEXTURE_ASSET_ID::FUNGUS, true);
                createVirus(renderer, { game_w - 220, (game_h - abs(VIRUS_BB_HEIGHT) - 120) }, TEXTURE_ASSET_ID::FUNGUS, true);
                createVirus(renderer, { game_w - 240, (game_h - abs(VIRUS_BB_HEIGHT) - 150) }, TEXTURE_ASSET_ID::FUNGUS, true);
                createVirus(renderer, { game_w - 275, (game_h - abs(VIRUS_BB_HEIGHT) - 200) }, TEXTURE_ASSET_ID::FUNGUS, true);
                createVirus(renderer, { game_w - 300, (game_h - abs(VIRUS_BB_HEIGHT) - 70) }, TEXTURE_ASSET_ID::FUNGUS, true);
                break;
            }

            // render story boxes
            if (player_position.x > game_w / 2 && player_position.x <= game_w) {
                if (player_position.y > game_h / 2) {
                    // story bottom right
                    createStoryBox(vec2(game_w / 2, game_h / 2), vec2{game_w, game_h}, TEXTURE_ASSET_ID::STORY_BR);
                } else {
                    // story top right
                    createStoryBox(vec2(game_w / 2, game_h / 2), vec2{game_w, game_h}, TEXTURE_ASSET_ID::STORY_TR);
                }
            } else if (player_position.x <= game_w / 2) {
                if (player_position.y > game_h / 2) {
                    // story bottom left
                    createStoryBox(vec2(game_w/2, game_h / 2), vec2{game_w, game_h}, TEXTURE_ASSET_ID::STORY_BL);
                } else {
                    // story top left
                    createStoryBox(vec2(game_w/2, game_h / 2), vec2{game_w, game_h}, TEXTURE_ASSET_ID::STORY_TL);
                }
            } else if (player_position.x > game_w) {
                //story far right
                createStoryBox(vec2(player_position.x, game_h / 2), vec2{game_w, game_h}, TEXTURE_ASSET_ID::STORY_FAR_RIGHT);
            }
        }
    }
    // Change backgrounds
    updateUI();
}

void WorldSystem::resolveCombat() {

    if (registry.battles.components.size() == 0)
        return;
    assert(registry.battles.components.size() == 1);

    Entity player = registry.players.entities[0];

    // Battle component always belongs to entity that initiates combat, i.e, player.
    // PRECONDITION: Only one Battle component in registry during battle scene. 0 otherwise
    Battle& battle = registry.battles.get(player);
    Entity enemy = battle.enemy;
    COMBAT_STATE prev_state = battle.prev_state;
    Fighter& player_fighter = registry.fighters.get(player);
    Fighter& enemy_fighter = registry.fighters.get(enemy);
    Entity hp1 = registry.hpbars.entities[0];
    Entity hp2 = registry.hpbars.entities[1];

    auto now = Clock::now();
    if (!wait) wait_start = Clock::now();

    switch (battle.current_state) {
    case COMBAT_STATE::WAIT:
        break;

    case COMBAT_STATE::ATTACK:
        battle.action = PLAYER_ACTION::IDLE;
        battle.curr_player_attack = curr_player_attack;
        // perform attack
        Mix_PlayChannel(-1, player_attack_sound, 0);
        break;

    case COMBAT_STATE::USE_ITEM:
        // use item
        // PICK ITEM TO USE IN MOUSE CLICK FUNCTION
        // CHECK HERE WHICH ITEM WAS PICKED
        // GOTO UPDATE_PLAYER_STATS AND UPDATE WHATEVER IS AFFECTED BY ITEM
        Mix_PlayChannel(-1, heal_sound, 0);
        break;

    case COMBAT_STATE::SWAP:
        break;

    case COMBAT_STATE::UPDATE_PLAYER_STATS:
        // Check what item was used/what attack was done on player and update HP accordingly
        if (prev_state == COMBAT_STATE::ENEMY_ATTACK) {
            player_fighter.health -= battle.curr_attack.base_dmg;
            if (player_fighter.health > 0) {
                registry.motions.get(hp1).position.x -= battle.curr_attack.base_dmg * 0.75;
                registry.motions.get(hp1).scale.x -= battle.curr_attack.base_dmg * 1.5;
            } else {
                registry.motions.get(hp1).scale.x = 0;
            }
            fprintf(stderr, "Player HP: %d\n", player_fighter.health);
        }
        else if (prev_state == COMBAT_STATE::USE_ITEM) {
            // UPDATE PLAYER STATS
            // HEAL OR ADJUST ENEMY ATTACK MULTIPLIER DEPENDING ON WHAT ITEM IS USED
        }
        break;

    case COMBAT_STATE::UPDATE_ENEMY_STATS:
        // Check what item was used/what attack was done on enemy and update HP accordingly
        if (prev_state == COMBAT_STATE::ATTACK) {
            if (!registry.items.entities.empty()) {
                for (Entity entity : registry.items.entities) {
                    registry.remove_all_components_of(entity);
                }
            }
            if (battle.curr_player_attack.name == "PUNCH") {
                registry.sickmen.get(enemy).sick = true;
                registry.sickmen.get(enemy).fire = false;
                registry.sickmen.get(enemy).kick = true;
            }
            else if (battle.curr_player_attack.name == "HEAT") {
                registry.sickmen.get(enemy).kick = false;
                registry.sickmen.get(enemy).sick = true;
                createFireball(renderer,{game_w - 300, (game_h - abs(SICKMAN_BB_HEIGHT)/1.5 )});
                registry.sickmen.get(enemy).fire = true;
                 // PLAY CORRECT AUDIO
            }
            else if (battle.curr_player_attack.name == "SHOOT") {
                registry.sickmen.get(enemy).kick = false;
                registry.sickmen.get(enemy).fire = false;
                createVaccine(renderer,{game_w - 300, (game_h - abs(SICKMAN_BB_HEIGHT)/1.5 )});
                registry.sickmen.get(enemy).sick = false;
                // PLAY CORRECT AUDIO
            }
            enemy_fighter.health -= battle.curr_player_attack.base_dmg;
            if (enemy_fighter.health > 0) {
                registry.motions.get(hp2).position.x -= battle.curr_player_attack.base_dmg * 0.75;
                registry.motions.get(hp2).scale.x -= battle.curr_player_attack.base_dmg * 1.5;
            } else {
                registry.motions.get(hp2).scale.x = 0;
                registry.sickmen.get(enemy).dead = true;
            }
            fprintf(stderr, "Enemy HP: %d\n", enemy_fighter.health);
        }

        else if (prev_state == COMBAT_STATE::ENEMY_ATTACK) {
            enemy_fighter.health -= battle.curr_attack.base_dmg;
        }
        fprintf(stderr, "Enemy HP: %d\n", enemy_fighter.health);
        break;

    case COMBAT_STATE::ENEMY_WAIT:
        if (enemy_fighter.health <= 0) {
            wait = true;
        }
        break;

    case COMBAT_STATE::PARTICLES:
        if ((float)(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - wait_start)).count() / 1000 > 5000) {
            wait = false;
        }
        break;

    case COMBAT_STATE::ENEMY_ATTACK:
        while ((float)(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now() - now)).count() / 1000 < 1500.f) {}
        // call AI BTree to find best move
        ai.attack();

        fprintf(stderr, "ATTACK NAME: ");
        fprintf(stderr, battle.curr_attack.name.c_str());
        fprintf(stderr, ", Damage: %d\n", battle.curr_attack.base_dmg);
        
        // Use move
        switch (battle.curr_attack.attack_type) {
        case Attack::ATTACK_TYPE::OFFENCE:
            Mix_PlayChannel(-1, enemy_attack_sound, 0);
            // Check if battle.enemy_attacks_done has battle.curr_attack
            // If it doesn't, put in enemy_attacks_done and record damage done

            break;

        case Attack::ATTACK_TYPE::DEFENCE:
            Mix_PlayChannel(-1, defend_sound, 0);
            break;

        case Attack::ATTACK_TYPE::HEALING:
            Mix_PlayChannel(-1, heal_sound, 0);
            break;

        default:
            break;
        }
        break;

    case COMBAT_STATE::RETURN_WIN:
        points++;
        Mix_PlayChannel(-1, player_get_item_sound, 0);
        changeState(LEVEL_STATE_SELECTOR, true, Sickman::PATHOGEN_TYPE::NA, battle.overworld_enemy);
        break;

    case COMBAT_STATE::RETURN_LOSS:
        // check reason for loss
        if (player_fighter.health <= 0) {
            points = 0;
            Mix_PlayChannel(-1, player_dead_sound, 0);
        }
        changeState(LEVEL_STATE_SELECTOR, false, Sickman::PATHOGEN_TYPE::NA, battle.overworld_enemy);
        break;

    default:
        break;
    }
}

// Possible state transitions: waiting for input, attacking, updating enemy health, enemy deciding attack, enemy attacking, updating player health, waiting for input
// Possible state transitions: waiting for input, running, return to level selector
// Possible state transitions: waiting for input, attacking, updating enemy health, enemy dies, return to level selector (win)
// Possible state transitions: waiting for input, attacking, updating enemy health, enemy deciding attack, enemy attacking, updating player health, player dies, return to level selector (loss)
void WorldSystem::updateCombatState()
{
    if (registry.battles.components.size() == 0)
        return;
    assert(registry.battles.components.size() == 1);

    Entity player = registry.players.entities[0];

    // Battle component always belongs to entity that initiates combat, i.e, player.
    // PRECONDITION: Only one Battle component in registry during battle scene. 0 otherwise
    Battle& battle = registry.battles.get(player);
    Entity enemy = battle.enemy;
    COMBAT_STATE prev_state = battle.prev_state;
    battle.prev_state = battle.current_state;
    Fighter& player_fighter = registry.fighters.get(player);
    Fighter& enemy_fighter = registry.fighters.get(enemy);

    switch (battle.current_state) {
    case COMBAT_STATE::WAIT:
        // GOTO RETURN_LOSS if player hp <= 0
        if (player_fighter.health <= 0) {
            battle.current_state = COMBAT_STATE::RETURN_LOSS;
            break;
        }

        switch (battle.action) {
        case PLAYER_ACTION::ATTACK:
            battle.current_state = COMBAT_STATE::ATTACK;
            break;

        case PLAYER_ACTION::USE_ITEM:
            battle.current_state = COMBAT_STATE::USE_ITEM;
            break;

        case PLAYER_ACTION::SWAP:
            battle.current_state = COMBAT_STATE::SWAP;
            break;

        case PLAYER_ACTION::RUN:
            battle.current_state = COMBAT_STATE::RETURN_LOSS;
            break;

        default:
            battle.current_state = COMBAT_STATE::WAIT;
            break;
        }
        break;

    case COMBAT_STATE::ATTACK:
        battle.current_state = COMBAT_STATE::UPDATE_ENEMY_STATS;
        break;

    case COMBAT_STATE::USE_ITEM:
        battle.current_state = COMBAT_STATE::UPDATE_PLAYER_STATS;

    case COMBAT_STATE::SWAP:
        battle.current_state = COMBAT_STATE::ENEMY_WAIT;

    case COMBAT_STATE::UPDATE_PLAYER_STATS:
        // if came from player, goto enemy_wait
        if (prev_state == COMBAT_STATE::USE_ITEM)
            battle.current_state = COMBAT_STATE::ENEMY_WAIT;
        else
            battle.current_state = COMBAT_STATE::WAIT;
        break;

    case COMBAT_STATE::UPDATE_ENEMY_STATS:
        // if came from enemy, goto wait
        if (prev_state == COMBAT_STATE::ENEMY_ATTACK)
            battle.current_state = COMBAT_STATE::WAIT;
        else
            battle.current_state = COMBAT_STATE::ENEMY_WAIT;
        break;

    case COMBAT_STATE::ENEMY_WAIT:
        // GOTO ENEMY_DEATH if enemy hp <= 0

        if (enemy_fighter.health <= 0)
            battle.current_state = COMBAT_STATE::PARTICLES;
        else
            battle.current_state = COMBAT_STATE::ENEMY_ATTACK;
        break;

    case COMBAT_STATE::ENEMY_ATTACK:
        // IF attack, update player stats
        if (battle.curr_attack.attack_type == Attack::ATTACK_TYPE::OFFENCE)
            battle.current_state = COMBAT_STATE::UPDATE_PLAYER_STATS;
        else
            battle.current_state = COMBAT_STATE::UPDATE_ENEMY_STATS;
        break;

    case COMBAT_STATE::PARTICLES:
        if (wait) {
            battle.current_state = COMBAT_STATE::PARTICLES;
        }
        else {
            battle.current_state = COMBAT_STATE::RETURN_WIN;
        }
        break;

    case COMBAT_STATE::RETURN_WIN:
        battle.current_state = COMBAT_STATE::RETURN_WIN;
        break;

    case COMBAT_STATE::RETURN_LOSS:
        battle.current_state = COMBAT_STATE::RETURN_LOSS;
        break;

    default:
        battle.current_state = battle.current_state;
        break;
    }

}

void WorldSystem::startBattle(Entity enemy, Entity overworld_enemy) {
    Entity player = registry.players.entities[0];
    Battle& battle = registry.battles.emplace(player);
    battle.current_state = COMBAT_STATE::WAIT;
    battle.prev_state = COMBAT_STATE::WAIT;
    battle.action = PLAYER_ACTION::IDLE;
    battle.enemy = enemy;
    battle.overworld_enemy = overworld_enemy;

    vec3 multipliers = {1.0, 1.0, 1.0};
    std::vector<Attack> attacks = {};
    if (registry.motions.has(overworld_enemy)) {
        Motion& overworld_enemy_motion = registry.motions.get(overworld_enemy);    
        multipliers = getEnemyMultipliers(j, overworld_enemy_motion.position.x, overworld_enemy_motion.position.y);
        printf("DamageReduction: %f\n", multipliers[0]);
        printf("DamageMultiplier: %f\n", multipliers[1]);
        printf("HealthMultiplier: %f\n", multipliers[2]);
        attacks = getEnemyAttacks(j, overworld_enemy_motion.position.x, overworld_enemy_motion.position.y);
    }
    for (int i = 0; i < attacks.size(); i++) {
        battle.all_attacks.push_back(attacks[i]);
        if (Attack::ATTACK_TYPE::OFFENCE == attacks[i].attack_type) {
            battle.attacks_left.push_back(attacks[i]);
        }
        std::cout << "Attack name: " << attacks[i].name << std::endl;
    }
    
    Attack attack1;
    attack1.attack_type = Attack::ATTACK_TYPE::OFFENCE;
    attack1.base_dmg = 20;
    attack1.name = "Infect";

    Attack attack2;
    attack2.attack_type = Attack::ATTACK_TYPE::OFFENCE;
    attack2.base_dmg = 15;
    attack2.name = "Poison";

    Attack attack3;
    attack3.attack_type = Attack::ATTACK_TYPE::OFFENCE;
    attack3.base_dmg = 25;
    attack3.name = "Neural Damage";

    Attack defence;
    defence.attack_type = Attack::ATTACK_TYPE::DEFENCE;
    defence.base_dmg = -1;
    defence.name = "Mutate";

    Attack heal;
    heal.attack_type = Attack::ATTACK_TYPE::HEALING;
    heal.base_dmg = -1;
    heal.name = "Regenerate";

    Sickman& sickman = registry.sickmen.get(enemy);
    // switch (sickman.pathogen) {
    // case Sickman::PATHOGEN_TYPE::BACTERIA:
    //     battle.all_attacks.push_back(attack1);
    //     battle.attacks_left.push_back(attack1);
    //     battle.all_attacks.push_back(attack2);
    //     battle.attacks_left.push_back(attack2);
    //     battle.all_attacks.push_back(defence);
    //     battle.all_attacks.push_back(heal);
    //     break;

    // case Sickman::PATHOGEN_TYPE::FUNGUS:
    //     battle.all_attacks.push_back(attack3);
    //     battle.attacks_left.push_back(attack3);
    //     battle.all_attacks.push_back(attack2);
    //     battle.attacks_left.push_back(attack2);
    //     battle.all_attacks.push_back(attack1);
    //     battle.attacks_left.push_back(attack1);
    //     battle.all_attacks.push_back(defence);
    //     break;

    // case Sickman::PATHOGEN_TYPE::VIRUS:
    //     battle.all_attacks.push_back(attack3);
    //     battle.attacks_left.push_back(attack3);
    //     battle.all_attacks.push_back(attack2);
    //     battle.attacks_left.push_back(attack2);
    //     battle.all_attacks.push_back(attack1);
    //     battle.attacks_left.push_back(attack1);
    //     battle.all_attacks.push_back(heal);
    // default:
    //     break;
    // }
    // Get enemy attack list from json file and add to all_attacks
    // Add all attacks with ATTACK_TYPE::OFFENCE to attacks_left
}

void WorldSystem::createParticles(int num_particles, vec2 pos)
{
    //Entity particle = Entity();
    //auto& emitter = registry.emitters.emplace(particle);
    //emitter.num_particles = 100;
    //emitter.lifetime = 3.5f;
    //emitter.pos = pos;

    //auto& particle_motions = registry.particleMotions.emplace(particle);
    //for (int i = 0; i < 100; i++) {
    //    particle_motions.pos_x[i] = pos.x;
    //    particle_motions.pos_y[i] = pos.y;
    //    particle_motions.scales[i] = 5.f;

    //    /*auto& renderReq = registry.renderRequests.emplace(particle);
    //    renderReq.used_effect = EFFECT_ASSET_ID::COLOURED;
    //    renderReq.used_geometry = GEOMETRY_BUFFER_ID::PARTICLE;
    //    renderReq.used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;*/
    //}

}



