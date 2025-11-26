#include "renderer/shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Built-in shader sources
const char* SHADER_BASIC_VERTEX = 
"#version 100\n"
"precision mediump float;\n"
"\n"
"attribute vec3 a_position;\n"
"attribute vec3 a_normal;\n"
"attribute vec2 a_texcoord;\n"
"\n"
"uniform mat4 u_model;\n"
"uniform mat4 u_view;\n"
"uniform mat4 u_projection;\n"
"\n"
"varying vec3 v_normal;\n"
"varying vec2 v_texcoord;\n"
"varying vec3 v_world_pos;\n"
"\n"
"void main() {\n"
"    vec4 world_pos = u_model * vec4(a_position, 1.0);\n"
"    v_world_pos = world_pos.xyz;\n"
"    v_normal = mat3(u_model) * a_normal;\n"
"    v_texcoord = a_texcoord;\n"
"    gl_Position = u_projection * u_view * world_pos;\n"
"}\n";

const char* SHADER_BASIC_FRAGMENT = 
"#version 100\n"
"precision mediump float;\n"
"\n"
"varying vec3 v_normal;\n"
"varying vec2 v_texcoord;\n"
"varying vec3 v_world_pos;\n"
"\n"
"uniform sampler2D u_texture;\n"
"uniform vec3 u_light_pos;\n"
"uniform vec3 u_light_color;\n"
"uniform vec3 u_ambient;\n"
"\n"
"void main() {\n"
"    // Simple directional lighting\n"
"    vec3 normal = normalize(v_normal);\n"
"    vec3 light_dir = normalize(u_light_pos - v_world_pos);\n"
"    float diff = max(dot(normal, light_dir), 0.0);\n"
"\n"
"    vec3 lighting = u_ambient + u_light_color * diff;\n"
"\n"
"    // Point-sampled texture (retro style)\n"
"    vec4 tex_color = texture2D(u_texture, v_texcoord);\n"
"\n"
"    gl_FragColor = vec4(lighting * tex_color.rgb, tex_color.a);\n"
"\n"
"    // Alpha test for sprite cutout\n"
"    if (gl_FragColor.a < 0.5) {\n"
"        discard;\n"
"    }\n"
"}\n";

const char* SHADER_BILLBOARD_VERTEX = 
"#version 100\n"
"precision mediump float;\n"
"\n"
"attribute vec3 a_position;\n"
"attribute vec2 a_texcoord;\n"
"\n"
"uniform mat4 u_view;\n"
"uniform mat4 u_projection;\n"
"uniform vec3 u_billboard_pos;\n"
"uniform vec2 u_billboard_size;\n"
"\n"
"varying vec2 v_texcoord;\n"
"\n"
"void main() {\n"
"    // Create billboard matrix (always face camera)\n"
"    vec3 right = vec3(u_view[0][0], u_view[1][0], u_view[2][0]);\n"
"    vec3 up = vec3(u_view[0][1], u_view[1][1], u_view[2][1]);\n"
"\n"
"    vec3 billboard_pos = u_billboard_pos +\n"
"        right * a_position.x * u_billboard_size.x +\n"
"        up * a_position.y * u_billboard_size.y;\n"
"\n"
"    v_texcoord = a_texcoord;\n"
"    gl_Position = u_projection * u_view * vec4(billboard_pos, 1.0);\n"
"}\n";

const char* SHADER_BILLBOARD_FRAGMENT = 
"#version 100\n"
"precision mediump float;\n"
"\n"
"varying vec2 v_texcoord;\n"
"\n"
"uniform sampler2D u_texture;\n"
"uniform vec4 u_color;\n"
"uniform float u_flip_horizontal;\n"
"\n"
"void main() {\n"
"    vec2 texcoord = v_texcoord;\n"
"    \n"
"    // Handle horizontal flipping for sprite mirroring\n"
"    if (u_flip_horizontal > 0.5) {\n"
"        texcoord.x = 1.0 - texcoord.x;\n"
"    }\n"
"    \n"
"    vec4 tex_color = texture2D(u_texture, texcoord);\n"
"    gl_FragColor = tex_color * u_color;\n"
"\n"
"    // Alpha test\n"
"    if (gl_FragColor.a < 0.1) {\n"
"        discard;\n"
"    }\n"
"}\n";

const char* SHADER_UNLIT_VERTEX = 
"#version 100\n"
"precision mediump float;\n"
"\n"
"attribute vec3 a_position;\n"
"attribute vec2 a_texcoord;\n"
"attribute vec4 a_color;\n"
"\n"
"uniform mat4 u_mvp;\n"
"\n"
"varying vec2 v_texcoord;\n"
"varying vec4 v_color;\n"
"\n"
"void main() {\n"
"    v_texcoord = a_texcoord;\n"
"    v_color = a_color;\n"
"    gl_Position = u_mvp * vec4(a_position, 1.0);\n"
"}\n";

const char* SHADER_UNLIT_FRAGMENT = 
"#version 100\n"
"precision mediump float;\n"
"\n"
"varying vec2 v_texcoord;\n"
"varying vec4 v_color;\n"
"\n"
"uniform sampler2D u_texture;\n"
"\n"
"void main() {\n"
"    vec4 tex_color = texture2D(u_texture, v_texcoord);\n"
"    gl_FragColor = tex_color * v_color;\n"
"}\n";

// Helper functions
uint32_t shader_compile_source(uint32_t type, const char* source) {
    if (!source) {
        fprintf(stderr, "Shader source is null\n");
        return 0;
    }
    
    uint32_t shader = glCreateShader(type);
    if (shader == 0) {
        fprintf(stderr, "Failed to create shader\n");
        return 0;
    }
    
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        shader_print_log(shader, false);
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

bool shader_link_program(uint32_t program) {
    glLinkProgram(program);
    
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        shader_print_log(program, true);
        return false;
    }
    
    return true;
}

void shader_print_log(uint32_t shader_or_program, bool is_program) {
    int log_length = 0;
    
    if (is_program) {
        glGetProgramiv(shader_or_program, GL_INFO_LOG_LENGTH, &log_length);
    } else {
        glGetShaderiv(shader_or_program, GL_INFO_LOG_LENGTH, &log_length);
    }
    
    if (log_length > 1) {
        char* log = malloc(log_length);
        if (log) {
            if (is_program) {
                glGetProgramInfoLog(shader_or_program, log_length, NULL, log);
                fprintf(stderr, "Program link error:\n%s\n", log);
            } else {
                glGetShaderInfoLog(shader_or_program, log_length, NULL, log);
                fprintf(stderr, "Shader compile error:\n%s\n", log);
            }
            free(log);
        }
    }
}

bool shader_create(Shader* shader, const char* vertex_src, const char* fragment_src, const char* name) {
    if (!shader || !vertex_src || !fragment_src) {
        return false;
    }
    
    memset(shader, 0, sizeof(Shader));
    
    if (name) {
        strncpy(shader->name, name, sizeof(shader->name) - 1);
    }
    
    // Compile vertex shader
    shader->vertex_shader = shader_compile_source(GL_VERTEX_SHADER, vertex_src);
    if (shader->vertex_shader == 0) {
        fprintf(stderr, "Failed to compile vertex shader for '%s'\n", shader->name);
        return false;
    }
    
    // Compile fragment shader
    shader->fragment_shader = shader_compile_source(GL_FRAGMENT_SHADER, fragment_src);
    if (shader->fragment_shader == 0) {
        fprintf(stderr, "Failed to compile fragment shader for '%s'\n", shader->name);
        glDeleteShader(shader->vertex_shader);
        return false;
    }
    
    // Create program
    shader->program = glCreateProgram();
    if (shader->program == 0) {
        fprintf(stderr, "Failed to create shader program for '%s'\n", shader->name);
        glDeleteShader(shader->vertex_shader);
        glDeleteShader(shader->fragment_shader);
        return false;
    }
    
    // Attach shaders
    glAttachShader(shader->program, shader->vertex_shader);
    glAttachShader(shader->program, shader->fragment_shader);
    
    // Link program
    if (!shader_link_program(shader->program)) {
        fprintf(stderr, "Failed to link shader program for '%s'\n", shader->name);
        shader_destroy(shader);
        return false;
    }
    
    // Cache uniform and attribute locations
    shader_cache_locations(shader);
    
    shader->is_valid = true;
    printf("Shader '%s' compiled successfully\n", shader->name);
    
    return true;
}

void shader_destroy(Shader* shader) {
    if (!shader || !shader->is_valid) {
        return;
    }
    
    if (shader->program) {
        glDeleteProgram(shader->program);
    }
    
    if (shader->vertex_shader) {
        glDeleteShader(shader->vertex_shader);
    }
    
    if (shader->fragment_shader) {
        glDeleteShader(shader->fragment_shader);
    }
    
    printf("Shader '%s' destroyed\n", shader->name);
    memset(shader, 0, sizeof(Shader));
}

void shader_use(const Shader* shader) {
    if (!shader || !shader->is_valid) {
        return;
    }
    
    glUseProgram(shader->program);
}

void shader_validate(const Shader* shader) {
    if (!shader || !shader->is_valid) {
        return;
    }
    
    glValidateProgram(shader->program);
    
    int success;
    glGetProgramiv(shader->program, GL_VALIDATE_STATUS, &success);
    if (!success) {
        fprintf(stderr, "Shader validation failed for '%s'\n", shader->name);
        shader_print_log(shader->program, true);
    }
}

int shader_get_uniform_location(const Shader* shader, const char* name) {
    if (!shader || !shader->is_valid || !name) {
        return -1;
    }
    
    return glGetUniformLocation(shader->program, name);
}

int shader_get_attribute_location(const Shader* shader, const char* name) {
    if (!shader || !shader->is_valid || !name) {
        return -1;
    }
    
    return glGetAttribLocation(shader->program, name);
}

void shader_cache_locations(Shader* shader) {
    if (!shader || !shader->is_valid) {
        return;
    }
    
    // Cache common uniform locations
    shader->u_model = shader_get_uniform_location(shader, "u_model");
    shader->u_view = shader_get_uniform_location(shader, "u_view");
    shader->u_projection = shader_get_uniform_location(shader, "u_projection");
    shader->u_mvp = shader_get_uniform_location(shader, "u_mvp");
    shader->u_time = shader_get_uniform_location(shader, "u_time");
    shader->u_resolution = shader_get_uniform_location(shader, "u_resolution");
    
    shader->u_light_pos = shader_get_uniform_location(shader, "u_light_pos");
    shader->u_light_color = shader_get_uniform_location(shader, "u_light_color");
    shader->u_ambient = shader_get_uniform_location(shader, "u_ambient");
    shader->u_camera_pos = shader_get_uniform_location(shader, "u_camera_pos");
    
    shader->u_texture = shader_get_uniform_location(shader, "u_texture");
    shader->u_color = shader_get_uniform_location(shader, "u_color");
    shader->u_opacity = shader_get_uniform_location(shader, "u_opacity");
    
    // Cache attribute locations
    shader->a_position = shader_get_attribute_location(shader, "a_position");
    shader->a_normal = shader_get_attribute_location(shader, "a_normal");
    shader->a_texcoord = shader_get_attribute_location(shader, "a_texcoord");
    shader->a_color = shader_get_attribute_location(shader, "a_color");
}

// Uniform setters
void shader_set_mat4(const Shader* shader, const char* name, const Mat4* matrix) {
    if (!shader || !shader->is_valid || !name || !matrix) {
        return;
    }
    
    int location = shader_get_uniform_location(shader, name);
    shader_set_mat4_cached(shader, location, matrix);
}

void shader_set_vec3(const Shader* shader, const char* name, Vec3 v) {
    if (!shader || !shader->is_valid || !name) {
        return;
    }
    
    int location = shader_get_uniform_location(shader, name);
    shader_set_vec3_cached(shader, location, v);
}

void shader_set_vec4(const Shader* shader, const char* name, Vec4 v) {
    if (!shader || !shader->is_valid || !name) {
        return;
    }
    
    int location = glGetUniformLocation(shader->program, name);
    if (location >= 0) {
        glUniform4f(location, v.x, v.y, v.z, v.w);
    }
}

void shader_set_float(const Shader* shader, const char* name, float value) {
    if (!shader || !shader->is_valid || !name) {
        return;
    }
    
    int location = shader_get_uniform_location(shader, name);
    shader_set_float_cached(shader, location, value);
}

void shader_set_int(const Shader* shader, const char* name, int value) {
    if (!shader || !shader->is_valid || !name) {
        return;
    }
    
    int location = shader_get_uniform_location(shader, name);
    shader_set_int_cached(shader, location, value);
}

// Optimized cached uniform setters
void shader_set_mat4_cached(const Shader* shader, int location, const Mat4* matrix) {
    if (!shader || !shader->is_valid || !matrix || location < 0) {
        return;
    }
    
    glUniformMatrix4fv(location, 1, GL_FALSE, matrix->m);
}

void shader_set_vec3_cached(const Shader* shader, int location, Vec3 v) {
    if (!shader || !shader->is_valid || location < 0) {
        return;
    }
    
    glUniform3f(location, v.x, v.y, v.z);
}

void shader_set_float_cached(const Shader* shader, int location, float value) {
    if (!shader || !shader->is_valid || location < 0) {
        return;
    }
    
    glUniform1f(location, value);
}

void shader_set_int_cached(const Shader* shader, int location, int value) {
    if (!shader || !shader->is_valid || location < 0) {
        return;
    }
    
    glUniform1i(location, value);
}

// Shader Manager
bool shader_manager_init(ShaderManager* manager) {
    if (!manager) {
        return false;
    }
    
    memset(manager, 0, sizeof(ShaderManager));
    
    if (!shader_manager_load_builtin_shaders(manager)) {
        return false;
    }
    
    manager->initialized = true;
    printf("Shader manager initialized with %d built-in shaders\n", SHADER_COUNT);
    
    return true;
}

void shader_manager_destroy(ShaderManager* manager) {
    if (!manager || !manager->initialized) {
        return;
    }
    
    for (int i = 0; i < SHADER_COUNT; i++) {
        shader_destroy(&manager->shaders[i]);
    }
    
    memset(manager, 0, sizeof(ShaderManager));
    printf("Shader manager destroyed\n");
}

Shader* shader_manager_get(ShaderManager* manager, ShaderType type) {
    if (!manager || !manager->initialized || type >= SHADER_COUNT) {
        return NULL;
    }
    
    return &manager->shaders[type];
}

bool shader_manager_load_builtin_shaders(ShaderManager* manager) {
    if (!manager) {
        return false;
    }
    
    // Load basic shader
    if (!shader_create(&manager->shaders[SHADER_BASIC], 
                      SHADER_BASIC_VERTEX, SHADER_BASIC_FRAGMENT, "basic")) {
        return false;
    }
    
    // Load billboard shader
    if (!shader_create(&manager->shaders[SHADER_BILLBOARD], 
                      SHADER_BILLBOARD_VERTEX, SHADER_BILLBOARD_FRAGMENT, "billboard")) {
        return false;
    }
    
    // Load unlit shader
    if (!shader_create(&manager->shaders[SHADER_UNLIT], 
                      SHADER_UNLIT_VERTEX, SHADER_UNLIT_FRAGMENT, "unlit")) {
        return false;
    }
    
    // Create a simple debug shader (solid color)
    const char* debug_vertex = 
        "#version 100\n"
        "attribute vec3 a_position;\n"
        "uniform mat4 u_mvp;\n"
        "void main() {\n"
        "    gl_Position = u_mvp * vec4(a_position, 1.0);\n"
        "}\n";
    
    const char* debug_fragment = 
        "#version 100\n"
        "precision mediump float;\n"
        "uniform vec4 u_color;\n"
        "void main() {\n"
        "    gl_FragColor = u_color;\n"
        "}\n";
    
    if (!shader_create(&manager->shaders[SHADER_DEBUG], 
                      debug_vertex, debug_fragment, "debug")) {
        return false;
    }
    
    return true;
}