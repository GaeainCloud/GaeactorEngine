#version 330 core

layout (lines) in;
//layout (line_strip, max_vertices = 9) out;
layout (triangle_strip, max_vertices = 9) out;


in VS_OUT {
    vec4 color;
} gs_in[];

uniform float arrowSize;
out vec4 Color;
void main()
{
    Color = gs_in[0].color;

    vec4 vertex1 = gl_in[0].gl_Position;
    vec4 vertex2 = gl_in[1].gl_Position;

    vec2 lineDirection = normalize(vertex2.xy - vertex1.xy);
    vec2 lineNormal = vec2(-lineDirection.y, lineDirection.x);
    float lineLength = length(vertex2.xy - vertex1.xy);



    vec4 vertex3 = vertex1 + vec4(lineDirection*lineLength*7/8, 0.0, 0.0);


    vec4 vertexa =  vertex1 + vec4(lineNormal * arrowSize, 0.0, 0.0);
    vec4 vertexb =  vertex1 - vec4(lineNormal * arrowSize, 0.0, 0.0);
    vec4 vertexc =  vertex3 - vec4(lineNormal * arrowSize, 0.0, 0.0);
    vec4 vertexd =  vertex3 - vec4(lineNormal * arrowSize*6, 0.0, 0.0);
    vec4 vertexe =  vertex2;
    vec4 vertexf =  vertex3 + vec4(lineNormal * arrowSize*6, 0.0, 0.0);
    vec4 vertexg =  vertex3 + vec4(lineNormal * arrowSize, 0.0, 0.0);

//    gl_Position = vertexa;
//    EmitVertex();
//    gl_Position = vertexb;
//    EmitVertex();
//    gl_Position = vertexc;
//    EmitVertex();
//    gl_Position = vertexd;
//    EmitVertex();
//    gl_Position = vertexe;
//    EmitVertex();
//    gl_Position = vertexf;
//    EmitVertex();
//    gl_Position = vertexg;
//    EmitVertex();
//    gl_Position = vertexa;
//    EmitVertex();


    gl_Position = vertexb;
    EmitVertex();
    gl_Position = vertexa;
    EmitVertex();
    gl_Position = vertexc;
    EmitVertex();
    gl_Position = vertexg;
    EmitVertex();
    gl_Position = vertexe;
    EmitVertex();
    gl_Position = vertexf;
    EmitVertex();
    gl_Position = vertexd;
    EmitVertex();
    EndPrimitive();
}