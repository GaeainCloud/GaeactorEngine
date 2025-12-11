
#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 transform;
out VS_OUT {
    mat4 projectionmatrix;
} vs_out;

void main()
{
    vs_out.projectionmatrix=transform;
    gl_Position = transform * vec4(position, 1.0);
}