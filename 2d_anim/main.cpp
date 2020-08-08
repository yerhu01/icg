#include "OpenGP/GL/Eigen.h"
#include "OpenGP/GL/glfw_helpers.h"
#include "Mesh/Mesh.h"
#include <math.h>

using namespace OpenGP;

typedef Eigen::Transform<float,3,Eigen::Affine> Transform;

Mesh body;
Mesh wing;
std::vector<Vec3> controlPoints;      //bat bezier curve path
std::vector<OpenGP::Vec3> curvePoints; //bezier curve points of wings

void init();

//get point at time t on line n1 to n2
float getPt( float n1 , float n2 , float t ){
    float diff = n2 - n1;

    return n1 + ( diff * t );
}

//get curve points based on given controlpoints,  used for bat wing curve and path
Vec3 deCasteljau(std::vector<Vec3>& controlPoints, float t){
    if(controlPoints.size() == 1){
        //1 point, this point is on curve
        return controlPoints[0];
    }else{
        std::vector<Vec3> newControlPoints = std::vector<Vec3>();
        for(int i = 0; i < controlPoints.size()-1; i++){
            float x = getPt( controlPoints[i].x() , controlPoints[i+1].x() , t );
            float y = getPt( controlPoints[i].y() , controlPoints[i+1].y() , t );
            float z = 0.0f;
            newControlPoints.push_back(Vec3(x, y, z));
        }
        return deCasteljau(newControlPoints, t);
    }
}

//animation
void display(){
    glClear(GL_COLOR_BUFFER_BIT);


    //Get the time and frequency
    float time_s = glfwGetTime();
    float freq = M_PI*time_s;

    //Get the fraction part of the time
    float intpart;
    float fractpart = modf (time_s , &intpart);

    // **** Body transform
    Transform body_M = Transform::Identity();
    //use the fraction part of the time [0,1] to draw the bezier curve path
    Vec3 bezierPath = deCasteljau(controlPoints,fractpart);
    //make the body move along the bezier path by subtracting the bezier curve from the initial position
    body_M *= Eigen::Translation3f(0.2-bezierPath(0)*0.5, 0.2-bezierPath(1)*0.5, 0.0);
    //make the body get larger over time
    body_M *= Eigen::AlignedScaling3f(0.05 +fractpart*0.25, 0.05 +fractpart*0.25, 1.0);

    // **** Right wing transform
    Transform right_wing_M = Transform::Identity();
    //make the right wing move along the bezier path, slightly to the right of the body
    right_wing_M *= Eigen::Translation3f(0.3-bezierPath(0)*0.5, 0.2-bezierPath(1)*0.5, 0.0);
    //make the wing oscillate up and down by adding the cos function
    float scale_f = 0.01*std::cos(freq*5);
    right_wing_M *= Eigen::AngleAxisf(0.1+scale_f*50, -Eigen::Vector3f::UnitZ());
    // make the right wing get larger over time
    right_wing_M *= Eigen::AlignedScaling3f(0.01 +fractpart, 0.01 +fractpart, 1.0);

    // **** Left wing transform, same as right wing except it is flipped and flaps with sin function
    Transform left_wing_M = Transform::Identity();
    left_wing_M *= Eigen::Translation3f(0.1-bezierPath(0)*0.5, 0.2-bezierPath(1)*0.5, 0.0);
    float scale_f2 = 0.01*std::sin(freq*5);
    left_wing_M *= Eigen::AngleAxisf(0.1+scale_f2*50, -Eigen::Vector3f::UnitZ());
    left_wing_M *= Eigen::AlignedScaling3f(-(0.01 +fractpart), 0.01 +fractpart, 1.0);

    // draw the body, the right wing and the left wing
    body.draw(body_M.matrix(),0);
    wing.draw(right_wing_M.matrix(),1);
    wing.draw(left_wing_M.matrix(),1);
}

int main(int, char**){
    glfwInitWindowSize(512, 512);
    glfwMakeWindow("Bat");
    glfwDisplayFunc(display);
    init();
    glfwMainLoop();
    return EXIT_SUCCESS;
}

//inputs wing bezier curve points
void drawWingCurve(std::vector<OpenGP::Vec3> wingControlPoints){
    for( float i = 0.0f ; i <= 1.0f ; i += 0.01f ){
        Vec3 curvePoint = deCasteljau(wingControlPoints, i);
        curvePoints.push_back(curvePoint);
    }
}

//initialize body/wing/path
void init(){
    glClearColor(1.0f,1.0f,1.0f, 1.0 );

    // Enable alpha blending so texture backgroudns remain transparent
    glEnable (GL_BLEND); glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    body.init();
    wing.init();

    // BODY
    //set up quad vertices and indices for body
    std::vector<OpenGP::Vec3> quadVert;
    quadVert.push_back(OpenGP::Vec3(-1.0f, -1.0f, 0.0f));
    quadVert.push_back(OpenGP::Vec3(1.0f, -1.0f, 0.0f));
    quadVert.push_back(OpenGP::Vec3(1.0f, 1.0f, 0.0f));
    quadVert.push_back(OpenGP::Vec3(-1.0f, 1.0f, 0.0f));
    std::vector<unsigned> quadInd;
    quadInd.push_back(0);
    quadInd.push_back(1);
    quadInd.push_back(2);
    quadInd.push_back(0);
    quadInd.push_back(2);
    quadInd.push_back(3);
    body.loadVertices(quadVert, quadInd);
    //set up texture coordinates for body
    std::vector<OpenGP::Vec2> quadTCoord;
    quadTCoord.push_back(OpenGP::Vec2(0.0f, 0.0f));
    quadTCoord.push_back(OpenGP::Vec2(1.0f, 0.0f));
    quadTCoord.push_back(OpenGP::Vec2(1.0f, 1.0f));
    quadTCoord.push_back(OpenGP::Vec2(0.0f, 1.0f));
    body.loadTexCoords(quadTCoord);
    body.loadTextures("bat_body.png");


    // ANIMATION PATH
    //control points for the bezier curve path
    controlPoints = std::vector<Vec3>();
    controlPoints.push_back(Vec3(-0.7f,-1.7f, 0.0f));
    controlPoints.push_back(Vec3(-0.3f, 1.0f, 0.0f));
    controlPoints.push_back(Vec3( 0.3f, 0.5f, 0.0f));
    controlPoints.push_back(Vec3( 0.7f, -1.7f, 0.0f));

    // WING
    //set up the control points for the 5 curves of wing
    std::vector<OpenGP::Vec3> wingControlPoints1;
    std::vector<OpenGP::Vec3> wingControlPoints2;
    std::vector<OpenGP::Vec3> wingControlPoints3;
    std::vector<OpenGP::Vec3> wingControlPoints4;
    std::vector<OpenGP::Vec3> wingControlPoints5;
    wingControlPoints1.push_back(Vec3(0.0f,0.2f,0.0f));
    wingControlPoints1.push_back(Vec3(0.15f, 0.16f,0.0f));
    wingControlPoints1.push_back(Vec3( 0.18f, 0.2f,0.0f));
    wingControlPoints1.push_back(Vec3(0.2f, 0.3f,0.0f));
    wingControlPoints2.push_back(Vec3(0.2f,0.3f,0.0f));
    wingControlPoints2.push_back(Vec3(0.36f, 0.24f,0.0f));
    wingControlPoints2.push_back(Vec3( 0.64f, 0.31f,0.0f));
    wingControlPoints2.push_back(Vec3(0.8f, 0.1f,0.0f));
    wingControlPoints3.push_back(Vec3(0.8f,0.1f,0.0f));
    wingControlPoints3.push_back(Vec3(0.61f, 0.13f,0.0f));
    wingControlPoints3.push_back(Vec3( 0.49f, 0.01f,0.0f));
    wingControlPoints3.push_back(Vec3(0.4f, -0.1f,0.0f));
    wingControlPoints4.push_back(Vec3(0.4f,-0.1f,0.0f));
    wingControlPoints4.push_back(Vec3(0.26f, -0.02f,0.0f));
    wingControlPoints4.push_back(Vec3( 0.06f, 0.03f,0.0f));
    wingControlPoints4.push_back(Vec3(-0.1f, -0.15f,0.0f));
    wingControlPoints5.push_back(Vec3(-0.1f, -0.15f,0.0f));
    wingControlPoints5.push_back(Vec3(-0.13f, -0.04f,0.0f));
    wingControlPoints5.push_back(Vec3( -0.07f, 0.14f,0.0f));
    wingControlPoints5.push_back(Vec3(0.0f, 0.215f,0.0f));

    //draw the 5 bezier curves of the wing given the control points
    curvePoints = std::vector<OpenGP::Vec3>();
    curvePoints.push_back(Vec3(0.3f,0.1f,0.0f)); //starting point of triangle fan
    drawWingCurve(wingControlPoints1);
    drawWingCurve(wingControlPoints2);
    drawWingCurve(wingControlPoints3);
    drawWingCurve(wingControlPoints4);
    drawWingCurve(wingControlPoints5);

    //insert wing indices
    std::vector<unsigned> indicesCurve;
    for (unsigned int i = 0.0f ; i <= 501 ; i ++ ){
     indicesCurve.push_back(i);
    }
    wing.loadVertices(curvePoints, indicesCurve);
    //load wing textures
    wing.loadTexCoords(quadTCoord);
    wing.loadTextures("bat.png");
}
