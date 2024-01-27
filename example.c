#include <math.h>
#include <stdio.h>

#include "lib/dreid.h"
#include "lib/dd_gfx.h"
#include "lib/dd_log.h"

uint32_t vbo, vao, ebo, sp;

float vertices[] = {
     0.5f,  0.5f, 0.0f,     1.f, 1.f,    // top right
     0.5f, -0.5f, 0.0f,     1.f, 0.f,    // bottom right
    -0.5f, -0.5f, 0.0f,     0.f, 0.f,    // bottom left
    -0.5f,  0.5f, 0.0f,     0.f, 1.f,    // top left 
};

uint32_t indices[] = {
    0, 1, 3,
    1, 2, 3,
};

void setup(dd_ctx* ctx)
{
    wglSwapIntervalEXT(1);
    glClearColor(0.094f, 0.094f, 0.094f, 1.f);
    
    vbo = dd_make_vbo(vertices, sizeof(vertices), false);
    char* vs_source = dd_read_file("example.vs.glsl", null);
    char* fs_source = dd_read_file("example.fs.glsl", null);

    sp = dd_make_vsfs_shader_program(vs_source, fs_source);

    free(vs_source); free(fs_source);

    dd_shader_vertex_attrib attribs[] = {
        {.type=GL_FLOAT, .count = 3, .normalized = false},
        {.type=GL_FLOAT, .count = 2, .normalized = false}
    };
    vao = dd_set_vertex_attribs(attribs, sizeof(attribs) / sizeof(dd_shader_vertex_attrib));
    ebo = dd_make_ebo(indices, sizeof(indices), false);

    glUseProgram(sp);
    glBindVertexArray(vao);

    uint32_t texture = dd_make_texture_from_file("texture_01.png");
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, texture);
    dd_set_uniform(1i, sp, "tex", 0); 
}

int frame_counter = 0;
bool wireframe = false;

void frame(dd_ctx* ctx)
{
    frame_counter++;

    for_to(i, ctx->event_count) {
        dd_key_event ev = ctx->key_events[i];
        if (key_pressed(ev, 0x57)) {
            glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
            wireframe = !wireframe;
        }
    }

    double time = dd_time(ctx);
    float green_value = (sin(time / 1000.f) / 2.f) + 0.5f;
    dd_set_uniform(4f, sp, "color", 0.f, green_value, 0.f, 1.f);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    SwapBuffers(ctx->primary_window->dc);
    if (frame_counter == 60) {
        log_debug("frame, %lf, FPS: %.3lf", ctx->dt, 1000.f / ctx->dt);
        frame_counter = 0;
    }
}

void destroy(dd_ctx* ctx)
{
    log_info("destroy!");
}

int main(int argc, char** argv)
{
    return dd_app(&(dd_app_desc) {
        .width = 1024,
        .height = 1024,
        .title = "DreiD Example",
        .setup = setup,
        .frame = frame,
        .destroy = destroy,
        //.is_reactive = true, // updates only on window messages
    });
}