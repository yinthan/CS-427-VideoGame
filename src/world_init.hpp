#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "components.hpp"

const int game_w = 1200;
const int game_h = 800;

const int LEFT_WALL = 0;
const int RIGHT_WALL = 1;
const int UP_WALL = 2;
const int DOWN_WALL = 3;

const int MAX_HP = 100;

// These are hard coded to the dimensions of the entity texture
// TODO Graphics
const float VIRUS_BB_WIDTH = 0.0125f * 1193.f * 3; // what should be dimensions of the virus??
const float VIRUS_BB_HEIGHT = 0.0125f * 1198.f * 3;

const float PLAYER_BB_HEIGHT = 0.12f * game_h;

const float SICKMAN_BB_WIDTH = 0.175f * 679.f; // what should be dimensions of the virus??
const float SICKMAN_BB_HEIGHT = 0.175f * 1411.f;

const float ITEM_BB_WIDTH = 0.4f * 512.f; // what should be dimensions of the virus??
const float ITEM_BB_HEIGHT = 0.4f * 512.f;

const float BUTTON_MAIN_BB_WIDTH = 0.3f * game_w;
const float BUTTON_MAIN_BB_HEIGHT = 0.35f * game_h;

const float VACCINE_BB_WIDTH = 0.05f * 1482.f;
const float VACCINE_BB_HEIGHT = 0.05f * 1080.f;


const float FIREBALL_BB_WIDTH = 0.15f * 350.f;
const float FIREBALL_BB_HEIGHT = 0.15f * 350.f;

const float BUTTON_SELECT_BB_WIDTH = 0.2 * game_w;
const float BUTTON_SELECT_BB_HEIGHT = 0.1 * game_h;

// the player
Entity createPlayer(RenderSystem* renderer, vec2 position);
// the enemy
Entity createVirus(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID pathogen, bool in_combat);
//Entity createBacteria(RenderSystem* renderer, vec2 position);
//Entity createFungus(RenderSystem* renderer, vec2 position);

Entity createSickman(RenderSystem* renderer, vec2 position, Sickman::PATHOGEN_TYPE pathogen);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

Entity createHP(vec2 position);

// filler args, not sure what to fill in yet
Entity createPlatform(vec2 pos, vec2 size);

Entity createHelpBox(vec2 pos, vec2 size);

Entity createStoryBox(vec2 pos, vec2 size, TEXTURE_ASSET_ID story_board);

void createPlatformRow(int starting_width, int row_height, vec2 platform_scale, int number = 3);
// create walls for game boundaries
Entity createWall(int wall_num, int width, int height);

// create buttons for battle menu
Entity createButton(RenderSystem* renderer, vec2 position, MenuButton::Type type);

Entity createVaccine(RenderSystem* renderer, vec2 position);

Entity createFireball(RenderSystem* renderer, vec2 position);


//Create mouse component for interation
Entity createMouse();


//Create UI for battle menu
Entity createUI();


