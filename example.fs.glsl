#version 330 core

in vec2 uv;
out vec4 frag_color;

uniform vec4 color;
uniform sampler2D tex;

void main() 
{
    frag_color = texture(tex, uv);
}