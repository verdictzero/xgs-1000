#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef RASPBERRY_PI
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <bcm_host.h>
#else
// Desktop fallback - define what we need without including headers
typedef float GLfloat;
typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLint;
typedef unsigned char GLubyte;
typedef char GLchar;

// Define the constants we need
#define GL_COLOR_BUFFER_BIT      0x00004000
#define GL_DEPTH_BUFFER_BIT      0x00000100
#define GL_DEPTH_TEST            0x0B71
#define GL_LEQUAL               0x0203
#define GL_CULL_FACE            0x0B44
#define GL_BACK                 0x0405
#define GL_CCW                  0x0901
#define GL_BLEND                0x0BE2
#define GL_SRC_ALPHA            0x0302
#define GL_ONE_MINUS_SRC_ALPHA  0x0303
#define GL_TEXTURE_2D           0x0DE1

// Additional constants needed
#define GL_VENDOR               0x1F00
#define GL_RENDERER             0x1F01
#define GL_VERSION              0x1F02
#define GL_EXTENSIONS           0x1F03
#define GL_MAX_TEXTURE_SIZE     0x0D33
#define GL_MAX_VERTEX_ATTRIBS   0x8869
#define GL_MAX_TEXTURE_IMAGE_UNITS 0x8872
#define GL_NO_ERROR             0
#define GL_INVALID_ENUM         0x0500
#define GL_INVALID_VALUE        0x0501
#define GL_INVALID_OPERATION    0x0502
#define GL_OUT_OF_MEMORY        0x0505

// Placeholder function declarations
void glViewport(GLint x, GLint y, GLint width, GLint height);
void glEnable(GLenum cap);
void glDisable(GLenum cap);
void glDepthFunc(GLenum func);
void glCullFace(GLenum mode);
void glFrontFace(GLenum mode);
void glBlendFunc(GLenum sfactor, GLenum dfactor);
void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
void glClear(GLuint mask);
void glUseProgram(GLuint program);
void glBindTexture(GLenum target, GLuint texture);
GLenum glGetError(void);
const GLubyte* glGetString(GLenum name);
void glGetIntegerv(GLenum pname, GLint* params);

// Shader functions needed
GLuint glCreateShader(GLenum type);
GLuint glCreateProgram(void);
void glShaderSource(GLuint shader, GLint count, const GLchar** string, const GLint* length);
void glCompileShader(GLuint shader);
void glAttachShader(GLuint program, GLuint shader);
void glLinkProgram(GLuint program);
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

#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int width;
    int height;
    bool fullscreen;
    bool vsync;

#ifdef RASPBERRY_PI
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
    
    // Broadcom VideoCore
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    EGL_DISPMANX_WINDOW_T native_window;
#else
    void* window; // Platform-specific window handle (SDL/GLFW)
    void* gl_context;
#endif

    // OpenGL state tracking
    uint32_t bound_program;
    uint32_t bound_vao;
    uint32_t bound_texture;
    
    // Performance stats
    uint32_t frame_count;
    double last_fps_time;
    float current_fps;
    
} RenderContext;

typedef struct {
    int major_version;
    int minor_version;
    const char* vendor;
    const char* renderer;
    const char* version;
    const char* extensions;
    
    // Supported features
    bool has_vao;
    bool has_instancing;
    bool has_texture_compression;
    
    // Limits
    int max_texture_size;
    int max_vertex_attribs;
    int max_texture_units;
    
} GLCapabilities;

// Context management
bool render_context_init(RenderContext* ctx, int width, int height, bool fullscreen);
void render_context_swap_buffers(RenderContext* ctx);
void render_context_update_fps(RenderContext* ctx);
void render_context_destroy(RenderContext* ctx);

// Viewport and state
void render_context_set_viewport(RenderContext* ctx, int x, int y, int width, int height);
void render_context_clear(float r, float g, float b, float a);

// OpenGL state caching (to reduce state changes)
void render_context_use_program(RenderContext* ctx, uint32_t program);
void render_context_bind_texture(RenderContext* ctx, uint32_t texture);

// Capability detection
bool render_context_query_capabilities(GLCapabilities* caps);
void render_context_print_info(const GLCapabilities* caps);

// Error handling
void render_context_check_errors(const char* operation);
const char* render_context_get_error_string(uint32_t error);

#ifdef __cplusplus
}
#endif

#endif // CONTEXT_H