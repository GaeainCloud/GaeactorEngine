#version 330 core
layout (location = 0) in vec3 position;
layout(location = 1) in vec4 VertexColor; 
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

uniform float rotationAngle;
void main()
{
    Color =  VertexColor;
    vs_out.color =  veccolor;
//  mat2 rotationMatrix = mat2(cos(rotationAngle), -sin(rotationAngle), sin(rotationAngle), cos(rotationAngle));
//    mat2 rotationMatrix = mat2(cos(rotationAngle), sin(rotationAngle), -sin(rotationAngle), cos(rotationAngle));

//    vec2 rotatedPosition = rotationMatrix * position.xy;
//    vec3 rposition = vec3(rotatedPosition,0.0);
    gl_Position = proj_matrix*view*mv_matrix*vec4(position,1.0);
}