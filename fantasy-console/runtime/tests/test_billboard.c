#include "renderer/billboard_sprite.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

void test_elevation_calculation() {
    Vec3 entity_pos = {0, 0, 0};
    Vec3 camera_pos;
    
    // Test TOP elevation (camera high above)
    camera_pos = (Vec3){0, 10, 1}; // Very high camera
    SpriteElevation elevation = sprite_calculate_elevation(&entity_pos, &camera_pos);
    assert(elevation == SPRITE_ELEVATION_TOP);
    
    // Test MID elevation (camera at same level)
    camera_pos = (Vec3){0, 0, 10}; // Same height, distance away
    elevation = sprite_calculate_elevation(&entity_pos, &camera_pos);
    assert(elevation == SPRITE_ELEVATION_MID);
    
    // Test BOTTOM elevation (camera far below)
    camera_pos = (Vec3){0, -10, 1}; // Very low camera
    elevation = sprite_calculate_elevation(&entity_pos, &camera_pos);
    assert(elevation == SPRITE_ELEVATION_BOTTOM);
    
    // Test UPPER elevation
    camera_pos = (Vec3){0, 5, 5}; // 45° above
    elevation = sprite_calculate_elevation(&entity_pos, &camera_pos);
    assert(elevation == SPRITE_ELEVATION_UPPER);
    
    // Test LOWER elevation
    camera_pos = (Vec3){0, -5, 5}; // 45° below
    elevation = sprite_calculate_elevation(&entity_pos, &camera_pos);
    assert(elevation == SPRITE_ELEVATION_LOWER);
    
    printf("✓ Elevation calculation test passed\n");
}

void test_angle_calculation() {
    Vec3 entity_pos = {0, 0, 0};
    Vec3 entity_forward = {0, 0, 1}; // Facing north (positive Z)
    Vec3 camera_pos;
    
    // Test angle 0 - camera behind entity (entity facing towards camera)
    camera_pos = (Vec3){0, 0, 5}; // Camera north of entity (entity faces toward camera)
    int angle = sprite_calculate_angle_index(&entity_pos, &entity_forward, &camera_pos);
    assert(angle == 0); // Should be facing camera
    
    // Test angle 8 - camera in front of entity (entity facing away from camera)
    camera_pos = (Vec3){0, 0, -5}; // Camera south of entity (entity faces away)
    angle = sprite_calculate_angle_index(&entity_pos, &entity_forward, &camera_pos);
    assert(angle == 8); // Should be facing away
    
    // Test angle 12 - camera to the west (entity facing right from camera's perspective)
    camera_pos = (Vec3){-5, 0, 0}; // Camera west of entity
    angle = sprite_calculate_angle_index(&entity_pos, &entity_forward, &camera_pos);
    assert(angle == 12); // Entity appears to face right from camera's view
    
    // Test angle 4 - camera to the east (entity facing left from camera's perspective)
    camera_pos = (Vec3){5, 0, 0}; // Camera east of entity
    angle = sprite_calculate_angle_index(&entity_pos, &entity_forward, &camera_pos);
    assert(angle == 4); // Entity appears to face left from camera's view
    
    printf("✓ Angle calculation test passed\n");
}

void test_filename_parsing() {
    char entity_name[64];
    SpriteAction action;
    SpriteElevation elevation;
    int angle_index;
    
    // Test valid filename
    bool result = sprite_parse_filename("PLAYER_IDLE_MID_08", entity_name, &action, &elevation, &angle_index);
    assert(result == true);
    assert(strcmp(entity_name, "PLAYER") == 0);
    assert(action == SPRITE_ACTION_IDLE);
    assert(elevation == SPRITE_ELEVATION_MID);
    assert(angle_index == 8);
    
    // Test filename with extension
    result = sprite_parse_filename("ENEMY_WALK_UPR_12.png", entity_name, &action, &elevation, &angle_index);
    assert(result == true);
    assert(strcmp(entity_name, "ENEMY") == 0);
    assert(action == SPRITE_ACTION_WALK);
    assert(elevation == SPRITE_ELEVATION_UPPER);
    assert(angle_index == 12);
    
    // Test invalid filename
    result = sprite_parse_filename("INVALID_FORMAT", entity_name, &action, &elevation, &angle_index);
    assert(result == false);
    
    // Test invalid action
    result = sprite_parse_filename("PLAYER_JUMP_MID_08", entity_name, &action, &elevation, &angle_index);
    assert(result == false);
    
    // Test invalid elevation
    result = sprite_parse_filename("PLAYER_IDLE_SIDE_08", entity_name, &action, &elevation, &angle_index);
    assert(result == false);
    
    // Test invalid angle
    result = sprite_parse_filename("PLAYER_IDLE_MID_99", entity_name, &action, &elevation, &angle_index);
    assert(result == false);
    
    printf("✓ Filename parsing test passed\n");
}

void test_sprite_manager() {
    SpriteManager manager;
    assert(sprite_manager_init(&manager));
    
    // Test loading collection
    uint32_t collection_id = sprite_manager_load_collection(&manager, "test_player", "assets/sprites/");
    assert(collection_id != UINT32_MAX);
    
    // Test getting collection
    SpriteCollection* collection = sprite_manager_get_collection(&manager, collection_id);
    assert(collection != NULL);
    assert(strcmp(collection->name, "test_player") == 0);
    assert(collection->active == true);
    
    // Test invalid collection ID
    SpriteCollection* invalid_collection = sprite_manager_get_collection(&manager, 999);
    assert(invalid_collection == NULL);
    
    sprite_manager_destroy(&manager);
    printf("✓ Sprite manager test passed\n");
}

void test_billboard_calculation() {
    // Create test sprite collection
    SpriteCollection collection;
    memset(&collection, 0, sizeof(SpriteCollection));
    strcpy(collection.name, "test");
    collection.active = true;
    collection.scale = 1.0f;
    
    // Initialize one sprite set with mirroring
    collection.elevations[SPRITE_ELEVATION_MID][SPRITE_ACTION_IDLE].has_mirroring = true;
    
    Vec3 entity_pos = {0, 0, 0};
    Vec3 entity_forward = {0, 0, 1}; // Facing north
    Vec3 camera_pos = {-5, 0, 0}; // Camera to the west
    
    SpriteLookupResult result = sprite_calculate_billboard(
        &collection, &entity_pos, &entity_forward, &camera_pos, SPRITE_ACTION_IDLE
    );
    
    printf("Debug billboard test: angle_index=%d, flip=%d\n", result.angle_index, result.flip_horizontal);
    
    assert(result.elevation == SPRITE_ELEVATION_MID);
    assert(result.angle_index == 4); // Angle 12 gets mirrored to 4 (16-12=4)
    assert(result.flip_horizontal == true); // Should be mirrored
    
    // Test mirroring case
    camera_pos = (Vec3){-5, 0, -5}; // Camera to the southwest (angle 14)
    result = sprite_calculate_billboard(
        &collection, &entity_pos, &entity_forward, &camera_pos, SPRITE_ACTION_IDLE
    );
    
    // Angle 14 should be mirrored to angle 2
    assert(result.flip_horizontal == true);
    
    printf("✓ Billboard calculation test passed\n");
}

void test_utility_functions() {
    // Test string conversion functions
    assert(strcmp(sprite_elevation_to_string(SPRITE_ELEVATION_TOP), "TOP") == 0);
    assert(strcmp(sprite_elevation_to_string(SPRITE_ELEVATION_MID), "MID") == 0);
    assert(strcmp(sprite_elevation_to_string(SPRITE_ELEVATION_BOTTOM), "BOT") == 0);
    
    assert(strcmp(sprite_action_to_string(SPRITE_ACTION_IDLE), "IDLE") == 0);
    assert(strcmp(sprite_action_to_string(SPRITE_ACTION_WALK), "WALK") == 0);
    assert(strcmp(sprite_action_to_string(SPRITE_ACTION_ATTACK), "ATTACK") == 0);
    
    printf("✓ Utility functions test passed\n");
}

void run_billboard_tests() {
    printf("Running billboard sprite system tests...\n");
    
    test_elevation_calculation();
    test_angle_calculation();
    test_filename_parsing();
    test_sprite_manager();
    test_billboard_calculation();
    test_utility_functions();
    
    printf("All billboard sprite tests passed!\n\n");
}