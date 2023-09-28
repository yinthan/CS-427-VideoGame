#include "json_parser.hpp"
#include <iostream>

vec2 getPlayerPosition(json j, int game_w, int game_h) {
    vec2 result = {0, 0};
    // std::string x_str = j["player"]["position"]["x"];
    auto player = j["player"];
    auto position = player["motion"]["position"];
    std::string x_str = position["x"];
    std::string y_str = position["y"];
    try {
        auto x = stof(x_str);
        result.x = x;
    } catch (...) {
        if (x_str == "game_h") {
            auto x = game_h - PLAYER_BB_HEIGHT / 2;
            result.x = x;
        }
    }
    try {
        auto y = stof(y_str);
        result.y = y;
    } catch (...) {
        if (y_str == "game_h") {
            auto y = game_h - PLAYER_BB_HEIGHT / 2;
            result.y = y;
        }
    }
    return result;
}

std::vector<vec2> getVirusPositions(json j, int game_w, int game_h) {
    std::vector<vec2> result;
    auto viruses = j["enemies"];
    for (int i = 0; i < viruses.size(); i++) {
        if (viruses[i]["type"] != "virus") {
            continue;
        }
        vec2 curr = {0, 0};
        auto position = viruses[i]["position"];
        std::string x_str = position["x"];
        std::string y_str = position["y"];
        try {
            auto x = stof(x_str);
            curr.x = x;
        } catch (...) {

        }
        try {
            auto y = stof(y_str);
            curr.y = y;
        } catch (...) {

        }
        result.push_back(curr);
    }


    return result;
}

std::vector<vec2> getFungusPositions(json j, int game_w, int game_h) {
    std::vector<vec2> result;
    auto viruses = j["enemies"];
    for (int i = 0; i < viruses.size(); i++) {
        if (viruses[i]["type"] != "fungus") {
            continue;
        }
        vec2 curr = {0, 0};
        auto position = viruses[i]["position"];
        std::string x_str = position["x"];
        std::string y_str = position["y"];
        try {
            auto x = stof(x_str);
            curr.x = x;
        } catch (...) {

        }
        try {
            auto y = stof(y_str);
            curr.y = y;
        } catch (...) {

        }
        result.push_back(curr);
    }


    return result;
}

std::vector<vec2> getBacteriaPositions(json j, int game_w, int game_h) {
    std::vector<vec2> result;
    auto viruses = j["enemies"];
    for (int i = 0; i < viruses.size(); i++) {
        if (viruses[i]["type"] != "bacteria") {
            continue;
        }
        vec2 curr = {0, 0};
        auto position = viruses[i]["position"];
        std::string x_str = position["x"];
        std::string y_str = position["y"];
        try {
            auto x = stof(x_str);
            curr.x = x;
        } catch (...) {

        }
        try {
            auto y = stof(y_str);
            curr.y = y;
        } catch (...) {

        }
        result.push_back(curr);
    }
    

    return result;
}

// returns the 3 battle multipliers related to enemy
// first is damage reduction, how much damage is reduced when hitting enemy
// 2nd is damage multiplier, how much base damage of enemy attacks is multiplied when hitting player
// 3rd is health multiplier, how much hp the enemy gets
vec3 getEnemyMultipliers(json j, float x, float y) {
    auto enemies = j["enemies"];
    for (int i = 0; i < enemies.size(); i++) {
        std::string x_str = enemies[i]["position"]["x"];
        std::string y_str = enemies[i]["position"]["y"];
        auto x_test = stof(x_str);
        auto y_test = stof(y_str);
        if (x_test == x && y_test == y) {
            return {enemies[i]["multipliers"]["damageReduction"],
             enemies[i]["multipliers"]["damageMultiplier"],
             enemies[i]["multipliers"]["healthMultiplier"]};
        }
    }
    return {1.0, 1.0, 1.0};
}

std::vector<Attack> getEnemyAttacks(json j, float x, float y) {
    auto enemies = j["enemies"];
    std::vector<Attack> result;
    for (int i = 0; i < enemies.size(); i++) {
        std::string x_str = enemies[i]["position"]["x"];
        std::string y_str = enemies[i]["position"]["y"];
        auto x_test = stof(x_str);
        auto y_test = stof(y_str);
        if (x_test == x && y_test == y) {
            auto attacks = enemies[i]["attacks"];
            for (int j = 0; j < attacks.size(); j++) {
                Attack curr;
                std::string attack_type = attacks[j]["type"];
                if (attack_type == "healing") {
                    curr.attack_type = Attack::ATTACK_TYPE::HEALING;
                } else if (attack_type == "defence") {
                    curr.attack_type = Attack::ATTACK_TYPE::DEFENCE;
                } else {
                    curr.attack_type = Attack::ATTACK_TYPE::OFFENCE;
                }
                curr.name = attacks[j]["name"];
                curr.base_dmg = attacks[j]["damage"];
                result.push_back(curr);
            }
        }
    }
    return result;
}

std::vector<std::vector<float>> getPlatformRows(json j, int game_w, int game_h) {
    std::vector<std::vector<float>> result;
    auto platforms = j["platforms"];
    for (int i = 0; i < platforms.size(); i++) {
        std::vector<float> curr;
        std::string x_str = platforms[i]["x"];
        std::string y_str = platforms[i]["y"];
        try {
            auto x = stof(x_str);
            curr.push_back(x);
        } catch (...) {

        }
        try {
            auto y = stof(y_str);
            curr.push_back(y);
        } catch (...) {

        }
        auto platform_scale = platforms[i]["platform_scale"];
        std::string plat_x_str = platform_scale["x"];
        std::string plat_y_str = platform_scale["y"];
        try {
            auto plat_x = stof(plat_x_str);
            curr.push_back(plat_x);
        } catch (...) {

        }
        try {
            auto plat_y = stof(plat_y_str);
            curr.push_back(plat_y);
        } catch (...) {

        }
        result.push_back(curr);
        // printf("curr: %f, %f, %f, %f\n", curr[0], curr[1], curr[2], curr[3]);
    }
    // printf("result size: %i\n", result.size());
    return result;
}

std::vector<int> getPlatformNumbers(json j) {
    std::vector<int> result;
    auto platforms = j["platforms"];
    for (int i = 0; i < platforms.size(); i++) {
        int curr = 3;
        if (platforms[i].contains("number")) {
            curr = platforms[i]["number"];
        }
        result.push_back(curr);
    }
    return result;
}