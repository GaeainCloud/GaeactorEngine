#version 330 core
precision lowp float; 
uniform   mat4 _MVP;
in vec2 _position;
in vec2 _uv;
out vec2 _outUV;

void main()
{
    _outUV  =   _uv;
    vec4    pos =   vec4(_position,0,1);
    gl_Position =   _MVP * pos;
}