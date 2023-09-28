// internal
#include "ai_system.hpp"


void AISystem::attack()
{
	Entity player = registry.players.entities[0];
	Battle& battle = registry.battles.get(player);
	Entity enemy = battle.enemy;
	std::vector<Attack> attacks = battle.all_attacks;

	// Conditions
	std::function<bool(Entity)> heal_condition = [this](Entity e) {return this->heal; };
	std::function<bool(Entity)> defend_condition = [this](Entity e) {return this->defend; };
	std::function<bool(Entity)> attack_condition = [this](Entity e) {return this->fight; };
	std::function<bool(Entity)> is_defensive_condition = [&battle](Entity e) {return registry.fighters.get(registry.players.entities[0]).health > 30 && registry.fighters.get(battle.enemy).health <= 30; };
	std::function<bool(Entity)> attacked_before = [&battle](Entity e) {return battle.enemy_attacks_done.size() > 0; };
	std::function<bool(Entity)> attacks_left = [&battle](Entity e) {return battle.attacks_left.size() > 0; };
	std::function<bool(Entity)> can_heal = [&attacks](Entity e) {return std::any_of(attacks.begin(), attacks.end(), [](Attack& a) {return a.attack_type == Attack::ATTACK_TYPE::HEALING; }); };
	std::function<bool(Entity)> can_defend = [&attacks](Entity e) {return std::any_of(attacks.begin(), attacks.end(), [](Attack& a) {return a.attack_type == Attack::ATTACK_TYPE::DEFENCE; }); };

	// Use attack branches
	UseAbility use_heal(battle, Attack::ATTACK_TYPE::HEALING);
	UseAbility use_defence(battle, Attack::ATTACK_TYPE::DEFENCE);

	// Attack type check branches
	setAttack attack_false(Attack::ATTACK_TYPE::OFFENCE, false, &this->heal, &this->defend, &this->fight);
	setAttack defence_false(Attack::ATTACK_TYPE::DEFENCE, false, &this->heal, &this->defend, &this->fight);
	setAttack healing_false(Attack::ATTACK_TYPE::HEALING, false, &this->heal, &this->defend, &this->fight);
	setAttack attack_true(Attack::ATTACK_TYPE::OFFENCE, true, &this->heal, &this->defend, &this->fight);
	setAttack defence_true(Attack::ATTACK_TYPE::DEFENCE, true, &this->heal, &this->defend, &this->fight);
	setAttack healing_true(Attack::ATTACK_TYPE::HEALING, true, &this->heal, &this->defend, &this->fight);

	// Sub-offence attack branches
	UseNewAttack first_attack(battle);
	UseNewAttack new_attack(battle);
	UseOldAttack old_attack(battle);
	BTIfCondition BT_attacks_left(&new_attack, &old_attack, attacks_left);
	BTIfCondition BT_attacked_before(&BT_attacks_left, &first_attack, attacked_before);

	// Select attack branches
	BTIfCondition2 BT_attack(&BT_attacked_before, attack_condition);
	BTIfCondition2 BT_heal(&use_heal, heal_condition);
	BTIfCondition2 BT_defend(&use_defence, defend_condition);

	BTRunPair attack_defence_false(&attack_false, &defence_false);
	BTRunPair all_false(&attack_defence_false, &healing_false);
	
	BTIfCondition BT_defensive(&defence_true, &attack_true, can_defend);
	BTIfCondition BT_healing(&healing_true, &BT_defensive, can_heal);
	BTIfCondition BT_def_branch(&BT_healing, &attack_true, is_defensive_condition);

	BTRunPair left_branch(&all_false, &BT_def_branch);

	// FINAL BEHAVIOUR TREE ASSEMBLY
	BTRunPair def_att_pair(&BT_defend, &BT_attack);
	BTRunPair heal_def_att(&BT_heal, &def_att_pair);
	BTRunPair behaviour_tree(&left_branch, &heal_def_att);

	behaviour_tree.init(enemy); 
 	BTState result = behaviour_tree.process(enemy);
	

}

