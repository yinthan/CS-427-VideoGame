#pragma once

// internal
#include "common.hpp"
#include "event.hpp"
#include "observer.hpp"
#include "ai_system.hpp"
// stlib
#include <vector>
#include <random>
#include <chrono>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"
#include "json_parser.hpp"
#include "nlohmann/json.hpp"
#include "save_load.hpp"


using json = nlohmann::json;
using Clock = std::chrono::high_resolution_clock;

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem : public Observer
{
public:
	WorldSystem();

	// Creates a window
	GLFWwindow* create_window(int width, int height);

	// starts the game
	void init(RenderSystem* renderer, int window_width_px, int window_height_px);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// Check for collisions
	void handle_collisions();

	void toggle_help();

	// Check for events
	void onNotify(Event event);

	// Should the game be over ?
	bool is_over()const;

	void animate();
private:
	// Input callback functions
	json j;
	json save_j;
	AISystem ai;
    bool l_button_down;
    vec2 prev_mouse_position;
  
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_input(int button, int action, int mods);

	// restart level
	void changeState(unsigned int new_state, bool win, Sickman::PATHOGEN_TYPE pathogen, Entity overworld_enemy);
	void restart_game();

	void updateUI();

	void removePlatforms();

	void resolveCombat();

	void updateCombatState();

	Attack curr_player_attack;

	void startBattle(Entity enemy, Entity overworld_enemy);

	// OpenGL window handle
	GLFWwindow* window;

	// Number of items gotten by the player, displayed in the window title
	unsigned int points;
	bool wait = false;
	

	// TODO Game state
	RenderSystem* renderer;
	float current_speed;
	float next_virus_spawn;
	vec2 mouse_position;
	Entity mouse;
	Entity UI;
	vec2 player_position;
    vec2 virus_position;
	std::vector<vec2> virus_positions_cache;
	std::vector<TEXTURE_ASSET_ID> pathogen_types;
	vec2 defeated_virus_position;

	int game_h;
	int game_w;


	//TODO game sounds
	// music references
	Mix_Music* background_music;
	Mix_Chunk* player_dead_sound;
	Mix_Chunk* player_get_item_sound;
	Mix_Chunk* player_attack_sound;
	Mix_Chunk* enemy_attack_sound;
	Mix_Chunk* defend_sound;
	Mix_Chunk* heal_sound;
	Mix_Chunk* jump_sound;

	// Particle stuff
	bool particles_present;
	void createParticles(int num_particles, vec2 pos);

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

    Background* city;
    Background* mountain;
    Background* sky;
};
