#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "nlohmann/json.hpp"
#include "world_init.hpp"

using json = nlohmann::json;

vec2 getPlayerPosition(json j, int game_w, int game_h);
std::vector<vec2> getVirusPositions(json j, int game_w, int game_h);
std::vector<vec2> getBacteriaPositions(json j, int game_w, int game_h);
std::vector<vec2> getFungusPositions(json j, int game_w, int game_h);
vec3 getEnemyMultipliers(json j, float x, float y);
std::vector<Attack> getEnemyAttacks(json j, float x, float y);
std::vector<std::vector<float>> getPlatformRows(json j, int game_w, int game_h);
std::vector<int> getPlatformNumbers(json j);