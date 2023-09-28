#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "nlohmann/json.hpp"
#include "world_init.hpp"

using json = nlohmann::json;

void saveState(int level, int state);
bool loadState(int state);
bool doesStateExist(int state);
void deleteState(int state);