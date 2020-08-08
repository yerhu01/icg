#include <OpenGP/GL/Application.h>

using namespace OpenGP;

const int width=720, height=720;
#define POINTSIZE 10.0f

const char* line_vshader =
#include "line_vshader.glsl"
;
const char* line_fshader =
#include "line_fshader.glsl"
;

void init();
float getPt( float n1 , float n2 , float perc );
Vec2 deCasteljau(std::vector<Vec2>& controlPoints, float t);

std::unique_ptr<Shader> lineShader;
std::unique_ptr<GPUMesh> line;
std::vector<Vec2> controlPoints; //bezier control points

std::vector<Vec2> curvePoints;  //curve points based on control points
std::unique_ptr<GPUMesh> curve;

int main(int, char**){

    Application app;
    init();

    // Mouse position and selected point
    Vec2 position = Vec2(0,0);
    Vec2 *selection = nullptr; //pointer to control point when we select (point we click and hold, null when not clicked)

    //control points are array of all the points?
    //selection is 0,1 , 2 or 3?

    // Display callback
    Window& window = app.create_window([&](Window&){
        glViewport(0,0,width,height);
        glClear(GL_COLOR_BUFFER_BIT);
        glPointSize(POINTSIZE);  //made points bigger

        lineShader->bind();

        // Draw line red
        lineShader->set_uniform("selection", -1);
        line->set_attributes(*lineShader);
        line->set_mode(GL_LINE_STRIP); //GL_Lines strip  -> feed in points and will connect the points with lines
        //line->set_mode(GL_TRIANGLE_STRIP);
        line->draw();

        //draw curve
        curve->set_attributes(*lineShader);
        curve->set_mode(GL_LINE_STRIP);
        curve->draw();

        // Draw points red and selected point blue
        if(selection!=nullptr) lineShader->set_uniform("selection", int(selection-&controlPoints[0]));
        line->set_mode(GL_POINTS); //draw points at new vertices
        line->draw();

        lineShader->unbind();
    });
    window.set_title("Mouse");
    window.set_size(width, height);

    // Mouse movement callback
    window.add_listener<MouseMoveEvent>([&](const MouseMoveEvent &m){
        // Mouse position in clip coordinates
        Vec2 p = 2.0f*(Vec2(m.position.x()/width,-m.position.y()/height) - Vec2(0.5f,-0.5f));  // -1 to 1
        if( selection && (p-position).norm() > 0.0f) {
            /// TODO: Make selected control points move with cursor
            selection->x() = position.x();
            selection->y() = position.y();
            line->set_vbo<Vec2>("vposition", controlPoints);

            //recalculate curve points
            curvePoints.clear();
            for( float t = 0 ; t < 1 ; t += 0.01f ){
                Vec2 curvePoint = deCasteljau(controlPoints, t);
                curvePoints.push_back(curvePoint);
            }
            curve->set_vbo<Vec2>("vposition", curvePoints);
        }
        position = p;
    });

    // Mouse click callback : moves the control points when clicked/dragged
    window.add_listener<MouseButtonEvent>([&](const MouseButtonEvent &e){
        // Mouse selection case
        if( e.button == GLFW_MOUSE_BUTTON_LEFT && !e.released) {
            selection = nullptr;
            for(auto&& v : controlPoints) {
                if ( (v-position).norm() < POINTSIZE/std::min(width,height) ) {  //if cursor is some radius near a point
                    selection = &v; //selection points to the control point
                    break;
                }
            }
        }
        // Mouse release case
        if( e.button == GLFW_MOUSE_BUTTON_LEFT && e.released) {
            if(selection) {
                selection->x() = position.x(); //set the control point x()
                selection->y() = position.y(); //set the control point y()
                selection = nullptr;
                line->set_vbo<Vec2>("vposition", controlPoints);

                //recalculate curve points
                curvePoints.clear();
                for( float t = 0 ; t < 1 ; t += 0.01f ){
                    Vec2 curvePoint = deCasteljau(controlPoints, t);
                    curvePoints.push_back(curvePoint);
                }
                curve->set_vbo<Vec2>("vposition", curvePoints);
            }
        }
    });

    return app.run();
}

//gets point at t on the line n1 to n2
float getPt( float n1 , float n2 , float t ){
    float diff = n2 - n1;

    return n1 + ( diff * t );
}

//gets bezier curve point based on the given controlpoints
Vec2 deCasteljau(std::vector<Vec2>& controlPoints, float t){
    if(controlPoints.size() == 1){
        //1 point, this point is on curve
        return controlPoints[0];
    }else{
        std::vector<Vec2> newControlPoints = std::vector<Vec2>();
        for(int i = 0; i < controlPoints.size()-1; i++){
            float x = getPt( controlPoints[i].x() , controlPoints[i+1].x() , t );
            float y = getPt( controlPoints[i].y() , controlPoints[i+1].y() , t );
            newControlPoints.push_back(Vec2(x, y));
        }
        return deCasteljau(newControlPoints, t);
    }
}

void init(){
    glClearColor(1,1,1, /*solid*/1.0 );

    lineShader = std::unique_ptr<Shader>(new Shader()); //compiling shader
    lineShader->verbose = true;
    lineShader->add_vshader_from_source(line_vshader);
    lineShader->add_fshader_from_source(line_fshader);
    lineShader->link();

    controlPoints = std::vector<Vec2>();  //setting initial value for control points
    controlPoints.push_back(Vec2(-0.7f,-0.2f)); //put in initial 4 points
    controlPoints.push_back(Vec2(-0.3f, 0.2f));
    controlPoints.push_back(Vec2( 0.3f, 0.5f));
    controlPoints.push_back(Vec2( 0.7f, 0.0f));

    controlPoints.push_back(Vec2( 0.0f, 0.0f));

    line = std::unique_ptr<GPUMesh>(new GPUMesh());
    line->set_vbo<Vec2>("vposition", controlPoints);
    std::vector<unsigned int> indices = {0,1,2,3,4};
    line->set_triangles(indices);

    curve = std::unique_ptr<GPUMesh>(new GPUMesh());
    curvePoints = std::vector<Vec2>();
    std::vector<unsigned int> curveIndices = std::vector<unsigned int>();
    int indexCount = 0;
    for( float t = 0 ; t < 1 ; t += 0.01f ){
        Vec2 curvePoint = deCasteljau(controlPoints, t);
        curvePoints.push_back(curvePoint);
        curveIndices.push_back(indexCount);
        indexCount++;
    }
    curve->set_vbo<Vec2>("vposition", curvePoints);
    curve->set_triangles(curveIndices);

}
