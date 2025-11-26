#include "include/core/math_types.h"
#include "include/renderer/billboard_sprite.h"
#include <stdio.h>
#include <math.h>

int main() {
    Vec3 entity_pos = {0, 0, 0};
    Vec3 entity_forward = {0, 0, 1}; // Facing north (positive Z)
    Vec3 camera_pos;
    
    printf("=== Testing different camera positions ===\n");
    
    // Test camera to the west (left)
    camera_pos = (Vec3){-5, 0, 0};
    printf("\nCamera west of entity:\n");
    
    int angle = sprite_calculate_angle_index(&entity_pos, &entity_forward, &camera_pos);
    printf("Angle for camera west: %d\n", angle);
    
    // Test camera to the east (right)
    camera_pos = (Vec3){5, 0, 0};
    angle = sprite_calculate_angle_index(&entity_pos, &entity_forward, &camera_pos);
    printf("Angle for camera east: %d\n", angle);
    
    // Test camera to the north (behind)
    camera_pos = (Vec3){0, 0, 5};
    angle = sprite_calculate_angle_index(&entity_pos, &entity_forward, &camera_pos);
    printf("Angle for camera north (behind): %d\n", angle);
    
    // Test camera to the south (front)  
    camera_pos = (Vec3){0, 0, -5};
    angle = sprite_calculate_angle_index(&entity_pos, &entity_forward, &camera_pos);
    printf("Angle for camera south (front): %d\n", angle);
    
    printf("\nAccording to spec:\n");
    printf("00 = Facing Camera (South)\n");
    printf("04 = Facing Left (West)\n"); 
    printf("08 = Facing Away (North)\n");
    printf("12 = Facing Right (East)\n");
    
    // Let's debug step by step
    Vec3 dir_to_cam = vec3_sub(camera_pos, entity_pos);
    dir_to_cam = vec3_normalize(dir_to_cam);
    printf("Direction to camera: (%.3f, %.3f, %.3f)\n", dir_to_cam.x, dir_to_cam.y, dir_to_cam.z);
    
    float global_angle_to_cam = rad_to_deg(atan2f(dir_to_cam.x, dir_to_cam.z));
    printf("Global angle to camera: %.2f degrees\n", global_angle_to_cam);
    
    float entity_facing_angle = rad_to_deg(atan2f(entity_forward.x, entity_forward.z));
    printf("Entity facing angle: %.2f degrees\n", entity_facing_angle);
    
    float relative_angle = global_angle_to_cam - entity_facing_angle;
    printf("Relative angle (raw): %.2f degrees\n", relative_angle);
    
    // Normalize to 0-360
    while (relative_angle < 0.0f) relative_angle += 360.0f;
    while (relative_angle >= 360.0f) relative_angle -= 360.0f;
    printf("Relative angle (normalized): %.2f degrees\n", relative_angle);
    
    float slice_size = 360.0f / 16.0f;
    float shifted_angle = relative_angle + (slice_size / 2.0f);
    while (shifted_angle < 0.0f) shifted_angle += 360.0f;
    while (shifted_angle >= 360.0f) shifted_angle -= 360.0f;
    printf("Shifted angle: %.2f degrees (slice size: %.2f)\n", shifted_angle, slice_size);
    
    int final_angle = (int)floorf(shifted_angle / slice_size);
    final_angle = final_angle % 16;
    printf("Final angle index: %d\n", final_angle);
    
    return 0;
}