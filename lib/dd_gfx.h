#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "vendor/glew.h"
#include "vendor/wglew.h"
#include "dreid.h"

typedef struct {
    float x, y, z;
} dd_vec3;

typedef struct {
    float x, y;   
} dd_vec2;

typedef struct {
    float x, y, z, w;
} dd_vec4;

uint32_t dd_make_vbo(float* vertices, uint32_t size, bool is_dynamic);
void dd_populate_vbo(uint32_t vbo, float* vertices, uint32_t size, bool is_dynamic);

typedef enum {
    DD_SHADER_FRAGMENT,
    DD_SHADER_VERTEX,
    // compute, geometry etc.
} dd_shadertype;

typedef struct {
    GLenum type;
    uint32_t count;
    bool normalized;
    uint32_t size; // gets calculated automatically
} dd_shader_vertex_attrib;

uint32_t dd_make_shader(dd_shadertype type, char* source);
uint32_t dd_make_vsfs_shader_program(char* vs_source, char* fs_source);
uint32_t dd_link_shader_program2(uint32_t shader_1, uint32_t shader_2);
uint32_t dd_set_vertex_attribs(dd_shader_vertex_attrib* attribs, uint16_t count); 
uint32_t dd_make_ebo(uint32_t* indices, uint32_t size, bool is_dynamic);
#define dd_set_uniform(type_suffix, shader_program, name, ...) {int32_t loc = glGetUniformLocation(shader_program, name); glUniform##type_suffix(loc, __VA_ARGS__);}
uint32_t dd_make_texture_from_file(char* file_name);
uint32_t dd_make_texture_from_memory(char* file_name, GLenum channels, int width, int height, uint8_t* data);