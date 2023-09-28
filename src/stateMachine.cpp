/*
#include "stateMachine.hpp"

template<typename Component>
void StateMachineSystem<Component>::Update(float elapsed)
{
}

template<typename Component>
void StateMachineSystem<Component>::add(std::string id, Entity entity)
{
	if (registry.players.has(entity)) {
		//State Transitions		
		if (_stateMap.at("idle").has(entity)) {
			_stateMap.at(id).emplace(entity);
			_stateMap.at("idle").remove(entity);
		}
		
	}
}

template<typename Component>
void StateMachineSystem<Component>::remove(std::string id, Entity entity)
{
	_stateMap.at(id).remove(entity);
}
*/