
#include "save_load.hpp"
#include <fstream>
#include "tiny_ecs_registry.hpp"

void saveState(int level, int state) {
    std::string filename = std::string(PROJECT_SOURCE_DIR) + "src/save_states/state" + std::to_string(state) + ".json";
    if (doesStateExist(state)) {
        // printf("State already exists\n");
        deleteState(state);
        // return;
    }
    json j;
    // const char a[] = "testing";
    // j["test"] = a;
    j["level"] = level;
    Entity player = registry.players.entities[0];
    Motion& player_motion = registry.motions.get(player);
    Player player_player = registry.players.components[0];
    j["player"]["motion"]["position"] = {{"x", std::to_string(player_motion.position.x)}, {"y", std::to_string(player_motion.position.y)}};
    j["player"]["motion"]["angle"] = player_motion.angle;
    j["player"]["motion"]["current_state"] = player_motion.current_state;
    j["player"]["motion"]["velocity"] = {{"x", player_motion.velocity.x}, {"y", player_motion.velocity.y}};
    j["player"]["motion"]["acceleration"] = {{"x", player_motion.acceleration.x}, {"y", player_motion.acceleration.y}};
    j["player"]["motion"]["scale"] = {{"x", player_motion.scale.x}, {"y", player_motion.scale.y}};
    j["player"]["player"]["player_speed"] = player_player.player_speed;
    //j["player"]["player"]["health"] = player_player.health;

    j["enemies"] = json::array();
    for (auto i = 0; i < registry.viruses.entities.size(); i++) {
        json object = json::object();
        Entity virus = registry.viruses.entities[i];
        Motion& virus_motion = registry.motions.get(virus);
        Virus virus_component = registry.viruses.components[i];

        // object["dead"] to check if virus is dead, need to implement later
        object["motion"]["position"] = {{"x", virus_motion.position.x}, {"y", virus_motion.position.y}};
        object["motion"]["angle"] = virus_motion.angle;
        object["motion"]["current_state"] = virus_motion.current_state;
        object["motion"]["velocity"] = {{"x", virus_motion.velocity.x}, {"y", virus_motion.velocity.y}};
        object["motion"]["acceleration"] = {{"x", virus_motion.acceleration.x}, {"y", virus_motion.acceleration.y}};
        object["motion"]["scale"] = {{"x", virus_motion.scale.x}, {"y", virus_motion.scale.y}};
        //object["virus"]["behaviour"] = virus_component.behaviour;
        //object["virus"]["health"] = virus_component.health;
        j["enemies"].push_back(object);
        // printf("got in loop, %f\n", object["motion"]["position"]["x"]);
    }

    std::ofstream o(filename);
    o << j;
    printf("State saved in state %i\n", state);
}

bool loadState(int state) {
    if (!doesStateExist(state)) {
        printf("Can't load state %i, doesn't exist\n", state);
        return false;
    }
    std::string filename = std::string(PROJECT_SOURCE_DIR) + "src/save_states/state" + std::to_string(state) + ".json";
    std::ifstream i(filename);
    json j = json::parse(i);
    // vec2 result = {j["player"]["motion"]["position"]["x"], j["player"]["motion"]["position"]["y"]};
    return true;

}

bool doesStateExist(int state) {
    std::string filename = std::string(PROJECT_SOURCE_DIR) + "src/save_states/state" + std::to_string(state) + ".json";
    std::ifstream i(filename);
    return i.good(); 

}

void deleteState(int state) {
    std::string filename = std::string(PROJECT_SOURCE_DIR) + "src/save_states/state" + std::to_string(state) + ".json";
    if (!doesStateExist(state)) {
        return;
    }
    std::ifstream i(filename);
    i.close();
}