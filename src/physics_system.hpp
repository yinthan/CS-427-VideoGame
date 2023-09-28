#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "subject.hpp"

const float GRAVITY_ACCEL = 500.f;

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem: public Subject
{
public:
    void step(float elapsed_ms, float window_width_px, float window_height_px);


	PhysicsSystem()
	{
	}
};

vec2 get_bounding_box(const Motion& motion);
bool collides(const Motion& motion1, const Motion& motion2);