#include "renderer/context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef RASPBERRY_PI

bool render_context_init(RenderContext* ctx, int width, int height, bool fullscreen) {
    if (!ctx) return false;
    
    memset(ctx, 0, sizeof(RenderContext));
    ctx->width = width;
    ctx->height = height;
    ctx->fullscreen = fullscreen;
    ctx->vsync = true;
    
    printf("Initializing OpenGL ES 2.0 context for Raspberry Pi...\n");
    
    // Initialize Broadcom VideoCore
    bcm_host_init();
    
    // Get display size
    uint32_t screen_width, screen_height;
    if (graphics_get_display_size(0, &screen_width, &screen_height) < 0) {
        fprintf(stderr, "Failed to get display size\n");
        return false;
    }
    
    printf("Screen resolution: %ux%u\n", screen_width, screen_height);
    
    // If fullscreen, use screen size
    if (fullscreen) {
        ctx->width = (int)screen_width;
        ctx->height = (int)screen_height;
    }
    
    // EGL configuration
    static const EGLint config_attribs[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_STENCIL_SIZE, 0,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };
    
    static const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    
    // Get EGL display
    ctx->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (ctx->display == EGL_NO_DISPLAY) {
        fprintf(stderr, "Failed to get EGL display\n");
        return false;
    }
    
    // Initialize EGL
    EGLint major, minor;
    if (!eglInitialize(ctx->display, &major, &minor)) {
        fprintf(stderr, "Failed to initialize EGL\n");
        return false;
    }
    
    printf("EGL version: %d.%d\n", major, minor);
    
    // Choose configuration
    EGLConfig config;
    EGLint num_configs;
    if (!eglChooseConfig(ctx->display, config_attribs, &config, 1, &num_configs)) {
        fprintf(stderr, "Failed to choose EGL config\n");
        return false;
    }
    
    if (num_configs == 0) {
        fprintf(stderr, "No suitable EGL configs found\n");
        return false;
    }
    
    // Bind OpenGL ES API
    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        fprintf(stderr, "Failed to bind OpenGL ES API\n");
        return false;
    }
    
    // Create native window using DispmanX
    ctx->dispman_display = vc_dispmanx_display_open(0);
    ctx->dispman_update = vc_dispmanx_update_start(0);
    
    VC_RECT_T dst_rect = {0, 0, (int32_t)screen_width, (int32_t)screen_height};
    VC_RECT_T src_rect = {0, 0, ctx->width << 16, ctx->height << 16};
    
    ctx->dispman_element = vc_dispmanx_element_add(
        ctx->dispman_update, ctx->dispman_display, 0, &dst_rect, 0,
        &src_rect, DISPMANX_PROTECTION_NONE, 0, 0, 0
    );
    
    ctx->native_window.element = ctx->dispman_element;
    ctx->native_window.width = ctx->width;
    ctx->native_window.height = ctx->height;
    
    vc_dispmanx_update_submit_sync(ctx->dispman_update);
    
    // Create EGL surface
    ctx->surface = eglCreateWindowSurface(ctx->display, config, &ctx->native_window, NULL);
    if (ctx->surface == EGL_NO_SURFACE) {
        fprintf(stderr, "Failed to create EGL surface\n");
        return false;
    }
    
    // Create EGL context
    ctx->context = eglCreateContext(ctx->display, config, EGL_NO_CONTEXT, context_attribs);
    if (ctx->context == EGL_NO_CONTEXT) {
        fprintf(stderr, "Failed to create EGL context\n");
        return false;
    }
    
    // Make context current
    if (!eglMakeCurrent(ctx->display, ctx->surface, ctx->surface, ctx->context)) {
        fprintf(stderr, "Failed to make EGL context current\n");
        return false;
    }
    
    // Enable vsync
    if (ctx->vsync) {
        eglSwapInterval(ctx->display, 1);
    }
    
    // Set viewport
    glViewport(0, 0, ctx->width, ctx->height);
    
    // Enable depth testing by default
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    // Enable backface culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    // Alpha blending for sprites
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    printf("OpenGL ES context created successfully\n");
    printf("Resolution: %dx%d\n", ctx->width, ctx->height);
    
    return true;
}

void render_context_swap_buffers(RenderContext* ctx) {
    if (!ctx) return;
    eglSwapBuffers(ctx->display, ctx->surface);
    ctx->frame_count++;
}

void render_context_destroy(RenderContext* ctx) {
    if (!ctx) return;
    
    printf("Destroying OpenGL ES context...\n");
    
    if (ctx->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(ctx->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        
        if (ctx->context != EGL_NO_CONTEXT) {
            eglDestroyContext(ctx->display, ctx->context);
        }
        
        if (ctx->surface != EGL_NO_SURFACE) {
            eglDestroySurface(ctx->display, ctx->surface);
        }
        
        eglTerminate(ctx->display);
    }
    
    // Cleanup DispmanX
    if (ctx->dispman_element) {
        ctx->dispman_update = vc_dispmanx_update_start(0);
        vc_dispmanx_element_remove(ctx->dispman_update, ctx->dispman_element);
        vc_dispmanx_update_submit_sync(ctx->dispman_update);
    }
    
    if (ctx->dispman_display) {
        vc_dispmanx_display_close(ctx->dispman_display);
    }
    
    bcm_host_deinit();
    
    memset(ctx, 0, sizeof(RenderContext));
}

#else
// Desktop implementation (placeholder for development/testing)

bool render_context_init(RenderContext* ctx, int width, int height, bool fullscreen) {
    if (!ctx) return false;
    
    memset(ctx, 0, sizeof(RenderContext));
    ctx->width = width;
    ctx->height = height;
    ctx->fullscreen = fullscreen;
    ctx->vsync = true;
    
    printf("Desktop OpenGL context not implemented yet\n");
    printf("This is a placeholder for development on desktop\n");
    printf("Target resolution: %dx%d\n", width, height);
    
    return true;
}

void render_context_swap_buffers(RenderContext* ctx) {
    if (!ctx) return;
    ctx->frame_count++;
}

void render_context_destroy(RenderContext* ctx) {
    if (!ctx) return;
    printf("Desktop context cleanup (placeholder)\n");
    memset(ctx, 0, sizeof(RenderContext));
}

#endif

// Common functions (work on both Pi and desktop)

void render_context_set_viewport(RenderContext* ctx, int x, int y, int width, int height) {
    if (!ctx) return;
    glViewport(x, y, width, height);
}

void render_context_clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void render_context_use_program(RenderContext* ctx, uint32_t program) {
    if (!ctx) return;
    
    if (ctx->bound_program != program) {
        glUseProgram(program);
        ctx->bound_program = program;
    }
}

void render_context_bind_texture(RenderContext* ctx, uint32_t texture) {
    if (!ctx) return;
    
    if (ctx->bound_texture != texture) {
        glBindTexture(GL_TEXTURE_2D, texture);
        ctx->bound_texture = texture;
    }
}

void render_context_update_fps(RenderContext* ctx) {
    if (!ctx) return;
    
    static clock_t last_time = 0;
    clock_t current_time = clock();
    
    if (last_time != 0) {
        double delta_time = (double)(current_time - last_time) / CLOCKS_PER_SEC;
        if (delta_time >= 1.0) {
            ctx->current_fps = (float)ctx->frame_count / (float)delta_time;
            ctx->frame_count = 0;
            ctx->last_fps_time = delta_time;
            last_time = current_time;
        }
    } else {
        last_time = current_time;
    }
}

bool render_context_query_capabilities(GLCapabilities* caps) {
    if (!caps) return false;
    
    memset(caps, 0, sizeof(GLCapabilities));
    
    // Get basic info
    caps->vendor = (const char*)glGetString(GL_VENDOR);
    caps->renderer = (const char*)glGetString(GL_RENDERER);
    caps->version = (const char*)glGetString(GL_VERSION);
    caps->extensions = (const char*)glGetString(GL_EXTENSIONS);
    
    // Parse version
    if (caps->version) {
        sscanf(caps->version, "%d.%d", &caps->major_version, &caps->minor_version);
    }
    
    // Get limits
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &caps->max_texture_size);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &caps->max_vertex_attribs);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &caps->max_texture_units);
    
    // Check extensions (simplified - would need proper parsing for real use)
    caps->has_vao = (caps->extensions && strstr(caps->extensions, "GL_OES_vertex_array_object"));
    caps->has_instancing = (caps->extensions && strstr(caps->extensions, "GL_EXT_instanced_arrays"));
    caps->has_texture_compression = (caps->extensions && strstr(caps->extensions, "GL_EXT_texture_compression"));
    
    return true;
}

void render_context_print_info(const GLCapabilities* caps) {
    if (!caps) return;
    
    printf("\n=== OpenGL Information ===\n");
    printf("Vendor: %s\n", caps->vendor ? caps->vendor : "Unknown");
    printf("Renderer: %s\n", caps->renderer ? caps->renderer : "Unknown");
    printf("Version: %s\n", caps->version ? caps->version : "Unknown");
    printf("GL Version: %d.%d\n", caps->major_version, caps->minor_version);
    
    printf("\nLimits:\n");
    printf("Max texture size: %d\n", caps->max_texture_size);
    printf("Max vertex attribs: %d\n", caps->max_vertex_attribs);
    printf("Max texture units: %d\n", caps->max_texture_units);
    
    printf("\nExtensions:\n");
    printf("VAO support: %s\n", caps->has_vao ? "Yes" : "No");
    printf("Instancing: %s\n", caps->has_instancing ? "Yes" : "No");
    printf("Texture compression: %s\n", caps->has_texture_compression ? "Yes" : "No");
    printf("========================\n\n");
}

void render_context_check_errors(const char* operation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        fprintf(stderr, "OpenGL error in %s: %s\n", 
                operation ? operation : "unknown", 
                render_context_get_error_string(error));
    }
}

const char* render_context_get_error_string(uint32_t error) {
    switch (error) {
        case GL_NO_ERROR: return "No error";
        case GL_INVALID_ENUM: return "Invalid enum";
        case GL_INVALID_VALUE: return "Invalid value";
        case GL_INVALID_OPERATION: return "Invalid operation";
        case GL_OUT_OF_MEMORY: return "Out of memory";
        default: return "Unknown error";
    }
}