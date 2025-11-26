#include "core/math_types.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// === Vec3 Operations ===

Vec3 vec3_add(Vec3 a, Vec3 b) {
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 vec3_mul(Vec3 v, float s) {
    return (Vec3){v.x * s, v.y * s, v.z * s};
}

Vec3 vec3_div(Vec3 v, float s) {
    if (s == 0.0f) return VEC3_ZERO;
    float inv_s = 1.0f / s;
    return (Vec3){v.x * inv_s, v.y * inv_s, v.z * inv_s};
}

float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

float vec3_length_squared(Vec3 v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

float vec3_length(Vec3 v) {
    return sqrtf(vec3_length_squared(v));
}

Vec3 vec3_normalize(Vec3 v) {
    float len = vec3_length(v);
    if (len == 0.0f) return VEC3_ZERO;
    return vec3_div(v, len);
}

Vec3 vec3_lerp(Vec3 a, Vec3 b, float t) {
    return vec3_add(vec3_mul(a, 1.0f - t), vec3_mul(b, t));
}

float vec3_distance(Vec3 a, Vec3 b) {
    return vec3_length(vec3_sub(a, b));
}

// === Vec4 Operations ===

Vec4 vec4_add(Vec4 a, Vec4 b) {
    return (Vec4){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

Vec4 vec4_sub(Vec4 a, Vec4 b) {
    return (Vec4){a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

Vec4 vec4_mul(Vec4 v, float s) {
    return (Vec4){v.x * s, v.y * s, v.z * s, v.w * s};
}

float vec4_dot(Vec4 a, Vec4 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float vec4_length(Vec4 v) {
    return sqrtf(vec4_dot(v, v));
}

Vec4 vec4_normalize(Vec4 v) {
    float len = vec4_length(v);
    if (len == 0.0f) return (Vec4){0, 0, 0, 0};
    return vec4_mul(v, 1.0f / len);
}

// === Mat4 Operations ===

Mat4 mat4_identity(void) {
    Mat4 m;
    memset(&m, 0, sizeof(Mat4));
    m.m[0] = 1.0f;   // [0][0]
    m.m[5] = 1.0f;   // [1][1]
    m.m[10] = 1.0f;  // [2][2]
    m.m[15] = 1.0f;  // [3][3]
    return m;
}

Mat4 mat4_zero(void) {
    Mat4 m;
    memset(&m, 0, sizeof(Mat4));
    return m;
}

Mat4 mat4_perspective(float fov_y, float aspect, float near, float far) {
    Mat4 m = mat4_zero();
    
    float tan_half_fovy = tanf(fov_y * 0.5f);
    
    m.m[0] = 1.0f / (aspect * tan_half_fovy);  // [0][0]
    m.m[5] = 1.0f / tan_half_fovy;             // [1][1]
    m.m[10] = -(far + near) / (far - near);    // [2][2]
    m.m[11] = -1.0f;                           // [2][3]
    m.m[14] = -(2.0f * far * near) / (far - near); // [3][2]
    
    return m;
}

Mat4 mat4_orthographic(float left, float right, float bottom, float top, float near, float far) {
    Mat4 m = mat4_zero();
    
    m.m[0] = 2.0f / (right - left);     // [0][0]
    m.m[5] = 2.0f / (top - bottom);     // [1][1]
    m.m[10] = -2.0f / (far - near);     // [2][2]
    m.m[12] = -(right + left) / (right - left);   // [3][0]
    m.m[13] = -(top + bottom) / (top - bottom);   // [3][1]
    m.m[14] = -(far + near) / (far - near);       // [3][2]
    m.m[15] = 1.0f;                     // [3][3]
    
    return m;
}

Mat4 mat4_look_at(Vec3 eye, Vec3 target, Vec3 up) {
    Vec3 f = vec3_normalize(vec3_sub(target, eye));  // forward
    Vec3 s = vec3_normalize(vec3_cross(f, up));      // right
    Vec3 u = vec3_cross(s, f);                       // up
    
    Mat4 m = mat4_identity();
    
    m.m[0] = s.x;                    // [0][0]
    m.m[4] = s.y;                    // [1][0]
    m.m[8] = s.z;                    // [2][0]
    m.m[1] = u.x;                    // [0][1]
    m.m[5] = u.y;                    // [1][1]
    m.m[9] = u.z;                    // [2][1]
    m.m[2] = -f.x;                   // [0][2]
    m.m[6] = -f.y;                   // [1][2]
    m.m[10] = -f.z;                  // [2][2]
    m.m[12] = -vec3_dot(s, eye);     // [3][0]
    m.m[13] = -vec3_dot(u, eye);     // [3][1]
    m.m[14] = vec3_dot(f, eye);      // [3][2]
    
    return m;
}

Mat4 mat4_translate(Vec3 translation) {
    Mat4 m = mat4_identity();
    m.m[12] = translation.x;  // [3][0]
    m.m[13] = translation.y;  // [3][1]
    m.m[14] = translation.z;  // [3][2]
    return m;
}

Mat4 mat4_scale(Vec3 scale) {
    Mat4 m = mat4_zero();
    m.m[0] = scale.x;   // [0][0]
    m.m[5] = scale.y;   // [1][1]
    m.m[10] = scale.z;  // [2][2]
    m.m[15] = 1.0f;     // [3][3]
    return m;
}

Mat4 mat4_rotate_x(float angle_radians) {
    Mat4 m = mat4_identity();
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);
    
    m.m[5] = c;   // [1][1]
    m.m[6] = s;   // [1][2]
    m.m[9] = -s;  // [2][1]
    m.m[10] = c;  // [2][2]
    
    return m;
}

Mat4 mat4_rotate_y(float angle_radians) {
    Mat4 m = mat4_identity();
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);
    
    m.m[0] = c;    // [0][0]
    m.m[2] = -s;   // [0][2]
    m.m[8] = s;    // [2][0]
    m.m[10] = c;   // [2][2]
    
    return m;
}

Mat4 mat4_rotate_z(float angle_radians) {
    Mat4 m = mat4_identity();
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);
    
    m.m[0] = c;   // [0][0]
    m.m[1] = s;   // [0][1]
    m.m[4] = -s;  // [1][0]
    m.m[5] = c;   // [1][1]
    
    return m;
}

Mat4 mat4_mul(Mat4 a, Mat4 b) {
    Mat4 result = mat4_zero();
    
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            float sum = 0.0f;
            for (int k = 0; k < 4; k++) {
                sum += a.m[row + k * 4] * b.m[k + col * 4];
            }
            result.m[row + col * 4] = sum;
        }
    }
    
    return result;
}

Vec4 mat4_mul_vec4(Mat4 m, Vec4 v) {
    return (Vec4){
        m.m[0] * v.x + m.m[4] * v.y + m.m[8] * v.z + m.m[12] * v.w,
        m.m[1] * v.x + m.m[5] * v.y + m.m[9] * v.z + m.m[13] * v.w,
        m.m[2] * v.x + m.m[6] * v.y + m.m[10] * v.z + m.m[14] * v.w,
        m.m[3] * v.x + m.m[7] * v.y + m.m[11] * v.z + m.m[15] * v.w
    };
}

Vec3 mat4_mul_vec3(Mat4 m, Vec3 v) {
    Vec4 v4 = {v.x, v.y, v.z, 1.0f};
    Vec4 result = mat4_mul_vec4(m, v4);
    return (Vec3){result.x, result.y, result.z};
}

// === Quaternion Operations ===

Quat quat_identity(void) {
    return (Quat){0.0f, 0.0f, 0.0f, 1.0f};
}

Quat quat_from_axis_angle(Vec3 axis, float angle_radians) {
    Vec3 normalized_axis = vec3_normalize(axis);
    float half_angle = angle_radians * 0.5f;
    float s = sinf(half_angle);
    
    return (Quat){
        normalized_axis.x * s,
        normalized_axis.y * s,
        normalized_axis.z * s,
        cosf(half_angle)
    };
}

Quat quat_mul(Quat a, Quat b) {
    return (Quat){
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z,
        a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    };
}

Quat quat_normalize(Quat q) {
    float len = sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    if (len == 0.0f) return quat_identity();
    
    float inv_len = 1.0f / len;
    return (Quat){q.x * inv_len, q.y * inv_len, q.z * inv_len, q.w * inv_len};
}

// === Utility Functions ===

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float deg_to_rad(float degrees) {
    return degrees * (M_PI / 180.0f);
}

float rad_to_deg(float radians) {
    return radians * (180.0f / M_PI);
}

float sin_deg(float degrees) {
    return sinf(deg_to_rad(degrees));
}

float cos_deg(float degrees) {
    return cosf(deg_to_rad(degrees));
}

float tan_deg(float degrees) {
    return tanf(deg_to_rad(degrees));
}