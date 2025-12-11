#version 330 core

layout(points) in;
//layout(points, max_vertices=9) out;
layout(triangle_strip, max_vertices=36) out;

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

        for(n =0; n < gl_in.length();n++)
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
                // 正方体的6个面，每个面由2个三角形组成
            int[36] faces = int[](
                0, 1, 2, 2, 3, 0, // 底面
                4, 5, 6, 6, 7, 4, // 顶面
                0, 4, 5, 5, 1, 0, // 前面
                2, 6, 7, 7, 3, 2, // 后面
                0, 3, 7, 7, 4, 0, // 左面
                2, 6, 5, 5, 1, 2  // 右面
            );

            vec4[6] colors = vec4[](
                vec4(0.0, 1.0, 0.0, 1.0),
                vec4(0.0, 1.0, 0.0, 1.0),
                vec4(1.0, 0.0, 0.0, 1.0),
                vec4(1.0, 0.0, 0.0, 1.0),
                vec4(0.0, 0.0, 1.0, 1.0),
                vec4(0.0, 0.0, 1.0, 1.0)
            );

        /*
                fColor = colors[0];
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[0], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[1], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[2], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[3], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[0], 1.0);
                EmitVertex();
                
                fColor = colors[2];
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[4], 1.0);
                EmitVertex();
                EndPrimitive();
        */


            for (int i = 0; i < 2; i++) {        
                fColor = colors[i];
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+0]], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+1]], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+2]], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+3]], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+4]], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+5]], 1.0);
                EmitVertex();
                EndPrimitive();
            }


            for (int i = 0+2; i < 2+2; i++) {
                
                fColor = colors[i];
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+0]], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+1]], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+2]], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+3]], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+4]], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+5]], 1.0);
                EmitVertex();
                EndPrimitive();
            }

            
            for (int i = 0+2+2; i < 2+2+2; i++) {
                
                fColor = colors[i];
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+0]], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+1]], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+2]], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+3]], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+4]], 1.0);
                EmitVertex();
                gl_Position = vertex1 + projectionmatrix * vec4(vertices[faces[i*6+5]], 1.0);
                EmitVertex();
                EndPrimitive();
            }
        }
    }

}