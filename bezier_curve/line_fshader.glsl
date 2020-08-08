R"(
#version 330 core
uniform int selection;

out vec4 color;

void main() {
    if ( selection == gl_PrimitiveID ) { //selection is index of point clicked
        color = vec4(0,0,1,1); //draw blue
    } else {
        color = vec4(1,0,0,1); //draw red
    }
}
)"
