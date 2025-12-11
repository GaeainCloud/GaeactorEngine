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

const float TILE_SIZE = 256.0;
const float M_PI = 3.1415926536;
const float EARTH_RADIUS = 6370972.0; // meters

const float originShift = 2 * M_PI * EARTH_RADIUS / 2.0;

vec3 project_mercator_(vec3 lnglat) {
  float x = lnglat.x * originShift / 180.0;
  float y = -log(tan((90.0 + lnglat.y)*M_PI/360.0)) / (M_PI / 180.0);
  y = y * originShift / 180.0;
  return vec3(x, y,0) ;
}

void main()
{
   Color =  veccolor;
   vs_out.color =  veccolor;
//   Color =  VertexColor;
//   vs_out.color =  VertexColor;
   gl_Position = proj_matrix*view*mv_matrix*vec4(VertexPosition,1.0);
//   gl_Position = proj_matrix*view*mv_matrix*vec4(project_mercator_(VertexPosition),1.0);
}