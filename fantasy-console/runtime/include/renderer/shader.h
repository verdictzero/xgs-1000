#ifndef SHADER_H
#define SHADER_H

#ifdef RASPBERRY_PI
#include <GLES2/gl2.h>
#else
// Desktop fallback - define what we need
typedef float GLfloat;
typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLint;
typedef unsigned char GLubyte;
typedef char GLchar;

#define GL_VERTEX_SHADER   0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS  0x8B81
#define GL_LINK_STATUS     0x8B82
#define GL_VALIDATE_STATUS 0x8B83
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_NO_ERROR        0
#define GL_INVALID_ENUM    0x0500
#define GL_INVALID_VALUE   0x0501
#define GL_INVALID_OPERATION 0x0502
#define GL_OUT_OF_MEMORY   0x0505
#define GL_FALSE           0
#define GL_TRUE            1

// Shader functions
GLuint glCreateShader(GLenum type);
GLuint glCreateProgram(void);
void glShaderSource(GLuint shader, GLint count, const GLchar** string, const GLint* length);
void glCompileShader(GLuint shader);
void glAttachShader(GLuint program, GLuint shader);
void glLinkProgram(GLuint program);
void glUseProgram(GLuint program);
void glDeleteShader(GLuint shader);
void glDeleteProgram(GLuint program);
void glGetShaderiv(GLuint shader, GLenum pname, GLint* params);
void glGetProgramiv(GLuint program, GLenum pname, GLint* params);
void glGetShaderInfoLog(GLuint shader, GLint bufSize, GLint* length, GLchar* infoLog);
void glGetProgramInfoLog(GLuint program, GLint bufSize, GLint* length, GLchar* infoLog);
void glValidateProgram(GLuint program);
GLint glGetUniformLocation(GLuint program, const GLchar* name);
GLint glGetAttribLocation(GLuint program, const GLchar* name);
void glUniformMatrix4fv(GLint location, GLint count, GLubyte transpose, const GLfloat* value);
void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void glUniform1f(GLint location, GLfloat v0);
void glUniform1i(GLint location, GLint v0);
GLenum glGetError(void);

#endif

#include "core/math_types.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t program;
    uint32_t vertex_shader;
    uint32_t fragment_shader;
    
    // Common uniform locations (cached for performance)
    int u_model;
    int u_view;
    int u_projection;
    int u_mvp;
    int u_time;
    int u_resolution;
    
    // Lighting uniforms
    int u_light_pos;
    int u_light_color;
    int u_ambient;
    int u_camera_pos;
    
    // Material uniforms
    int u_texture;
    int u_color;
    int u_opacity;
    
    // Attribute locations
    int a_position;
    int a_normal;
    int a_texcoord;
    int a_color;
    
    bool is_valid;
    char name[64];
} Shader;

typedef enum {
    SHADER_BASIC = 0,
    SHADER_BILLBOARD,
    SHADER_UNLIT,
    SHADER_DEBUG,
    SHADER_COUNT
} ShaderType;

typedef struct {
    Shader shaders[SHADER_COUNT];
    bool initialized;
} ShaderManager;

// Shader compilation and management
bool shader_create(Shader* shader, const char* vertex_src, const char* fragment_src, const char* name);
void shader_destroy(Shader* shader);

// Shader usage
void shader_use(const Shader* shader);
void shader_validate(const Shader* shader);

// Uniform setters
void shader_set_mat4(const Shader* shader, const char* name, const Mat4* matrix);
void shader_set_vec3(const Shader* shader, const char* name, Vec3 v);
void shader_set_vec4(const Shader* shader, const char* name, Vec4 v);
void shader_set_float(const Shader* shader, const char* name, float value);
void shader_set_int(const Shader* shader, const char* name, int value);

// Optimized uniform setters using cached locations
void shader_set_mat4_cached(const Shader* shader, int location, const Mat4* matrix);
void shader_set_vec3_cached(const Shader* shader, int location, Vec3 v);
void shader_set_float_cached(const Shader* shader, int location, float value);
void shader_set_int_cached(const Shader* shader, int location, int value);

// Shader manager
bool shader_manager_init(ShaderManager* manager);
void shader_manager_destroy(ShaderManager* manager);
Shader* shader_manager_get(ShaderManager* manager, ShaderType type);

// Built-in shaders
bool shader_manager_load_builtin_shaders(ShaderManager* manager);

// Utility functions
int shader_get_uniform_location(const Shader* shader, const char* name);
int shader_get_attribute_location(const Shader* shader, const char* name);
void shader_cache_locations(Shader* shader);

// Shader compilation helpers
uint32_t shader_compile_source(uint32_t type, const char* source);
bool shader_link_program(uint32_t program);
void shader_print_log(uint32_t shader_or_program, bool is_program);

// Built-in shader source code
extern const char* SHADER_BASIC_VERTEX;
extern const char* SHADER_BASIC_FRAGMENT;
extern const char* SHADER_BILLBOARD_VERTEX;
extern const char* SHADER_BILLBOARD_FRAGMENT;
extern const char* SHADER_UNLIT_VERTEX;
extern const char* SHADER_UNLIT_FRAGMENT;

#ifdef __cplusplus
}
#endif

#endif // SHADER_H