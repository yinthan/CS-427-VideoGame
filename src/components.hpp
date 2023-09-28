#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"
#include <string>
#include <array>


/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

 // TODO GRAPHICS
enum class TEXTURE_ASSET_ID {
	VIRUS = 0,
	BACTERIA,
	FUNGUS,
	ITEM = FUNGUS + 1,
	SICKMAN = ITEM + 1,
	CITY = SICKMAN + 1,
	MOUNTAIN = CITY + 1,
	SKY = MOUNTAIN + 1,
	COMBAT = SKY + 1,
	PLATFORM = COMBAT + 1,
	HELP = PLATFORM + 1,
	VACCINE = HELP + 1,
	FIREBALL = VACCINE + 1,
	BATTLE_MENU = FIREBALL + 1,
	BUTTON_FIGHT = BATTLE_MENU + 1,
	BUTTON_ALLIES = BUTTON_FIGHT + 1,
	BUTTON_ITEMS = BUTTON_ALLIES + 1,
	BUTTON_RUN = BUTTON_ITEMS + 1,
	BATTLE_ATTACK,
	BUTTON_PUNCH,
	BUTTON_SHOOT,
	BUTTON_HEAT,
	BUTTON_BACK,
	STORY_BL = BUTTON_BACK + 1,
	STORY_BR = STORY_BL + 1,
	STORY_FAR_RIGHT = STORY_BR + 1,
	STORY_START = STORY_FAR_RIGHT + 1,
	STORY_TL = STORY_START + 1,
	STORY_TR = STORY_TL + 1,

	TEXTURE_COUNT = STORY_TR + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	PLAYER = COLOURED + 1,
	LINE = PLAYER + 1,
	TEXTURED = LINE + 1,
	ANIMATION = TEXTURED + 1,
	SCREEN = ANIMATION + 1,
	VACCINE = SCREEN + 1,
	SICKMAN = VACCINE + 1,
	FIREBALL = SICKMAN + 1,
	PARTICLE = FIREBALL + 1,
	EFFECT_COUNT = PARTICLE + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

struct Item {
	enum class item_type {
		attack_buff,
		defence_buff,
		healing
	};
	item_type type;
	float factor;
};

struct ItemInventory {
	std::vector<Item> items;
};

enum class GEOMETRY_BUFFER_ID {
	SPRITE = 0,
	ANIMATION = SPRITE + 1,
	PLAYER = ANIMATION + 1,
	DEBUG_LINE = PLAYER + 1,
    HP_LINE = DEBUG_LINE + 1,
	SCREEN_TRIANGLE = HP_LINE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

	int num_frames = 1;
};

// Player component
struct Player
{
	float player_speed;
};

struct Particle {
};

struct ParticleMotion {
	std::array<float, 100> pos_x;
	std::array<float, 100> pos_y;
	std::array<float, 100> scales;
};

// component for living virus (infected people)
// unsure if this would include infected objects

struct Fighter {
	int health;
	float damage_multiplier;
};

struct Attack {
	enum class ATTACK_TYPE {
		OFFENCE,
		DEFENCE,
		HEALING
	};
	ATTACK_TYPE attack_type;
	std::string name;
	int base_dmg;
};

struct Virus
{
	/*enum class PATHOGEN_TYPE {
		VIRUS,
		BACTERIA,
		FUNGUS,
		NA
	};*/
	TEXTURE_ASSET_ID pathogen;
	bool in_combat;
};

struct ParticleEmitter {
	int num_particles = 100;
	float lifetime;
	vec2 pos;
};


struct HP_bar
{

};

struct Sickman
{
    bool sick = true;
    bool kick = false;
	bool fire = false;
	bool dead = false;
	std::vector<Attack> all_attacks;
	enum class PATHOGEN_TYPE {
		VIRUS,
		BACTERIA,
		FUNGUS,
		NA
	};
	PATHOGEN_TYPE pathogen;
};

struct Solid_Platform {
};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0, 0 };
	float angle = 0;
	vec2 velocity = { 0, 0 };
	vec2 acceleration = { 0,0 };
	vec2 scale = { 10, 10 };
	//Every motion componet has motion state
	enum class State {Idle, Jump, Crouch, Climb};
	State current_state;

};
// Enabling gravity for objects
struct Gravity {
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
    // Note, an empty struct has size 1
};

// A struct to refer to debugging graphics in the ECS
struct Help
{
    bool in_help_mode = 0;
};
extern Help help;

struct HelpComponent {

};

struct Story
{
    bool show_story = 1;
};

struct StoryComponent {

};

// A timer that will be associated to dying player
struct DeathTimer
{
	float counter_ms = 3000;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

struct Background {
    int layer;
};

struct Ui {
	enum class State{NONE,MAIN,FIGHT,ALLIES,ITEMS,RUN};
	State current_state;
	enum class Type{MAIN_MENU, BATTLE_MENU, HELP_MENU};
	Type type = Type::MAIN_MENU;
};

struct MenuButton {
	enum class Type {BACK ,MAIN, FIGHT, ALLIES, ITEMS, RUN, MAIN_ATTACK, PUNCH, SHOOT, HEAT};
	Type type;
};
//Mouse tag to indicate entity is a mouse
struct Mouse {};

enum class COMBAT_STATE {
	WAIT,
	ATTACK,
	USE_ITEM,
	SWAP,
	UPDATE_PLAYER_STATS,
	UPDATE_ENEMY_STATS,
	ENEMY_WAIT,
	ENEMY_ATTACK,
	RETURN_WIN,
	RETURN_LOSS,
	PARTICLES
};

enum class PLAYER_ACTION {
	IDLE,
	ATTACK,
	USE_ITEM,
	SWAP,
	RUN
};

// Combat state machine
struct Battle {
	Entity enemy;
	COMBAT_STATE current_state;
	COMBAT_STATE prev_state;
	PLAYER_ACTION action;
	std::vector<Attack> all_attacks{};
	Attack curr_player_attack;
	Attack curr_attack;
	std::vector<Attack> attacks_left{};
	std::vector<Attack> enemy_attacks_done{};
	std::vector<int> dmg_done{};
	Entity overworld_enemy;
};

