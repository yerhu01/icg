/*
 * The purpose of this source file is to demonstrate the Mesh class
 * Its only functionality is to render vertices/normals/textures and load textures from png files
 * A demonstration of an ImGui menu window is also included in this file
*/
#include "Mesh/Mesh.h"
#include "OpenGP/GL/glfw_helpers.h"

#include <OpenGP/types.h>
#include <OpenGP/MLogger.h>
#include <OpenGP/GL/Application.h>
#include <OpenGP/GL/ImguiRenderer.h>

#include <string>
#include <iostream>
#include <fstream>
#include <math.h>
#define _USE_MATH_DEFINES

using namespace OpenGP;
using namespace std;

struct MeshObject{
    vector<Vec3> vertList; //vertex positions
    vector<unsigned int> indexList; //triangle vertex indices
    vector<Vec3> normList; //vn
    vector<Vec2> tCoordList; //vt
};

void writeObj(MeshObject o, string filename){
    ofstream outfile (filename);

    //vertices
    for(Vec3 v : o.vertList){
        outfile << "v";
        for(int i = 0; i < 3; i++){
             outfile << " " << v(i);
        }
        outfile << endl;
    }

    outfile << endl;

    //faces
    int i = 0;
    for(unsigned int f : o.indexList){
        if(i == 0){
            outfile << "f";
        }

        outfile << " " << f;
        i++;
        if(i == 3){
            outfile << endl;
            i = 0;
        }
    }

    //norms
    for(Vec3 v : o.normList){
        outfile << "vn";
        for(int i = 0; i < 3; i++){
             outfile << " " << v(i);
        }
        outfile << endl;
    }

    //tcoord
    for(Vec2 v : o.tCoordList){
        outfile << "vt";
        for(int i = 0; i < 2; i++){
            outfile << " " << v(i);
        }
        outfile << endl;
    }
    outfile.close();
}

void readObj(string filename, vector<Vec3>& v, vector<unsigned int>& f, vector<Vec3>& vn, vector<Vec2>& vt){
    string line;
    ifstream infile (filename);
    if (infile.is_open()){
        while(getline(infile,line)){
            string buff;
            stringstream ss(line); //insert string into stream
            vector<string> tokens;

            while(ss >> buff){  //seperate by spaces
                tokens.push_back(buff);
            }
            if(!tokens.empty()){
                if(tokens[0].compare("v") == 0){
                    //cout << stof(tokens[1]) << endl;

                    v.push_back(Vec3(stof(tokens[1],NULL),  //string to float
                                     stof(tokens[2],NULL),
                                     stof(tokens[3],NULL)));
                }else if(tokens[0].compare("f") == 0){
                    f.push_back(stoi(tokens[1],NULL, 10));  //string to int
                    f.push_back(stoi(tokens[2],NULL, 10));
                    f.push_back(stoi(tokens[3],NULL, 10));
                }else if(tokens[0].compare("vn") == 0){
                    vn.push_back(Vec3(stof(tokens[1],NULL),  //string to float
                                      stof(tokens[2],NULL),
                                      stof(tokens[3],NULL)));
                }else if(tokens[0].compare("vt") == 0){
                    vt.push_back(Vec2(stof(tokens[1],NULL),  //string to float
                                      stof(tokens[2],NULL)));
                }
            }
        }
        infile.close();
    }else{
        cout << "Unable to open file";
    }
}

void generateCubeMesh(string filename, Vec3 p, float length, bool addTexCoord){
    MeshObject cube;
    Vec3 ref = Vec3(p(0)-length/2, p(1)-length/2, p(2)+length/2); //front bottom left vertex

    for(int i = 0; i < 2; i++){ //front four then back four vertices
        cube.vertList.push_back(Vec3(ref));
        cube.vertList.push_back(Vec3(ref(0)+length, ref(1), ref(2)));
        cube.vertList.push_back(Vec3(ref(0)+length, ref(1)+length, ref(2)));
        cube.vertList.push_back(Vec3(ref(0), ref(1)+length, ref(2)));
        ref = Vec3(ref(0), ref(1), ref(2)-length);
    }

    cube.indexList.push_back(0); //front face. bottom triangle
    cube.indexList.push_back(1);
    cube.indexList.push_back(2);
    cube.indexList.push_back(2); //front face. top triangle
    cube.indexList.push_back(3);
    cube.indexList.push_back(0);
    cube.indexList.push_back(1); //right face
    cube.indexList.push_back(5);
    cube.indexList.push_back(6);
    cube.indexList.push_back(6);
    cube.indexList.push_back(2);
    cube.indexList.push_back(1);
    cube.indexList.push_back(5); //back face
    cube.indexList.push_back(4);
    cube.indexList.push_back(7);
    cube.indexList.push_back(7);
    cube.indexList.push_back(6);
    cube.indexList.push_back(5);
    cube.indexList.push_back(4); //left face
    cube.indexList.push_back(0);
    cube.indexList.push_back(3);
    cube.indexList.push_back(3);
    cube.indexList.push_back(7);
    cube.indexList.push_back(4);
    cube.indexList.push_back(3); //top face
    cube.indexList.push_back(2);
    cube.indexList.push_back(6);
    cube.indexList.push_back(6);
    cube.indexList.push_back(7);
    cube.indexList.push_back(3);
    cube.indexList.push_back(4); //bottom face
    cube.indexList.push_back(5);
    cube.indexList.push_back(1);
    cube.indexList.push_back(1);
    cube.indexList.push_back(0);
    cube.indexList.push_back(4);

    if(addTexCoord){
        cube.tCoordList.push_back(Vec2(0,0));
        cube.tCoordList.push_back(Vec2(1,0));
        cube.tCoordList.push_back(Vec2(1,1));
        cube.tCoordList.push_back(Vec2(0,1));
        cube.tCoordList.push_back(Vec2(1,0));
        cube.tCoordList.push_back(Vec2(0,0));
        cube.tCoordList.push_back(Vec2(0,1));
        cube.tCoordList.push_back(Vec2(1,1));
    }
    writeObj(cube,filename);
}

void generateSphereMesh(string filename, Vec3 c, float r, int n_lat, int n_long, bool addTexCoord){
    MeshObject sphere;

    //add pole vertices
    Vec3 top = Vec3(c(0),c(1)+r, c(2));
    Vec3 bottom = Vec3(c(0),c(1)-r, c(2));
    sphere.vertList.push_back(top); //index 0
    sphere.vertList.push_back(bottom); //index 1

    //add other vertices, ie intersections of longitudes and latitudes, top down
    for(int p = 1; p < n_lat; p++){ //exclude the poles
        for(int q = 0; q < n_long; q++){ //for each non pole latitude line, going to be n_long intersections
            Vec3 v = Vec3(c(0) + r*sin(M_PI*p/n_lat)*cos(2*M_PI*q/n_long),
                          c(1) + r*cos(M_PI*p/n_lat),
                          c(2) + r*sin(M_PI*p/n_lat)*sin(2*M_PI*q/n_long));

            sphere.vertList.push_back(v);
        }
    }

    //generate triangles connecting to the poles
    //top
    for(int q = 0; q < n_long; q++){
        //first row (latitude) of vertices start at pos 2
        sphere.indexList.push_back(2+q); //add 1st vertex
        if(q < n_long-1){
            //add 2nd vertex (to the right of 1st)
            sphere.indexList.push_back(q+2+1);
        }else{
           //last vertex of row, so must wrap around
            sphere.indexList.push_back(2);
        }
        sphere.indexList.push_back(0); //add top vertex
    }
    //bottom
    for(int q = 0; q < n_long; q++){
        //last row (latitude) of vertices start at last vertex of row (since going left)
        sphere.indexList.push_back(sphere.vertList.size()-1-q); //add 1st vertex
        if(q < n_long-1){
            //add 2nd vertex (to the left of 1st) - since upside down
            sphere.indexList.push_back(sphere.vertList.size()-1-q-1);
        }else{
           //first vertex of row, so must wrap around to last
            sphere.indexList.push_back(sphere.vertList.size()-1);
        }
        sphere.indexList.push_back(1); //bottom vertex
    }

    //generate other triangles
    for(int p = 1; p < n_lat-1; p++){ //exclude the poles and 2nd last latitude
        for(int q = 0; q < n_long; q++){
            //t1
            sphere.indexList.push_back(2+(p-1)*n_long+q); // pq
            if(q < n_long-1){
                sphere.indexList.push_back(2+p*n_long+q+1); //p+1 q+1
                sphere.indexList.push_back(2+(p-1)*n_long+q+1); // p q+1
            }else{
                //last one of row, wrap around
                sphere.indexList.push_back(2+p*n_long);   //p+1 q+1
                sphere.indexList.push_back(2+(p-1)*n_long); // p q+1
            }


            //t2
            sphere.indexList.push_back(2+(p-1)*n_long+q); // pq
            sphere.indexList.push_back(2+p*n_long+q); //p+1 q
            if(q < n_long-1){
                sphere.indexList.push_back(2+p*n_long+q+1); // p+1 q+1
            }else{
                //last one of row, wrap around
                sphere.indexList.push_back(2+p*n_long); // p+1 q+1
            }
        }
    }

    if(addTexCoord){
        for(Vec3 v: sphere.vertList){
            Vec3 d = Vec3(c(0)-v(0), c(1)-v(1),c(2)-v(2)).normalized(); //d is unit vector from v to center

            float u = 0.5 + atan2(d(2),d(0))/(2*M_PI);
            float v = 0.5 - asin(d(1))/M_PI;
            sphere.tCoordList.push_back(Vec2(u,v));
        }
    }
    writeObj(sphere,filename);
}

void generateCylinderMesh(string filename, Vec3 c, float r, float h, int n_div, bool addTexCoord){
    MeshObject cylinder;
    //determine center of cylinder caps
    //Vec3 tc = Vec3(c(0), c(1)+h/2, c(2)); //top cap
    //Vec3 bc = Vec3(c(0), c(1)-h/2, c(2));; //bottom cap
    //cylinder.vertList.push_back(tc); //index 0
    //cylinder.vertList.push_back(bc); //index 1

    //generate points along the circumference of cylindrical caps
    for(int p = 0; p < n_div; p++){ //points along circumference = n_div*2
        Vec3 vtop = Vec3(c(0)+r*sin(2*M_PI*p/n_div),
                         c(1)+h/2,
                         c(2)+r*cos(2*M_PI*p/n_div)); //point along circumference of top cap
        Vec3 vbottom = Vec3(c(0)+r*sin(2*M_PI*p/n_div),
                            c(1)-h/2,
                            c(2)+r*cos(2*M_PI*p/n_div)); //point along circumference of bottom cap

        cylinder.vertList.push_back(vtop);
        cylinder.vertList.push_back(vbottom);
    }

    //generate triangles //if want to include caps, add 2 to each of the push backs. ex first one is push_back(2+p*2)
    for(int p = 0; p < n_div; p++){
        //t1 upper side triangle
        cylinder.indexList.push_back(p*2); //vtop p
        cylinder.indexList.push_back(p*2+1); //vbottom p
        if(p < n_div-1){
            cylinder.indexList.push_back(p*2+2); //vtop p+1
        }else{
            //last one, must wrap around
            cylinder.indexList.push_back(0); //vtop p+1
        }

        //t2 lower side triangle
        cylinder.indexList.push_back(p*2+1); //vbottom p
        if(p < n_div-1){
            cylinder.indexList.push_back(p*2+1+2); //vbottom p+1
            cylinder.indexList.push_back(p*2+2); //vtop p+1
        }else{
            //last one, must wrap around
            cylinder.indexList.push_back(1); //vbottom p+1
            cylinder.indexList.push_back(0); //vtop p+1
        }
        /*
        //t3 top slice triangle
        cylinder.indexList.push_back(2+p*2); //vtop p
        if(p < n_div-1){
            cylinder.indexList.push_back(2+p*2+2); //vtop p+1
        }else{
            //last one, must wrap around
            cylinder.indexList.push_back(2); //vtop p+1
        }
        cylinder.indexList.push_back(0); //top center

        //t4 bottom slice triangle
        cylinder.indexList.push_back(2+p*2+1); //vbottom p
        if(p < n_div-1){
            cylinder.indexList.push_back(2+p*2+1+2); //vbottom p+1
        }else{
            //last one, must wrap around
            cylinder.indexList.push_back(2+1); //vbottom p+1
        }
        cylinder.indexList.push_back(1); //bottom center*/
    }

    if(addTexCoord){
        int i = 0;
        for(Vec3 v: cylinder.vertList){
            /*if(i == 0){
                // top cap
                cylinder.tCoordList.push_back(Vec2(0.25,0.75));
            }else if(i == 1){
                cylinder.tCoordList.push_back(Vec2(0.75,0.75));*/
            if(i%2 == 0){
                //top vertex
                float u = (float) i/(float) n_div;
                float v = 0.5;
                cylinder.tCoordList.push_back(Vec2(u,v));
            }else{
                //bottom vertex
                float u = (float) i/(float) n_div;
                float v = 0.0;
                cylinder.tCoordList.push_back(Vec2(u,v));
            }
            i++;
        }
    }
    writeObj(cylinder,filename);
}

void generateTorusMesh(string filename, Vec3 c, float bigR, float r, int n_cuts, int n_rings, bool addTexCoord){
    MeshObject torus;
    //bigR = torus center to tube center radius
    //r = tube radius

    //add other vertices, ie intersections of longitudes and latitudes, top down
    for(int p = 0; p < n_cuts; p++){
        Vec3 tubecenter = Vec3(c(0)+bigR*cos(2*M_PI*p/n_cuts),
                               c(1),
                               c(2)+bigR*sin(2*M_PI*p/n_cuts));
        for(int q = 0; q < n_rings; q++){ //for each cut, n_rings intersections
            Vec3 v = Vec3(tubecenter(0)+r*cos(2*M_PI*q/n_rings)*cos(2*M_PI*p/n_cuts),
                          tubecenter(1)+r*sin(2*M_PI*q/n_rings),
                          tubecenter(2)+r*cos(2*M_PI*q/n_rings)*sin(2*M_PI*p/n_cuts));

            torus.vertList.push_back(v);
        }
    }

    //generate triangles
    for(int p = 0; p < n_cuts; p++){
        for(int q = 0; q < n_rings; q++){
            //t1
            torus.indexList.push_back(p*n_rings+q); // pq
            if(q == n_rings-1 && p != n_cuts-1){
                //last ring wrap around
                torus.indexList.push_back(p*n_rings+0); //p q+1
                torus.indexList.push_back((p+1)*n_rings+0); // p+1 q+1
            }else if(p == n_cuts-1 && q != n_rings-1){
                //last cut wrap around
                torus.indexList.push_back(p*n_rings+(q+1)); //p q+1
                torus.indexList.push_back((0)*n_rings+(q+1)); // p+1 q+1
            }else if(q == n_rings-1 && p == n_cuts-1){
                //last ring and last cut, wrap around
                torus.indexList.push_back(p*n_rings+0); //p q+1
                torus.indexList.push_back(0); // p+1 q+1
            }else{
                torus.indexList.push_back(p*n_rings+(q+1)); //p q+1
                torus.indexList.push_back((p+1)*n_rings+(q+1)); // p+1 q+1
            }


            //t2
            torus.indexList.push_back(p*n_rings+q); // pq
            if(q == n_rings-1 && p != n_cuts-1){
                //last ring wrap around
                torus.indexList.push_back((p+1)*n_rings+0); // p+1 q+1
                torus.indexList.push_back((p+1)*n_rings+q); //p+1 q
            }else if(p == n_cuts-1 && q != n_rings-1){
                //last cut wrap around
                torus.indexList.push_back((0)*n_rings+(q+1)); // p+1 q+1
                torus.indexList.push_back(0*n_rings+q); //p+1 q
            }else if(q == n_rings-1 && p == n_cuts-1){
                //last ring and last cut, wrap around
                torus.indexList.push_back(0); // p+1 q+1
                torus.indexList.push_back(0*n_rings+q); //p+1 q
            }else{
                torus.indexList.push_back((p+1)*n_rings+(q+1)); // p+1 q+1
                torus.indexList.push_back((p+1)*n_rings+q); //p+1 q
            }
        }
    }

    if(addTexCoord){
        int ring = 0;
        int cut = 0;
        for(Vec3 v: torus.vertList){
            if(ring == n_rings){
                ring = 0;
                cut++;
            }
            float u = (float) cut / (float) n_cuts;
            float v = (float) ring/(float) n_rings;
            torus.tCoordList.push_back(Vec2(u,v));
            ring++;
        }
    }
    writeObj(torus,filename);
}

void loadRenderMesh(Mesh& renderMesh, string filename, string texturename){
    vector<Vec3> vertList; //v
    vector<unsigned int> indexList; //f
    vector<Vec3> normList; //vn
    vector<Vec2> tCoordList; //vt

    readObj(filename, vertList, indexList, normList, tCoordList);

    /// Example rendering a mesh
    /// Call to compile shaders
    renderMesh.init();

    /// Load Vertices and Indices (minimum required for Mesh::draw to work) aka v and f
    renderMesh.loadVertices(vertList, indexList);

    /// Load normals aka vn
    if(!normList.empty()){
        renderMesh.loadNormals(normList);
    }

    /// Load textures (assumes texcoords) aka vt
    if(texturename.compare("")!=0){
        renderMesh.loadTextures(texturename);
    }
    /// Load texture coordinates (assumes textures)
    if(!tCoordList.empty()){
        renderMesh.loadTexCoords(tCoordList);
    }
}

void drawRenderMesh(Mesh& renderMesh, Application& app){
    /// Create main window, set callback function
    auto &window1 = app.create_window([&](Window &window){
        int width, height;
        std::tie(width, height) = window.get_size();

        //OPENGL GLOBALS
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glClearColor(0.0f, 0.0f, 0.0f, 1); // background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /// Wireframe rendering, might be helpful when debugging your mesh generation
        //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

        float ratio = width / (float) height;
        Mat4x4 modelTransform = Mat4x4::Identity();
        Mat4x4 model = modelTransform.matrix();
        Mat4x4 projection = OpenGP::perspective(70.0f, ratio, 0.1f, 10.0f);

        //camera movement
        float time = .5f * (float)glfwGetTime();
        Vec3 cam_pos(2*cos(time), 2.0, 2*sin(time));
        Vec3 cam_look(0.0f, 0.0f, 0.0f);
        Vec3 cam_up(0.0f, 1.0f, 0.0f);
        Mat4x4 view = OpenGP::lookAt(cam_pos, cam_look, cam_up);

        renderMesh.draw(model, view, projection);
    });
    window1.set_title("Assignment 2");
}

int main() {

    Application app;
    Mesh renderMesh1;
    Mesh renderMesh2;
    Mesh renderMesh3;
    Mesh renderMesh4;
    string meshName1 = "cube.obj";
    string meshName2 = "sphere.obj";
    string meshName3 = "cylinder.obj";
    string meshName4 = "torus.obj";
    generateCubeMesh(meshName1, Vec3(-0.5f,-0.5f,0.5f), 1.0f, true);
    generateSphereMesh(meshName2, Vec3(-0.5f,-0.5f,0.5f), 1.0f, 20, 20, true);
    generateCylinderMesh(meshName3, Vec3(-0.5f,-0.5f,0.5f), 1.0f, 2.0f, 20, true);
    generateTorusMesh(meshName4, Vec3(-0.5f,-0.5f,0.5f), 1.0f, 0.25f, 20, 10, true);

    loadRenderMesh(renderMesh1, meshName1, "1.png");
    drawRenderMesh(renderMesh1, app);

    loadRenderMesh(renderMesh2, meshName2, "earth.png");
    drawRenderMesh(renderMesh2, app);

    loadRenderMesh(renderMesh3, meshName3, "soup.png");
    drawRenderMesh(renderMesh3, app);

    loadRenderMesh(renderMesh4, meshName4, "arrow.png");
    drawRenderMesh(renderMesh4, app);

    return app.run();
}
