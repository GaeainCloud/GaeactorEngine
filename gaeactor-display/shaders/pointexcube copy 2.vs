
out VS_OUT {
    mat4 projectionmatrix;
} vs_out;
void main() {
    vs_out.projectionmatrix=gl_ModelViewProjectionMatrix;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

