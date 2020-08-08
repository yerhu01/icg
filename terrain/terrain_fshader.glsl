R"(
#version 330 core
uniform sampler2D noiseTex;

uniform sampler2D grass;
uniform sampler2D rock;
uniform sampler2D sand;
uniform sampler2D snow;
uniform sampler2D water;

// The camera position
uniform vec3 viewPos;

in vec2 uv;
// Fragment position in world space coordinates
in vec3 fragPos;

out vec4 color;

void main() {

    // Directional light source
    vec3 lightDir = normalize(vec3(1,1,1));
    vec4 lightColor = vec4(1,1,1,1);

    // Texture size in pixels
    ivec2 size = textureSize(noiseTex, 0);

    /// TODO: Calculate surface normal N
    /// HINT: Use textureOffset(,,) to read height at uv + pixelwise offset
    /// HINT: Account for texture x,y dimensions in world space coordinates (default f_width=f_height=5)
    /*vec3 A = vec3( 0 );
    vec3 B = vec3( 0 );
    vec3 C = vec3( 0 );
    vec3 D = vec3( 0 );*/
    vec3 A = vec3( uv.x + 1.0/size.x, uv.y, textureOffset(noiseTex, uv, ivec2(1,0)) );
    vec3 B = vec3( uv.x - 1.0/size.x, uv.y, textureOffset(noiseTex, uv, ivec2(-1,0)) );
    vec3 C = vec3( uv.x, uv.y + 1.0/size.y, textureOffset(noiseTex, uv, ivec2(0,1)) );
    vec3 D = vec3( uv.x, uv.y - 1.0/size.y, textureOffset(noiseTex, uv, ivec2(0,-1)) );
    vec3 N = normalize( cross(normalize(A-B), normalize(C-D)) );

    /// TODO: Texture according to height and slope
    /// HINT: Read noiseTex for height at uv
    float low_height = -0.4f;
    float med_height = -0.2f;
    float high_height = 0.0;

    float med_slope = 0.7f;

    float h =  texture(noiseTex, uv).r;
    float slope = 1.0f - N.z;
    if(h<=low_height){
        color = texture(water,uv).rgba;//water
        N= vec3(0,0,1);
    }else if (h<med_height && h>low_height && slope <med_slope){
        color = texture(sand,uv).rgba;//sand
    }else if (h<med_height && h>=low_height && slope >=med_slope){
        color = texture(grass,uv).rgba;//grass
    }else if (h<high_height && h>=med_height && slope <med_slope){
        color = texture(grass,uv).rgba;//grass
    }else if (h<high_height && h>=med_height && slope >=med_slope){
        color = texture(rock,uv).rgba;//rock
    }else if (h>=high_height && slope >=med_slope){
        color = texture(rock,uv).rgba;//rock
    }else if (h>=high_height && slope <med_slope){
        color = texture(snow,uv).rgba;//snow
    }
    /// TODO: Calculate ambient, diffuse, and specular lighting
    /// HINT: max(,) dot(,) reflect(,) normalize()
    //c = (N+vec3(1))/2;

    //ambient light
    float ambientStrength = 0.1f;
    vec4 ambient = ambientStrength * lightColor;

    //diffuse light
    float diff = max(dot(N, lightDir), 0.0f);
    vec4 diffuse = diff * lightColor;

    //specular light
    float specularStrength = 0.4f;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, N);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32.0f);
    vec4 specular = specularStrength * spec * lightColor;

    color = (ambient+diffuse+specular) * color;

}
)"
