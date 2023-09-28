// Source https://gameprogrammingpatterns.com/observer.html

#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "event.hpp"

class Event
{
public:
	enum EventType {COLLISION};

	EventType type;
	Entity entity;
	Entity entity_other;

	//Creating the Event
	Event(EventType t) : type(t) {};

	//collision
	Event(EventType t, Entity e, Entity e_o): type(t), entity(e), entity_other(e_o) {}



};