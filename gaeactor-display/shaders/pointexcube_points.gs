#version 330 core

layout(points) in;

layout (points, max_vertices = 9) out;
in VS_OUT {
    mat4 projectionmatrix;
    float length;
} gs_in[];


uniform vec4 veccolor;
uniform float size;
out vec4 fColor;
void main()
{    
            //    if((gl_PrimitiveIDIn & 1) == 1)
    {
        int n = 0;

        //for(n =0; n < gl_in.length();n++)
        {                   
            vec4 vertex1 = gl_in[n].gl_Position; 
            mat4 projectionmatrix = gs_in[n].projectionmatrix;
            vec3[8] vertices = vec3[](
                vec3(-size, -size, -size),
                vec3(size, -size, -size),
                vec3(size, size, -size),
                vec3(-size, size, -size),
                vec3(-size, -size, size),
                vec3(size, -size, size),
                vec3(size, size, size),
                vec3(-size, size, size)
            );

            fColor = vec4(0.0, 1.0, 0.0, 1.0);    
            gl_Position = vertex1 + projectionmatrix * vec4(vec3(0, 0, 0), 1.0);
            EmitVertex();
            fColor = veccolor;   
            for (int i = 0; i < 8; i++) {        
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[i], 1.0);
                EmitVertex();
            }    
            EndPrimitive();
        }
    }
}