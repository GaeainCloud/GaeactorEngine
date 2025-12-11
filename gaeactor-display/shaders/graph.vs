#version 430
layout(location=0) in vec3 VertexPosition;
layout(location=1) in vec4 VertexColor;
out vec4 Color;

out VS_OUT {
    vec4 color;
} vs_out;

layout (std140) uniform MVPMatrix
{
	mat4 proj_matrix;
	mat4 view;
	mat4 mv_matrix;
};
uniform vec4 veccolor;

// uniform 变量
uniform vec4 projRect;    // projRect: vec4(left, top, width, height)
uniform vec2 screenSize;  // screenSize: vec2(width, height)
uniform vec2 pixel;  // screenSize: vec2(xPixel, yPixel)

void main()
{
   Color =  veccolor;
   vs_out.color =  veccolor;
//   Color =  VertexColor;
//   vs_out.color =  VertexColor;
  gl_Position = proj_matrix*view*mv_matrix*vec4(VertexPosition,1.0);
}