#include <stdlib.h>
#include "dd_gfx.h"
#include "dd_log.h"

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"
char infoLog[512];

uint32_t dd_make_vbo(float *vertices, uint32_t size, bool is_dynamic)
{
    uint32_t vbo;
    glGenBuffers(1, &vbo);
    dd_populate_vbo(vbo, vertices, size, is_dynamic);
    return vbo;
}

void dd_populate_vbo(uint32_t vbo, float *vertices, uint32_t size, bool is_dynamic)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, is_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
}

// ALWAYS call after dd_set_vertex_attribs
uint32_t dd_make_ebo(uint32_t* indices, uint32_t size, bool is_dynamic)
{
    uint32_t ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, is_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    return ebo;
}

uint32_t dd_make_shader(dd_shadertype type, char* source)
{
    GLenum shader_type;
    switch (type) {
        case DD_SHADER_FRAGMENT: { shader_type = GL_FRAGMENT_SHADER; break; }
        case DD_SHADER_VERTEX: { shader_type = GL_VERTEX_SHADER; break; }
    }
    uint32_t shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source, null);
    glCompileShader(shader);
    int32_t ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        glGetShaderInfoLog(shader, 512, null, infoLog);
        log_error("Failed to compile shader!\n%s", infoLog);
        exit(-1);
    }
    return shader;
}

uint32_t dd_link_shader_program2(uint32_t shader_1, uint32_t shader_2)
{
    uint32_t shader_program = glCreateProgram();
    glAttachShader(shader_program, shader_1); 
    glAttachShader(shader_program, shader_2); 
    glLinkProgram(shader_program);
    int32_t ok;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &ok);
    if (!ok) {
        glGetProgramInfoLog(shader_program, 512, null, infoLog);
        log_error("Failed to link shaders:\n%s", infoLog);
        exit(-1);
    }
    glDeleteShader(shader_1); 
    glDeleteShader(shader_2); 

    return shader_program;
}

uint32_t dd_make_vsfs_shader_program(char* vs_source, char* fs_source)
{
    uint32_t vs = dd_make_shader(DD_SHADER_VERTEX, vs_source);
    uint32_t fs = dd_make_shader(DD_SHADER_FRAGMENT, fs_source);
    uint32_t sp = dd_link_shader_program2(vs, fs);
    return sp;
}

static uint32_t dd_get_sizeof_type(GLenum type)
{
	switch(type)
	{
		case GL_BYTE:
		case GL_UNSIGNED_BYTE:
			return sizeof(GLbyte);
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
			return sizeof(GLshort);
		case GL_INT_2_10_10_10_REV:
		case GL_INT:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
		case GL_UNSIGNED_INT:
			return sizeof(GLint);
		case GL_FLOAT:
			return sizeof(GLfloat);
		case GL_DOUBLE:
			return sizeof(GLdouble);
		case GL_FIXED:
			return sizeof(GLfixed);
		case GL_HALF_FLOAT:
			return sizeof(GLhalf);
	}

	return 0;
}

uint32_t dd_set_vertex_attribs(dd_shader_vertex_attrib* attribs, uint16_t count) 
{
    if (count > 16) {
        // non fatal since shader compilation didn't fail if we reach this point
        log_warn("To many vertex attributes. Only 16 vertex attributes are guaranteed to be available.");
    }

    // TODO: this is very stupid
    uint32_t stride = 0;
    for_to(i, count) {
        dd_shader_vertex_attrib* attr = &attribs[i];
        if (attr->count > 4) {
            log_fatal("Too many values. One vertex attribute can only contain 4 values."); exit(-1);
        }
        attr->size = dd_get_sizeof_type(attr->type) * attr->count; 
        stride += attr->size;
    }

    uint32_t vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    uint64_t offset = 0;
    for_to(i, count) {
        dd_shader_vertex_attrib* attr = &attribs[i];
        glVertexAttribPointer(i, attr->count, attr->type, attr->normalized, stride, (void*)offset);
        offset += attr->size;
        glEnableVertexAttribArray(i);
    }
    return vao;
}

uint32_t dd_make_texture_from_file(char* file_name)
{
    int width, height, nch;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(file_name, &width, &height, &nch, 0);
    if (!data) {
        log_error("Failed to load texture \"%s\"", file_name); exit(-1);
    }
    log_debug("%d, %d, %d", width, height, nch);
    uint32_t texture; glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, nch == 4 ? GL_RGBA : GL_RGB, width, height, 0, nch == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    return texture;
}

uint32_t dd_make_texture_from_memory(GLenum channels, int width, int height, uint8_t* data)
{
    uint32_t texture; glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, channels, width, height, 0, channels, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    return texture;
}