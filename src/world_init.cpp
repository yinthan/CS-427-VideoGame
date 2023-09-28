#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

// TODO player and Enemy creation

Entity createPlayer(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::PLAYER);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.acceleration = { 0.f, 0.f };
    motion.scale = mesh.original_size * 15.f;
    motion.scale.x *= -1;
    motion.scale.y *= -1; // point front to the right

	printf("%f %f", motion.scale.x, motion.scale.y);

	auto& gravity = registry.gravities.emplace(entity);
	auto& player = registry.players.emplace(entity);
	player.player_speed = 200.f;

	auto& fighter = registry.fighters.emplace(entity);
	fighter.health = MAX_HP;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::PLAYER,
		 GEOMETRY_BUFFER_ID::PLAYER });

	return entity;
}


Entity createVirus(RenderSystem* renderer, vec2 position, TEXTURE_ASSET_ID pathogen, bool in_combat)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::ANIMATION);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = vec2({-VIRUS_BB_WIDTH, VIRUS_BB_HEIGHT });

	Virus& virus = registry.viruses.emplace(entity);
	virus.pathogen = pathogen;
	virus.in_combat = in_combat;

	registry.renderRequests.insert(
		entity,
		{ pathogen,
		 EFFECT_ASSET_ID::ANIMATION,
		 GEOMETRY_BUFFER_ID::ANIMATION }); // how are we rendering viruses?

	int num_frames = 2;

	switch (pathogen) {
		case TEXTURE_ASSET_ID::BACTERIA:
		num_frames = 2;
		break;
		case TEXTURE_ASSET_ID::VIRUS:
		num_frames = 3;
		break;
		case TEXTURE_ASSET_ID::FUNGUS:
		num_frames = 2;
		break;
	}
	
	registry.renderRequests.get(entity).num_frames = num_frames;

	return entity;
}

//Entity createBacteria(RenderSystem* renderer, vec2 position)
//{
//	auto entity = Entity();
//
//	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
//	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//	registry.meshPtrs.emplace(entity, &mesh);
//
//	// Initialize the motion
//	auto& motion = registry.motions.emplace(entity);
//	motion.angle = 0.f;
//	motion.velocity = { 0.f, 0.f };
//	motion.position = position;
//	motion.scale = vec2({ -VIRUS_BB_WIDTH, VIRUS_BB_HEIGHT });
//
//	Virus& virus = registry.viruses.emplace(entity);
//	virus.pathogen = Virus::PATHOGEN_TYPE::BACTERIA;
//
//	registry.renderRequests.insert(
//		entity,
//		{ TEXTURE_ASSET_ID::BACTERIA,
//		 EFFECT_ASSET_ID::TEXTURED,
//		 GEOMETRY_BUFFER_ID::SPRITE }); // how are we rendering viruses?
//
//	return entity;
//}
//
//Entity createFungus(RenderSystem* renderer, vec2 position)
//{
//	auto entity = Entity();
//
//	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
//	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
//	registry.meshPtrs.emplace(entity, &mesh);
//
//	// Initialize the motion
//	auto& motion = registry.motions.emplace(entity);
//	motion.angle = 0.f;
//	motion.velocity = { 0.f, 0.f };
//	motion.position = position;
//	motion.scale = vec2({ -VIRUS_BB_WIDTH, VIRUS_BB_HEIGHT });
//
//	Virus& virus = registry.viruses.emplace(entity);
//	virus.pathogen = Virus::PATHOGEN_TYPE::FUNGUS;
//
//	registry.renderRequests.insert(
//		entity,
//		{ TEXTURE_ASSET_ID::FUNGUS,
//		 EFFECT_ASSET_ID::TEXTURED,
//		 GEOMETRY_BUFFER_ID::SPRITE }); // how are we rendering viruses?
//
//	return entity;
//}

Entity createSickman(RenderSystem* renderer, vec2 position, Sickman::PATHOGEN_TYPE pathogen)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.position = position;
	motion.scale = vec2({ -SICKMAN_BB_WIDTH, SICKMAN_BB_HEIGHT});
	Sickman& sickman = registry.sickmen.emplace(entity);
	sickman.pathogen = pathogen;
	// Add attacks

	Fighter& fighter = registry.fighters.emplace(entity);
	fighter.health = MAX_HP;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SICKMAN,
		 EFFECT_ASSET_ID::SICKMAN,
		 GEOMETRY_BUFFER_ID::SPRITE }); 

	return entity;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::LINE,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE });

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createHP(vec2 position)
{
    Entity entity = Entity();

    // Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
    registry.renderRequests.insert(
            entity,
            { TEXTURE_ASSET_ID::TEXTURE_COUNT,
              EFFECT_ASSET_ID::LINE,
              GEOMETRY_BUFFER_ID::HP_LINE });

    // Create motion
    Motion& motion = registry.motions.emplace(entity);
    motion.angle = 0.f;
    motion.velocity = { 0, 0 };
    motion.position = position;
    motion.scale = {150,10};

    registry.hpbars.emplace(entity);
    return entity;
}

Entity createPlatform(vec2 pos, vec2 size) {
	Entity entity = Entity();

    registry.renderRequests.insert(
            entity,
            {TEXTURE_ASSET_ID::PLATFORM,
             EFFECT_ASSET_ID::TEXTURED,
             GEOMETRY_BUFFER_ID::SPRITE });

	auto& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = size;
    motion.angle = 3.1415926f;

	registry.solid_platforms.emplace(entity);
	return entity;

}

Entity createHelpBox(vec2 pos, vec2 size) {
    Entity entity = Entity();

    registry.renderRequests.insert(
            entity,
            {TEXTURE_ASSET_ID::HELP,
             EFFECT_ASSET_ID::TEXTURED,
             GEOMETRY_BUFFER_ID::SPRITE });

    auto& motion = registry.motions.emplace(entity);
    motion.position = pos;
    motion.scale = size;
    registry.helpComponent.emplace(entity);
    return entity;
}

Entity createStoryBox(vec2 pos, vec2 size, TEXTURE_ASSET_ID story_board) {
    Entity entity = Entity();

    registry.renderRequests.insert(
        entity,
        {story_board,
         EFFECT_ASSET_ID::TEXTURED,
         GEOMETRY_BUFFER_ID::SPRITE });

    auto& motion = registry.motions.emplace(entity);
    motion.position = pos;
    motion.scale = size;
    registry.storyComponents.emplace(entity);
    return entity;
}

void createPlatformRow(int starting_width, int row_height, vec2 platform_scale, int number) {
	for (int i = 0; i < number; i++) {
    	createPlatform(vec2(starting_width + platform_scale.x * i, row_height), platform_scale);
    // createPlatform(vec2(starting_width + platform_scale.x, row_height), platform_scale);
    // createPlatform(vec2(starting_width + platform_scale.x*2, row_height), platform_scale);
	}

}

Entity createWall(int wall_num, int width, int height) {

	Entity entity = Entity();

	auto& motion = registry.motions.emplace(entity);
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	switch (wall_num)
	{
	case LEFT_WALL:
		motion.position = { 0, height / 2 };
		motion.scale = { 10, height };
		break;
	case RIGHT_WALL:
		motion.position = { width, height / 2 };
		motion.scale = { 10, height / 2 };
		break;
	case UP_WALL:
		motion.position = { width / 2, 0 };
		motion.scale = { width, 10 };
		break;
	case DOWN_WALL:
		motion.position = { width / 2, height };
		motion.scale = { width, 10 };
		break;
	default:
		motion.position = { 0, 0 };
		motion.scale = { 10, 10 };
		break;
	}
	registry.solid_platforms.emplace(entity);
	return entity;
}

Entity createButton(RenderSystem* renderer, vec2 position, MenuButton::Type type)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.position = position;

	switch (type) {
	case MenuButton::Type::MAIN:
	case MenuButton::Type::MAIN_ATTACK:
		motion.scale = { BUTTON_MAIN_BB_WIDTH, BUTTON_MAIN_BB_HEIGHT };
		break;
	default:
		motion.scale = { BUTTON_SELECT_BB_WIDTH, BUTTON_SELECT_BB_HEIGHT };
	}

    MenuButton & button = registry.menuButtons.emplace(entity);
    button.type = type;

	TEXTURE_ASSET_ID texture_asset_id;

	switch (type) {
	case MenuButton::Type::MAIN:
		texture_asset_id = TEXTURE_ASSET_ID::BATTLE_MENU;
		break;
	case MenuButton::Type::FIGHT:
		texture_asset_id = TEXTURE_ASSET_ID::BUTTON_FIGHT;
		break;
	case MenuButton::Type::ALLIES:
		texture_asset_id = TEXTURE_ASSET_ID::BUTTON_ALLIES;
		break;
	case MenuButton::Type::ITEMS:
		texture_asset_id = TEXTURE_ASSET_ID::BUTTON_ITEMS;
		break;
	case MenuButton::Type::RUN:
		texture_asset_id = TEXTURE_ASSET_ID::BUTTON_RUN;
		break;
	case MenuButton::Type::BACK:
		texture_asset_id = TEXTURE_ASSET_ID::BUTTON_BACK;
		break;
	case MenuButton::Type::MAIN_ATTACK:
		texture_asset_id = TEXTURE_ASSET_ID::BATTLE_ATTACK;
		break;
	case MenuButton::Type::PUNCH:
		texture_asset_id = TEXTURE_ASSET_ID::BUTTON_PUNCH;
		break;
    case MenuButton::Type::SHOOT:
        texture_asset_id = TEXTURE_ASSET_ID::BUTTON_SHOOT;
        break;
	case MenuButton::Type::HEAT:
        texture_asset_id = TEXTURE_ASSET_ID::BUTTON_HEAT;
        break;
	default:
		texture_asset_id = TEXTURE_ASSET_ID::BUTTON_RUN;
		break;
	}


	registry.renderRequests.insert(
		entity,
		{ texture_asset_id,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE }); 

	return entity;
}

Entity createVaccine(RenderSystem* renderer, vec2 position)
{
    auto entity = Entity();

    // Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    registry.meshPtrs.emplace(entity, &mesh);

    // Initialize the motion
    auto& motion = registry.motions.emplace(entity);
    motion.angle = 4.25f;
    motion.velocity = { 0.f, 0.f };
    motion.position = position;


    motion.scale = vec2({VACCINE_BB_WIDTH, VACCINE_BB_HEIGHT });
    registry.items.emplace(entity);

    registry.renderRequests.insert(
            entity,
            { TEXTURE_ASSET_ID::VACCINE,
              EFFECT_ASSET_ID::VACCINE,
              GEOMETRY_BUFFER_ID::SPRITE }); 

    return entity;
}

Entity createFireball(RenderSystem* renderer, vec2 position)
{
    auto entity = Entity();

    // Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
    Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
    registry.meshPtrs.emplace(entity, &mesh);

    // Initialize the motion
    auto& motion = registry.motions.emplace(entity);
    motion.angle = 0.f;
    motion.velocity = { 0.f, 0.f };
    motion.position = position;


    motion.scale = vec2({FIREBALL_BB_WIDTH, FIREBALL_BB_HEIGHT });
    registry.items.emplace(entity);

    registry.renderRequests.insert(
            entity,
            { TEXTURE_ASSET_ID::FIREBALL,
              EFFECT_ASSET_ID::FIREBALL,
              GEOMETRY_BUFFER_ID::SPRITE }); 

    return entity;
}

Entity createMouse()
{
	Entity entity = Entity();

	registry.motions.emplace(entity);
	registry.mouses.emplace(entity);

	return entity;
}

Entity createUI()
{
	Entity entity = Entity();

	registry.uis.emplace(entity);

	return entity;
}


