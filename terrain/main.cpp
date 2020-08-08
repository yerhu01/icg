#include <OpenGP/GL/Application.h>
#include "OpenGP/GL/Eigen.h"

#include "loadTexture.h"
#include "noise.h"

using namespace OpenGP;
const int width=1280, height=720;

const char* skybox_vshader =
#include "skybox_vshader.glsl"
;
const char* skybox_fshader =
#include "skybox_fshader.glsl"
;

const char* terrain_vshader =
#include "terrain_vshader.glsl"
;
const char* terrain_fshader =
#include "terrain_fshader.glsl"
;

const unsigned resPrim = 999999; // The index at which we begin a new triangle strip
constexpr float PI = 3.14159265359f;

void init();
void genTerrainMesh();
void genCubeMesh();
void drawSkybox();
void drawTerrain();

std::unique_ptr<Shader> skyboxShader;
std::unique_ptr<GPUMesh> skyboxMesh;
GLuint skyboxTexture;

std::unique_ptr<Shader> terrainShader;
std::unique_ptr<GPUMesh> terrainMesh;
std::unique_ptr<R32FTexture> heightTexture;
std::map<std::string, std::unique_ptr<RGBA8Texture>> terrainTextures;

Vec3 cameraPos;
Vec3 cameraFront;
Vec3 cameraUp;
float halflife;
float yaw;
float pitch;

std::vector<Vec3> controlPoints; //bezier control points
std::vector<Vec3> curvePoints;  //curve points based on control points

//get point at time t on line n1 to n2
float getPt( float n1 , float n2 , float t ){
    float diff = n2 - n1;

    return n1 + ( diff * t );
}

//get curve points based on given controlpoints
Vec3 deCasteljau(std::vector<Vec3>& controlPoints, float t){
    if(controlPoints.size() == 1){
        //1 point, this point is on curve
        return controlPoints[0];
    }else{
        std::vector<Vec3> newControlPoints = std::vector<Vec3>();
        for(int i = 0; i < controlPoints.size()-1; i++){
            float x = getPt( controlPoints[i].x() , controlPoints[i+1].x() , t );
            float y = getPt( controlPoints[i].y() , controlPoints[i+1].y() , t );
            float z = controlPoints[i].z();  //doesn't change
            newControlPoints.push_back(Vec3(x, y, z));
        }
        return deCasteljau(newControlPoints, t);
    }
}

int main(int, char**){

    Application app;

    init();
    genCubeMesh();
    genTerrainMesh();

    //for bezier curve path
    int camPosInd = 0;
    int cameraSpeed = 2;

    cameraPos = curvePoints[camPosInd];//Vec3(-0.5f, 0.0f, 1.0f);
    cameraFront = Vec3(0,-1,0); //direction camera facing based on mouse position
    cameraUp = Vec3(0,0,1);
    yaw = 0.0f;
    pitch = 0.0f;

    //float cameraSpeed = 0.05f;


    // Display callback
    Window& window = app.create_window([&](Window&){
        glViewport(0,0,width,height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawSkybox();
        glClear(GL_DEPTH_BUFFER_BIT);
        drawTerrain();
    });
    window.set_title("Assignment 4");
    window.set_size(width, height);

    Vec2 mouse(0,0);
    window.add_listener<MouseMoveEvent>([&](const MouseMoveEvent &m){
        ///--- Camera control
        Vec2 delta = m.position - mouse;
        delta[1] = -delta[1];
        float sensitivity = 0.005f;
        delta = sensitivity * delta;

        yaw += delta[0];
        pitch += delta[1];

        if(pitch > PI/2.0f - 0.01f)  pitch =  PI/2.0f - 0.01f;
        if(pitch <  -PI/2.0f + 0.01f) pitch =  -PI/2.0f + 0.01f;

        Vec3 front(0,0,0);
        front[0] = sin(yaw)*cos(pitch);
        front[1] = cos(yaw)*cos(pitch);
        front[2] = sin(pitch);

        cameraFront = front.normalized();
        mouse = m.position;
    });

    window.add_listener<KeyEvent>([&](const KeyEvent &k){
        ///--- TODO: Implement WASD keys HINT: compare k.key to GLFW_KEY_W
        if(k.key == GLFW_KEY_W){
            //cameraPos += cameraSpeed * cameraFront; //forward
            for(int i = 0; i<curvePoints.size();i++){
                curvePoints[i]= curvePoints[i]+Vec3(0,0,0.05);
            }
            cameraPos = curvePoints[camPosInd];
        }
        if(k.key == GLFW_KEY_S){
            //cameraPos -= cameraSpeed * cameraFront; //back
            for(int i = 0; i<curvePoints.size();i++){
                curvePoints[i]= curvePoints[i]-Vec3(0,0,0.05);
            }
            cameraPos = curvePoints[camPosInd];
        }
        if(k.key == GLFW_KEY_A){
            //cameraPos -= cameraFront.cross(cameraUp).normalized()*cameraSpeed; // left
            if(camPosInd-cameraSpeed >= 0){
                camPosInd = camPosInd-cameraSpeed;
                cameraPos = curvePoints[camPosInd];
            }
        }
        if(k.key == GLFW_KEY_D){
            //cameraPos += cameraFront.cross(cameraUp).normalized()*cameraSpeed; //right
            if(camPosInd+cameraSpeed < curvePoints.size() ){
                camPosInd = camPosInd+cameraSpeed;
                cameraPos = curvePoints[camPosInd];
            }
        }
    });

    return app.run();
}

void init(){
    glClearColor(1,1,1, /*solid*/1.0 );

    controlPoints = std::vector<Vec3>();  //setting initial value for control points
    controlPoints.push_back(Vec3(0.5f, 2.5f, 1.5f)); //put in initial 5 points
    controlPoints.push_back(Vec3( -1.5f, 0.5f, 0.5f));
    controlPoints.push_back(Vec3( 0.5f, -1.5f, 0.5f));
    controlPoints.push_back(Vec3( 0.5f, 0.5f, 0.5f));
    for( float t = 0 ; t < 1 ; t += 0.01f ){
        Vec3 curvePoint = deCasteljau(controlPoints, t);
        curvePoints.push_back(curvePoint);
    }

    ///--- Compile shaders
    skyboxShader = std::unique_ptr<Shader>(new Shader());
    skyboxShader->verbose = true;
    skyboxShader->add_vshader_from_source(skybox_vshader);
    skyboxShader->add_fshader_from_source(skybox_fshader);
    skyboxShader->link();

    terrainShader = std::unique_ptr<Shader>(new Shader());
    terrainShader->verbose = true;
    terrainShader->add_vshader_from_source(terrain_vshader);
    terrainShader->add_fshader_from_source(terrain_fshader);
    terrainShader->link();

    ///--- Get height texture
    heightTexture = std::unique_ptr<R32FTexture>(fBm2DTexture());

    ///--- Load terrain and cubemap textures
    const std::string list[] = {"grass", "rock", "sand", "snow", "water"};
    for (int i=0 ; i < 5 ; ++i) {
        loadTexture(terrainTextures[list[i]], (list[i]+".png").c_str());
        terrainTextures[list[i]]->bind();
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    const std::string skyList[] = {"miramar_ft", "miramar_bk", "miramar_dn", "miramar_up", "miramar_rt", "miramar_lf"};
    glGenTextures(1, &skyboxTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
    int tex_wh = 1024;
    for(int i=0; i < 6; ++i) {
        std::vector<unsigned char> image;
        loadTexture(image, (skyList[i]+".png").c_str());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA, tex_wh, tex_wh, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void genTerrainMesh() {
    /// Create a flat (z=0) mesh for the terrain with given dimensions, using triangle strips
    terrainMesh = std::unique_ptr<GPUMesh>(new GPUMesh());
    int n_width = 256; // Grid resolution
    int n_height = 256;
    float f_width = 5.0f; // Grid width, centered at 0,0
    float f_height = 5.0f;

    std::vector<Vec3> points;
    std::vector<unsigned int> indices;
    std::vector<Vec2> texCoords;

    ///--- Vertex positions, tex coords
    for(int j=0; j<n_height; ++j) {
        for(int i=0; i<n_width; ++i) {
            /// TODO: calculate vertex positions, texture indices done for you
            points.push_back(Vec3(i/(float)n_width, j/(float)n_height, 0.0f)); //points.push_back(Vec3(0, 0, 0.0f));
            texCoords.push_back( Vec2( i/(float)(n_width-1), j/(float)(n_height-1)) );
        }
    }

    ///--- Element indices using triangle strips
    for(int j=0; j<n_height-1; ++j) {
        ///--- The two vertices at the base of each strip
        indices.push_back(j*n_width);
        indices.push_back((j+1)*n_width);
        for(int i=1; i<n_width; ++i) {
            /// TODO: push_back next two vertices HINT: Each one will generate a new triangle
            indices.push_back(j*n_width+i);
            indices.push_back((j+1)*n_width+i);
        }
        ///--- A new strip will begin when this index is reached
        indices.push_back(resPrim);
    }

    terrainMesh->set_vbo<Vec3>("vposition", points);
    terrainMesh->set_triangles(indices);
    terrainMesh->set_vtexcoord(texCoords);
}

void genCubeMesh() {
    ///--- Generate a cube mesh for skybox, already done
    skyboxMesh = std::unique_ptr<GPUMesh>(new GPUMesh());
    std::vector<Vec3> points;
    points.push_back(Vec3( 1, 1, 1)); // 0
    points.push_back(Vec3(-1, 1, 1)); // 1
    points.push_back(Vec3( 1, 1,-1)); // 2
    points.push_back(Vec3(-1, 1,-1)); // 3
    points.push_back(Vec3( 1,-1, 1)); // 4
    points.push_back(Vec3(-1,-1, 1)); // 5
    points.push_back(Vec3(-1,-1,-1)); // 6
    points.push_back(Vec3( 1,-1,-1)); // 7
    std::vector<unsigned int> indices = { 3, 2, 6, 7, 4, 2, 0, 3, 1, 6, 5, 4, 1, 0 };
    skyboxMesh->set_vbo<Vec3>("vposition", points);
    skyboxMesh->set_triangles(indices);
}

void drawSkybox() {
    skyboxShader->bind();

    // Set transformations
    Vec3 look = cameraFront + cameraPos;
    Mat4x4 V = lookAt(cameraPos, look, cameraUp); // pos, look, up
    skyboxShader->set_uniform("V", V);
    Mat4x4 P = perspective(80.0f, width/(float)height, 0.1f, 60.0f);
    skyboxShader->set_uniform("P", P);

    /// TODO: Bind Textures and set uniform
    /// HINT: Use GL_TEXTURE0, and texture type GL_TEXTURE_CUBE_MAP
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);


    /// TODO: Set atrributes, draw cube using GL_TRIANGLE_STRIP mode
    glEnable(GL_DEPTH_TEST);
    skyboxMesh->set_attributes(*skyboxShader);
    skyboxMesh->set_mode(GL_TRIANGLE_STRIP);
    skyboxMesh->draw();

    skyboxShader->unbind();
}

void drawTerrain() {
    terrainShader->bind();

    /// TODO: Create transformation matrices HINT: use lookAt and perspective
    Mat4x4 M = Mat4x4::Identity(); // Identity should be fine
    terrainShader->set_uniform("M", M);

    Vec3 look = cameraFront + cameraPos;
    //for camera move
     Mat4x4 V = lookAt(cameraPos, look, cameraUp); // pos, look, up
    //Mat4x4 V = lookAt(Vec3(0.5,0,0.5), Vec3(0.5,0.5,0), Vec3(0,0,1));//Mat4x4::Identity(); /// HERE
    terrainShader->set_uniform("V", V);

    Mat4x4 P = perspective(70.0f, width / (float)height, 0.01f, 10.0f); //Mat4x4::Identity(); /// AND HERE
    terrainShader->set_uniform("P", P);

    terrainShader->set_uniform("viewPos", cameraPos);

    // Bind textures
    int i = 0;
    for( std::map<std::string, std::unique_ptr<RGBA8Texture>>::iterator it = terrainTextures.begin(); it != terrainTextures.end(); ++it ) {
        glActiveTexture(GL_TEXTURE1+i);
        (it->second)->bind();
        terrainShader->set_uniform(it->first.c_str(), 1+i);
        ++i;
    }
    /// TODO: Bind height texture to GL_TEXTURE0 and set uniform noiseTex
    glActiveTexture(GL_TEXTURE0);
    heightTexture->bind();
    terrainShader->set_uniform("noiseTex", 0);

    // Draw terrain using triangle strips
    glEnable(GL_DEPTH_TEST);
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ); //comment/uncomment this for debugging
    terrainMesh->set_attributes(*terrainShader);
    terrainMesh->set_mode(GL_TRIANGLE_STRIP);
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(resPrim);
    /// TODO: Uncomment line below once this function is implemented
    terrainMesh->draw();

    heightTexture->unbind();
    terrainShader->unbind();
}
