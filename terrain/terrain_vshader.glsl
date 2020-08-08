R"(
#version 330 core
uniform sampler2D noiseTex;

in vec3 vposition;
in vec2 vtexcoord;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec2 uv;
out vec3 fragPos;

void main() {
    /// TODO: Get height h at uv
    float h =  texture(noiseTex, vtexcoord).r*0.4;
    if (h <-0.4f){
            h = -0.4f;
    }

    uv = vtexcoord;
    fragPos = vposition.xyz + vec3(0,0,h);  //Apply displacement to vposition
    gl_Position = P*V*M*vec4(vposition.x, vposition.y, vposition.z + h, 1.0); //multiply model, then view, then projection
}
)"
