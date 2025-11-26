// Desktop OpenGL stub implementations
// These are placeholder functions for building on systems without OpenGL headers
// The actual Pi Zero 2 W will use the real OpenGL ES implementation

#ifndef RASPBERRY_PI

#include "renderer/context.h"
#include <stdio.h>

// OpenGL function stubs
void glViewport(GLint x, GLint y, GLint width, GLint height) {
    printf("Desktop stub: glViewport(%d, %d, %d, %d)\n", x, y, width, height);
}

void glEnable(GLenum cap) {
    printf("Desktop stub: glEnable(0x%x)\n", cap);
}

void glDisable(GLenum cap) {
    printf("Desktop stub: glDisable(0x%x)\n", cap);
}

void glDepthFunc(GLenum func) {
    printf("Desktop stub: glDepthFunc(0x%x)\n", func);
}

void glCullFace(GLenum mode) {
    printf("Desktop stub: glCullFace(0x%x)\n", mode);
}

void glFrontFace(GLenum mode) {
    printf("Desktop stub: glFrontFace(0x%x)\n", mode);
}

void glBlendFunc(GLenum sfactor, GLenum dfactor) {
    printf("Desktop stub: glBlendFunc(0x%x, 0x%x)\n", sfactor, dfactor);
}

void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) {
    printf("Desktop stub: glClearColor(%.2f, %.2f, %.2f, %.2f)\n", red, green, blue, alpha);
}

void glClear(GLuint mask) {
    printf("Desktop stub: glClear(0x%x)\n", mask);
}

void glUseProgram(GLuint program) {
    printf("Desktop stub: glUseProgram(%u)\n", program);
}

void glBindTexture(GLenum target, GLuint texture) {
    printf("Desktop stub: glBindTexture(0x%x, %u)\n", target, texture);
}

GLenum glGetError(void) {
    return 0; // GL_NO_ERROR
}

const GLubyte* glGetString(GLenum name) {
    switch (name) {
        case 0x1F00: return (const GLubyte*)"Desktop Stub Vendor";     // GL_VENDOR
        case 0x1F01: return (const GLubyte*)"Desktop Stub Renderer";   // GL_RENDERER  
        case 0x1F02: return (const GLubyte*)"Desktop Stub 2.0";        // GL_VERSION
        case 0x1F03: return (const GLubyte*)"";                        // GL_EXTENSIONS
        default: return (const GLubyte*)"Unknown";
    }
}

void glGetIntegerv(GLenum pname, GLint* params) {
    switch (pname) {
        case 0x0D33: *params = 1024; break;    // GL_MAX_TEXTURE_SIZE
        case 0x8869: *params = 16; break;      // GL_MAX_VERTEX_ATTRIBS
        case 0x8872: *params = 8; break;       // GL_MAX_TEXTURE_IMAGE_UNITS
        default: *params = 0; break;
    }
}

// Shader function stubs
GLuint glCreateShader(GLenum type) {
    static GLuint next_id = 1;
    printf("Desktop stub: glCreateShader(0x%x) -> %u\n", type, next_id);
    return next_id++;
}

GLuint glCreateProgram(void) {
    static GLuint next_id = 1000;
    printf("Desktop stub: glCreateProgram() -> %u\n", next_id);
    return next_id++;
}

void glShaderSource(GLuint shader, GLint count, const GLchar** string, const GLint* length) {
    (void)string; (void)length; // Suppress unused parameter warnings
    printf("Desktop stub: glShaderSource(%u, %d, ...)\n", shader, count);
}

void glCompileShader(GLuint shader) {
    printf("Desktop stub: glCompileShader(%u)\n", shader);
}

void glAttachShader(GLuint program, GLuint shader) {
    printf("Desktop stub: glAttachShader(%u, %u)\n", program, shader);
}

void glLinkProgram(GLuint program) {
    printf("Desktop stub: glLinkProgram(%u)\n", program);
}

void glDeleteShader(GLuint shader) {
    printf("Desktop stub: glDeleteShader(%u)\n", shader);
}

void glDeleteProgram(GLuint program) {
    printf("Desktop stub: glDeleteProgram(%u)\n", program);
}

void glGetShaderiv(GLuint shader, GLenum pname, GLint* params) {
    (void)shader; (void)pname; // Suppress unused parameter warnings
    *params = 1; // Always return success for stubs
}

void glGetProgramiv(GLuint program, GLenum pname, GLint* params) {
    (void)program; (void)pname; // Suppress unused parameter warnings
    *params = 1; // Always return success for stubs
}

void glGetShaderInfoLog(GLuint shader, GLint bufSize, GLint* length, GLchar* infoLog) {
    if (infoLog && bufSize > 0) {
        infoLog[0] = '\0';
    }
    if (length) {
        *length = 0;
    }
}

void glGetProgramInfoLog(GLuint program, GLint bufSize, GLint* length, GLchar* infoLog) {
    if (infoLog && bufSize > 0) {
        infoLog[0] = '\0';
    }
    if (length) {
        *length = 0;
    }
}

void glValidateProgram(GLuint program) {
    printf("Desktop stub: glValidateProgram(%u)\n", program);
}

GLint glGetUniformLocation(GLuint program, const GLchar* name) {
    static GLint next_id = 0;
    printf("Desktop stub: glGetUniformLocation(%u, '%s') -> %d\n", program, name, next_id);
    return next_id++;
}

GLint glGetAttribLocation(GLuint program, const GLchar* name) {
    static GLint next_id = 0;
    printf("Desktop stub: glGetAttribLocation(%u, '%s') -> %d\n", program, name, next_id);
    return next_id++;
}

void glUniformMatrix4fv(GLint location, GLint count, GLubyte transpose, const GLfloat* value) {
    (void)value; // Suppress unused parameter warning
    printf("Desktop stub: glUniformMatrix4fv(%d, %d, %d, ...)\n", location, count, transpose);
}

void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
    printf("Desktop stub: glUniform3f(%d, %.2f, %.2f, %.2f)\n", location, v0, v1, v2);
}

void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    printf("Desktop stub: glUniform4f(%d, %.2f, %.2f, %.2f, %.2f)\n", location, v0, v1, v2, v3);
}

void glUniform1f(GLint location, GLfloat v0) {
    printf("Desktop stub: glUniform1f(%d, %.2f)\n", location, v0);
}

void glUniform1i(GLint location, GLint v0) {
    printf("Desktop stub: glUniform1i(%d, %d)\n", location, v0);
}

#endif // !RASPBERRY_PI