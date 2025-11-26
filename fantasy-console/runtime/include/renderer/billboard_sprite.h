#ifndef BILLBOARD_SPRITE_H
#define BILLBOARD_SPRITE_H

#include "core/math_types.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 16-Angle Spherical Billboarding System
// Based on Doom-style sprites extended to full 3D viewing

#define SPRITE_HORIZONTAL_ANGLES 16
#define SPRITE_ANGLE_SLICE 22.5f  // 360 / 16 = 22.5 degrees per slice

typedef enum {
    SPRITE_ELEVATION_TOP = 0,   // +90° (Zenith)
    SPRITE_ELEVATION_UPPER = 1, // +45° (Upper Ring)
    SPRITE_ELEVATION_MID = 2,   // 0°   (Mid Ring)
    SPRITE_ELEVATION_LOWER = 3, // -45° (Lower Ring)
    SPRITE_ELEVATION_BOTTOM = 4, // -90° (Nadir)
    SPRITE_ELEVATION_COUNT = 5
} SpriteElevation;

typedef enum {
    SPRITE_ACTION_IDLE = 0,
    SPRITE_ACTION_WALK = 1,
    SPRITE_ACTION_ATTACK = 2,
    SPRITE_ACTION_DEATH = 3,
    SPRITE_ACTION_COUNT = 4
} SpriteAction;

// Single sprite frame
typedef struct {
    uint32_t texture_id;    // OpenGL texture ID
    uint16_t width;         // Sprite width in pixels
    uint16_t height;        // Sprite height in pixels
    float pivot_x;          // Pivot point (0.0-1.0)
    float pivot_y;          // Pivot point (0.0-1.0)
} SpriteFrame;

// Sprite set for one action at one elevation
typedef struct {
    SpriteFrame frames[SPRITE_HORIZONTAL_ANGLES];
    bool has_mirroring;     // If true, angles 9-15 are mirrors of 7-1
} SpriteSet;

// Complete sprite collection for one entity
typedef struct {
    SpriteSet elevations[SPRITE_ELEVATION_COUNT][SPRITE_ACTION_COUNT];
    char name[64];          // Entity name for debugging
    float scale;            // Global scale multiplier
    bool active;            // Is this sprite collection in use
} SpriteCollection;

// Result of sprite lookup
typedef struct {
    SpriteFrame* frame;     // Pointer to the sprite frame to render
    bool flip_horizontal;   // Should sprite be mirrored horizontally?
    SpriteElevation elevation; // Which elevation band was selected
    int angle_index;        // Which angle index (0-15)
} SpriteLookupResult;

// Sprite billboard component for entities
typedef struct {
    uint32_t collection_id; // Index into sprite manager's collections
    SpriteAction current_action;
    float animation_time;   // For animated sprites (future)
    Vec3 world_position;    // World position for billboard calculation
    Vec3 facing_direction;  // Which way the entity is facing
    float scale_override;   // Per-instance scale (multiplies collection scale)
    bool visible;           // Render this sprite?
} SpriteBillboard;

// Sprite manager - handles all sprite collections
#define MAX_SPRITE_COLLECTIONS 256

typedef struct {
    SpriteCollection collections[MAX_SPRITE_COLLECTIONS];
    uint32_t collection_count;
    
    // Rendering state
    uint32_t billboard_shader_program;
    uint32_t quad_vao;
    uint32_t quad_vbo;
    
    // Performance stats
    uint32_t sprites_rendered_this_frame;
    uint32_t sprites_culled_this_frame;
} SpriteManager;

// Initialize sprite system
bool sprite_manager_init(SpriteManager* manager);

// Destroy sprite system
void sprite_manager_destroy(SpriteManager* manager);

// Load a sprite collection from assets
uint32_t sprite_manager_load_collection(SpriteManager* manager, const char* name, const char* base_path);

// Get sprite collection by ID
SpriteCollection* sprite_manager_get_collection(SpriteManager* manager, uint32_t collection_id);

// Core billboard calculation
SpriteLookupResult sprite_calculate_billboard(const SpriteCollection* collection, 
                                             const Vec3* entity_pos,
                                             const Vec3* entity_forward,
                                             const Vec3* camera_pos,
                                             SpriteAction action);

// Render a single sprite billboard
void sprite_render_billboard(SpriteManager* manager, 
                           const SpriteBillboard* billboard,
                           const Vec3* camera_pos,
                           const Vec3* camera_forward,
                           const Mat4* view_matrix,
                           const Mat4* projection_matrix);

// Render all visible sprite billboards in scene
void sprite_manager_render_all(SpriteManager* manager,
                              SpriteBillboard* billboards,
                              uint32_t billboard_count,
                              const Vec3* camera_pos,
                              const Vec3* camera_forward,
                              const Mat4* view_matrix,
                              const Mat4* projection_matrix);

// Utility functions
SpriteElevation sprite_calculate_elevation(const Vec3* entity_pos, const Vec3* camera_pos);
int sprite_calculate_angle_index(const Vec3* entity_pos, const Vec3* entity_forward, const Vec3* camera_pos);

// Asset loading helpers
bool sprite_load_texture_from_file(const char* filepath, uint32_t* texture_id, uint16_t* width, uint16_t* height);
bool sprite_parse_filename(const char* filename, char* entity_name, SpriteAction* action, 
                          SpriteElevation* elevation, int* angle_index);

// Debug functions
void sprite_manager_print_stats(const SpriteManager* manager);
const char* sprite_elevation_to_string(SpriteElevation elevation);
const char* sprite_action_to_string(SpriteAction action);

#ifdef __cplusplus
}
#endif

#endif // BILLBOARD_SPRITE_H