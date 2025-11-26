#include "renderer/billboard_sprite.h"
#include "core/memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

// OpenGL includes - will be replaced with proper graphics headers
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Use math functions from math_types.h

// Normalize angle to 0-360 range
static float normalize_angle(float angle) {
    while (angle < 0.0f) angle += 360.0f;
    while (angle >= 360.0f) angle -= 360.0f;
    return angle;
}

bool sprite_manager_init(SpriteManager* manager) {
    if (!manager) return false;
    
    memset(manager, 0, sizeof(SpriteManager));
    
    printf("Sprite manager initialized\n");
    printf("- 16 horizontal angles (%.1f° per slice)\n", SPRITE_ANGLE_SLICE);
    printf("- 5 elevation bands (TOP/UPR/MID/LWR/BOT)\n");
    printf("- Sprite mirroring optimization enabled\n");
    
    return true;
}

void sprite_manager_destroy(SpriteManager* manager) {
    if (!manager) return;
    
    // TODO: Free OpenGL resources
    // for each collection, free textures
    
    printf("Sprite manager destroyed (%u collections)\n", manager->collection_count);
    memset(manager, 0, sizeof(SpriteManager));
}

SpriteElevation sprite_calculate_elevation(const Vec3* entity_pos, const Vec3* camera_pos) {
    if (!entity_pos || !camera_pos) return SPRITE_ELEVATION_MID;
    
    // Calculate the vector from entity to camera
    Vec3 delta = vec3_sub(*camera_pos, *entity_pos);
    
    // Calculate horizontal distance (ignoring Y component)
    float distance_horizontal = sqrtf(delta.x * delta.x + delta.z * delta.z);
    float height_diff = delta.y;
    
    // Calculate pitch angle in degrees
    float pitch_angle = rad_to_deg(atan2f(height_diff, distance_horizontal));
    
    // Determine elevation band based on pitch thresholds
    if (pitch_angle > 67.5f) {
        return SPRITE_ELEVATION_TOP;
    } else if (pitch_angle > 22.5f) {
        return SPRITE_ELEVATION_UPPER;
    } else if (pitch_angle >= -22.5f) {
        return SPRITE_ELEVATION_MID;
    } else if (pitch_angle >= -67.5f) {
        return SPRITE_ELEVATION_LOWER;
    } else {
        return SPRITE_ELEVATION_BOTTOM;
    }
}

int sprite_calculate_angle_index(const Vec3* entity_pos, const Vec3* entity_forward, const Vec3* camera_pos) {
    if (!entity_pos || !entity_forward || !camera_pos) return 0;
    
    // 1. Get direction from entity to camera
    Vec3 dir_to_cam = vec3_sub(*camera_pos, *entity_pos);
    dir_to_cam = vec3_normalize(dir_to_cam);
    
    // 2. Calculate global angle to camera (in degrees)
    float global_angle_to_cam = rad_to_deg(atan2f(dir_to_cam.x, dir_to_cam.z));
    global_angle_to_cam = normalize_angle(global_angle_to_cam);
    
    // 3. Calculate entity's facing angle (assuming forward vector represents facing)
    float entity_facing_angle = rad_to_deg(atan2f(entity_forward->x, entity_forward->z));
    entity_facing_angle = normalize_angle(entity_facing_angle);
    
    // 4. Calculate relative angle (how the camera appears relative to entity's facing direction)
    float relative_angle = normalize_angle(global_angle_to_cam - entity_facing_angle);
    
    // 5. Offset by half a slice (11.25°) so angle 0 represents center of wedge
    float slice_size = 360.0f / SPRITE_HORIZONTAL_ANGLES;
    float shifted_angle = relative_angle + (slice_size / 2.0f);
    shifted_angle = normalize_angle(shifted_angle);
    
    // 6. Calculate index
    int angle_index = (int)floorf(shifted_angle / slice_size);
    
    // 7. Ensure modulo 16 for wrap-around
    angle_index = angle_index % SPRITE_HORIZONTAL_ANGLES;
    
    return angle_index;
}

SpriteLookupResult sprite_calculate_billboard(const SpriteCollection* collection,
                                             const Vec3* entity_pos,
                                             const Vec3* entity_forward,
                                             const Vec3* camera_pos,
                                             SpriteAction action) {
    SpriteLookupResult result = {0};
    
    if (!collection || !entity_pos || !entity_forward || !camera_pos) {
        return result;
    }
    
    if (action >= SPRITE_ACTION_COUNT) {
        action = SPRITE_ACTION_IDLE;
    }
    
    // Calculate elevation band
    SpriteElevation elevation = sprite_calculate_elevation(entity_pos, camera_pos);
    result.elevation = elevation;
    
    // Calculate angle index
    int angle_index = sprite_calculate_angle_index(entity_pos, entity_forward, camera_pos);
    result.angle_index = angle_index;
    
    // Get the sprite set for this elevation and action
    const SpriteSet* sprite_set = &collection->elevations[elevation][action];
    
    // Handle sprite mirroring optimization
    result.flip_horizontal = false;
    
    if (sprite_set->has_mirroring && angle_index > 8) {
        // Mirror sprites 9-15 using sprites 7-1
        angle_index = 16 - angle_index;
        result.flip_horizontal = true;
    }
    
    // Clamp angle index to valid range
    if (angle_index < 0) angle_index = 0;
    if (angle_index >= SPRITE_HORIZONTAL_ANGLES) angle_index = SPRITE_HORIZONTAL_ANGLES - 1;
    
    // Get the sprite frame
    result.frame = (SpriteFrame*)&sprite_set->frames[angle_index];
    
    return result;
}

bool sprite_parse_filename(const char* filename, char* entity_name, SpriteAction* action,
                          SpriteElevation* elevation, int* angle_index) {
    if (!filename || !entity_name || !action || !elevation || !angle_index) {
        return false;
    }
    
    // Expected format: {EntityName}_{Action}_{Elevation}_{AngleIndex}
    // Example: PLAYER_IDLE_MID_08
    
    char filename_copy[256];
    strncpy(filename_copy, filename, sizeof(filename_copy) - 1);
    filename_copy[sizeof(filename_copy) - 1] = '\0';
    
    // Remove file extension if present
    char* dot = strrchr(filename_copy, '.');
    if (dot) *dot = '\0';
    
    // Parse using strtok
    char* token = strtok(filename_copy, "_");
    if (!token) return false;
    
    // Entity name
    strncpy(entity_name, token, 63);
    entity_name[63] = '\0';
    
    // Action
    token = strtok(NULL, "_");
    if (!token) return false;
    
    if (strcmp(token, "IDLE") == 0) {
        *action = SPRITE_ACTION_IDLE;
    } else if (strcmp(token, "WALK") == 0) {
        *action = SPRITE_ACTION_WALK;
    } else if (strcmp(token, "ATTACK") == 0) {
        *action = SPRITE_ACTION_ATTACK;
    } else if (strcmp(token, "DEATH") == 0) {
        *action = SPRITE_ACTION_DEATH;
    } else {
        return false;
    }
    
    // Elevation
    token = strtok(NULL, "_");
    if (!token) return false;
    
    if (strcmp(token, "TOP") == 0) {
        *elevation = SPRITE_ELEVATION_TOP;
    } else if (strcmp(token, "UPR") == 0) {
        *elevation = SPRITE_ELEVATION_UPPER;
    } else if (strcmp(token, "MID") == 0) {
        *elevation = SPRITE_ELEVATION_MID;
    } else if (strcmp(token, "LWR") == 0) {
        *elevation = SPRITE_ELEVATION_LOWER;
    } else if (strcmp(token, "BOT") == 0) {
        *elevation = SPRITE_ELEVATION_BOTTOM;
    } else {
        return false;
    }
    
    // Angle index
    token = strtok(NULL, "_");
    if (!token) return false;
    
    int parsed_angle = atoi(token);
    if (parsed_angle < 0 || parsed_angle >= SPRITE_HORIZONTAL_ANGLES) {
        return false;
    }
    
    *angle_index = parsed_angle;
    return true;
}

uint32_t sprite_manager_load_collection(SpriteManager* manager, const char* name, const char* base_path) {
    if (!manager || !name || manager->collection_count >= MAX_SPRITE_COLLECTIONS) {
        return UINT32_MAX; // Invalid ID
    }
    
    uint32_t collection_id = manager->collection_count++;
    SpriteCollection* collection = &manager->collections[collection_id];
    
    // Initialize collection
    memset(collection, 0, sizeof(SpriteCollection));
    strncpy(collection->name, name, sizeof(collection->name) - 1);
    collection->scale = 1.0f;
    collection->active = true;
    
    printf("Loading sprite collection '%s' from '%s'\n", name, base_path);
    
    // TODO: Implement actual file loading
    // For now, just create placeholder data
    
    return collection_id;
}

SpriteCollection* sprite_manager_get_collection(SpriteManager* manager, uint32_t collection_id) {
    if (!manager || collection_id >= manager->collection_count) {
        return NULL;
    }
    
    return &manager->collections[collection_id];
}

const char* sprite_elevation_to_string(SpriteElevation elevation) {
    switch (elevation) {
        case SPRITE_ELEVATION_TOP: return "TOP";
        case SPRITE_ELEVATION_UPPER: return "UPR";
        case SPRITE_ELEVATION_MID: return "MID";
        case SPRITE_ELEVATION_LOWER: return "LWR";
        case SPRITE_ELEVATION_BOTTOM: return "BOT";
        default: return "UNKNOWN";
    }
}

const char* sprite_action_to_string(SpriteAction action) {
    switch (action) {
        case SPRITE_ACTION_IDLE: return "IDLE";
        case SPRITE_ACTION_WALK: return "WALK";
        case SPRITE_ACTION_ATTACK: return "ATTACK";
        case SPRITE_ACTION_DEATH: return "DEATH";
        default: return "UNKNOWN";
    }
}

void sprite_manager_print_stats(const SpriteManager* manager) {
    if (!manager) return;
    
    printf("\n=== Sprite Manager Statistics ===\n");
    printf("Collections loaded: %u/%u\n", manager->collection_count, MAX_SPRITE_COLLECTIONS);
    printf("Sprites rendered this frame: %u\n", manager->sprites_rendered_this_frame);
    printf("Sprites culled this frame: %u\n", manager->sprites_culled_this_frame);
    
    for (uint32_t i = 0; i < manager->collection_count; i++) {
        const SpriteCollection* collection = &manager->collections[i];
        if (collection->active) {
            printf("Collection '%s': scale=%.2f\n", collection->name, collection->scale);
        }
    }
    
    printf("=================================\n\n");
}

// Placeholder rendering functions (will be implemented with OpenGL)
void sprite_render_billboard(SpriteManager* manager,
                            const SpriteBillboard* billboard,
                            const Vec3* camera_pos,
                            const Vec3* camera_forward,
                            const Mat4* view_matrix,
                            const Mat4* projection_matrix) {
    // Suppress unused parameter warnings for now
    (void)camera_forward;
    (void)view_matrix;
    (void)projection_matrix;
    
    if (!manager || !billboard || !billboard->visible) return;
    
    SpriteCollection* collection = sprite_manager_get_collection(manager, billboard->collection_id);
    if (!collection || !collection->active) return;
    
    SpriteLookupResult result = sprite_calculate_billboard(
        collection,
        &billboard->world_position,
        &billboard->facing_direction,
        camera_pos,
        billboard->current_action
    );
    
    if (!result.frame || result.frame->texture_id == 0) return;
    
    // TODO: Actual OpenGL rendering
    // - Set up billboard matrix (face camera but maintain world position)
    // - Apply scaling
    // - Set texture
    // - Render quad
    // - Apply horizontal flipping if needed
    
    manager->sprites_rendered_this_frame++;
}

void sprite_manager_render_all(SpriteManager* manager,
                              SpriteBillboard* billboards,
                              uint32_t billboard_count,
                              const Vec3* camera_pos,
                              const Vec3* camera_forward,
                              const Mat4* view_matrix,
                              const Mat4* projection_matrix) {
    if (!manager || !billboards) return;
    
    manager->sprites_rendered_this_frame = 0;
    manager->sprites_culled_this_frame = 0;
    
    for (uint32_t i = 0; i < billboard_count; i++) {
        SpriteBillboard* billboard = &billboards[i];
        
        if (!billboard->visible) {
            manager->sprites_culled_this_frame++;
            continue;
        }
        
        // TODO: Frustum culling
        // TODO: Distance culling
        
        sprite_render_billboard(manager, billboard, camera_pos, camera_forward, 
                               view_matrix, projection_matrix);
    }
}