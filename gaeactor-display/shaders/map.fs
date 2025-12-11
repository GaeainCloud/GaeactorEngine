#version 330 core
precision  lowp float; 
uniform    sampler2D   _texture;
in    vec2        _outUV;
out vec4 FragColor;
void main()
{
    vec4   color   =   texture2D(_texture,_outUV);
    FragColor   =   color;
}