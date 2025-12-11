#version 430
layout(location=0) in vec3 VertexPosition;
layout(location=1) in vec4 VertexColor;
out vec4 Color;
layout (std140) uniform MVPMatrix
{
	mat4 proj_matrix;
	mat4 view;
	mat4 mv_matrix;
};
void main()
{
   Color = VertexColor;
   gl_Position = proj_matrix*view*mv_matrix*vec4(VertexPosition,1.0);
}