#ifndef MATH_TYPES_H
#define MATH_TYPES_H

#include <stdint.h>

#ifdef __ARM_NEON
#include <arm_neon.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// 3D Vector
typedef struct {
    float x, y, z;
} Vec3;

// 4D Vector  
typedef struct {
    float x, y, z, w;
} Vec4;

// Quaternion
typedef struct {
    float x, y, z, w;
} Quat;

// 4x4 Matrix (column-major for OpenGL)
typedef struct {
    float m[16];
} Mat4;

// 2D Vector
typedef struct {
    float x, y;
} Vec2;

// Integer 2D Vector
typedef struct {
    int x, y;
} Vec2i;

// === Vec3 Operations ===
Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_sub(Vec3 a, Vec3 b);
Vec3 vec3_mul(Vec3 v, float s);
Vec3 vec3_div(Vec3 v, float s);
float vec3_dot(Vec3 a, Vec3 b);
Vec3 vec3_cross(Vec3 a, Vec3 b);
float vec3_length(Vec3 v);
float vec3_length_squared(Vec3 v);
Vec3 vec3_normalize(Vec3 v);
Vec3 vec3_lerp(Vec3 a, Vec3 b, float t);
float vec3_distance(Vec3 a, Vec3 b);

// Vec3 constants
static const Vec3 VEC3_ZERO = {0.0f, 0.0f, 0.0f};
static const Vec3 VEC3_ONE = {1.0f, 1.0f, 1.0f};
static const Vec3 VEC3_UP = {0.0f, 1.0f, 0.0f};
static const Vec3 VEC3_DOWN = {0.0f, -1.0f, 0.0f};
static const Vec3 VEC3_FORWARD = {0.0f, 0.0f, 1.0f};
static const Vec3 VEC3_BACK = {0.0f, 0.0f, -1.0f};
static const Vec3 VEC3_RIGHT = {1.0f, 0.0f, 0.0f};
static const Vec3 VEC3_LEFT = {-1.0f, 0.0f, 0.0f};

// === Vec4 Operations ===
Vec4 vec4_add(Vec4 a, Vec4 b);
Vec4 vec4_sub(Vec4 a, Vec4 b);
Vec4 vec4_mul(Vec4 v, float s);
float vec4_dot(Vec4 a, Vec4 b);
float vec4_length(Vec4 v);
Vec4 vec4_normalize(Vec4 v);

// === Mat4 Operations ===
Mat4 mat4_identity(void);
Mat4 mat4_zero(void);
Mat4 mat4_perspective(float fov_y, float aspect, float near, float far);
Mat4 mat4_orthographic(float left, float right, float bottom, float top, float near, float far);
Mat4 mat4_look_at(Vec3 eye, Vec3 target, Vec3 up);
Mat4 mat4_translate(Vec3 translation);
Mat4 mat4_rotate_x(float angle_radians);
Mat4 mat4_rotate_y(float angle_radians);
Mat4 mat4_rotate_z(float angle_radians);
Mat4 mat4_rotate(Quat quaternion);
Mat4 mat4_scale(Vec3 scale);
Mat4 mat4_mul(Mat4 a, Mat4 b);
Vec4 mat4_mul_vec4(Mat4 m, Vec4 v);
Vec3 mat4_mul_vec3(Mat4 m, Vec3 v); // Treats Vec3 as Vec4 with w=1
Mat4 mat4_inverse(Mat4 m);
Mat4 mat4_transpose(Mat4 m);

// === Quaternion Operations ===
Quat quat_identity(void);
Quat quat_from_axis_angle(Vec3 axis, float angle_radians);
Quat quat_from_euler(float pitch, float yaw, float roll);
Quat quat_mul(Quat a, Quat b);
Quat quat_normalize(Quat q);
Mat4 quat_to_mat4(Quat q);
Vec3 quat_rotate_vec3(Quat q, Vec3 v);
Quat quat_slerp(Quat a, Quat b, float t);
float quat_dot(Quat a, Quat b);

// === Utility Functions ===
float lerp(float a, float b, float t);
float clamp(float value, float min, float max);
float deg_to_rad(float degrees);
float rad_to_deg(float radians);
float sin_deg(float degrees);
float cos_deg(float degrees);
float tan_deg(float degrees);

// Fast approximations (useful for ARM without FPU)
float fast_sqrt(float x);
float fast_inv_sqrt(float x);
float fast_sin(float x);
float fast_cos(float x);

// SIMD/NEON optimized versions (when available)
#ifdef HAS_NEON
Vec3 vec3_add_neon(Vec3 a, Vec3 b);
Vec3 vec3_sub_neon(Vec3 a, Vec3 b);
Vec3 vec3_mul_neon(Vec3 v, float s);
float vec3_dot_neon(Vec3 a, Vec3 b);
Vec3 vec3_cross_neon(Vec3 a, Vec3 b);
Mat4 mat4_mul_neon(Mat4 a, Mat4 b);
#endif

#ifdef __cplusplus
}
#endif

#endif // MATH_TYPES_H