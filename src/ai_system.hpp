#pragma once

#include <vector>

#include "tiny_ecs_registry.hpp"
#include "components.hpp"
#include "common.hpp"
#include "tiny_ecs_registry.hpp"
#include <cassert>
#include <iostream>
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include "tiny_ecs.hpp"

//extern bool fight;
//extern bool heal;
//extern bool defend;

class AISystem
{
public:
	AISystem() {}
	void attack();

private:
	bool heal;
	bool defend;
	bool fight;
};

enum class BTState {
	Running,
	Success,
	Failure
};

// The base class representing any node in our behaviour tree
class BTNode {
public:
	virtual void init(Entity e) {};

	virtual BTState process(Entity e) { return BTState::Success; };
};

// A composite node that loops through all children and exits when one fails
class BTRunPair : public BTNode {
private:
	int m_index;
	BTNode* m_children[2];

public:
	BTRunPair(BTNode* c0, BTNode* c1)
		: m_index(0) {
		m_children[0] = c0;
		m_children[1] = c1;
	}

	void init(Entity e) override
	{
		m_index = 0;
		// initialize the first child
		const auto& child = m_children[m_index];
		child->init(e);
	}

	BTState process(Entity e) override {
		if (m_index >= 2)
			return BTState::Success;

		// process current child
		BTState state = m_children[m_index]->process(e);

		// select a new active child and initialize its internal state
		if (state == BTState::Success) {
			++m_index;
			if (m_index >= 2) {
				return BTState::Success;
			}
			else {
				m_children[m_index]->init(e);
				return m_children[m_index]->process(e);
			}
		}
		else {
			return state;
		}
	}
};

// A general decorator with lambda condition
class BTIfCondition : public BTNode
{
public:
	BTIfCondition(BTNode* child_true, BTNode* child_false, std::function<bool(Entity)> condition)
		: m_childTrue(child_true), m_childFalse(child_false), m_condition(condition) {
	}

	virtual void init(Entity e) override {
		m_childTrue->init(e);
		m_childFalse->init(e);
	}

	virtual BTState process(Entity e) override {
		if (m_condition(e))
			return m_childTrue->process(e);
		else
			return m_childFalse->process(e);
	}

private:
	BTNode* m_childTrue;
	BTNode* m_childFalse;
	std::function<bool(Entity)> m_condition;
};

class BTIfCondition2 : public BTNode {
public:
	BTIfCondition2(BTNode* child, std::function<bool(Entity)> condition)
		: m_child(child), m_condition(condition) {
	}

	virtual void init(Entity e) override {
		m_child->init(e);
	}

	virtual BTState process(Entity e) override {
		if (m_condition(e))
			return m_child->process(e);
		else
			return BTState::Success;
	}

private:
	BTNode* m_child;
	std::function<bool(Entity)> m_condition;
};

class setAttack : public BTNode {
public:
	setAttack(Attack::ATTACK_TYPE type, bool val, bool* heal, bool* defend, bool* fight)
		: m_type(type), m_val(val), m_heal(heal), m_defend(defend), m_fight(fight) {}

private:
	void init(Entity e) override {
	}

	BTState process(Entity e) override {
		*m_fight = false;
		*m_heal = false;
		*m_defend = false;

		switch (m_type) {
		case Attack::ATTACK_TYPE::HEALING:
			*m_heal = m_val;
			break;
		case Attack::ATTACK_TYPE::DEFENCE:
			*m_defend = m_val;
			break;
		case Attack::ATTACK_TYPE::OFFENCE:
			*m_fight = m_val;
			break;
		default:
			break;
		}

		return BTState::Success;
	}

	Attack::ATTACK_TYPE m_type;
	bool m_val;
	bool* m_heal;
	bool* m_defend;
	bool* m_fight;
};

class UseAbility : public BTNode {
public:
	UseAbility(Battle& battle, Attack::ATTACK_TYPE type) noexcept
		: m_battle(battle), m_type(type) {}

private:
	void init(Entity e) override {
	}

	BTState process(Entity e) override {
		for (int i = 0; i < m_battle.all_attacks.size(); i++) {
			if (m_battle.all_attacks[i].attack_type == m_type) {
				m_battle.curr_attack = m_battle.all_attacks[i];
				break;
			}
		}
		return BTState::Success;
	}

	Battle& m_battle;
	Attack::ATTACK_TYPE m_type;
};

class UseNewAttack : public BTNode {
public:
	UseNewAttack(Battle& battle) noexcept
		: m_battle(battle) {}

private:
	void init(Entity e) override {
	}

	BTState process(Entity e) override {
		m_battle.curr_attack = m_battle.attacks_left.back();
		m_battle.attacks_left.pop_back();

		return BTState::Success;
	}

	Battle& m_battle;
};

class UseOldAttack : public BTNode {
public:
	UseOldAttack(Battle& battle) noexcept
		: m_battle(battle) {}

private:
	void init(Entity e) override {
	}

	BTState process(Entity e) override {

		int max_dmg_index = 0;
		int max_dmg = 0;
		for (int i = 0; i < m_battle.dmg_done.size(); i++) {
			if (m_battle.dmg_done.at(i) > max_dmg)
				max_dmg_index = i;
		}
		m_battle.curr_attack = m_battle.enemy_attacks_done.at(max_dmg_index);

		return BTState::Success;
	}

	Battle& m_battle;
};



