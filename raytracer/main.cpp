#include "OpenGP/Image/Image.h"
#include "bmpwrite.h"  //writes output to bit map file

using namespace OpenGP;  //otherwise would need to do OpenGP.Colour for everything

using Colour = Vec3; // RGB Value
Colour red() { return Colour(1.0f, 0.0f, 0.0f); }
Colour white() { return Colour(1.0f, 1.0f, 1.0f); }
Colour black() { return Colour(0.0f, 0.0f, 0.0f); }

Colour blue() { return Colour(0.0f, 0.0f, 1.0f); }
Colour gray() { return Colour(0.35f, 0.35f, 0.35f); }
Colour lightgray() { return Colour(0.8f, 0.8f, 0.8f); }

struct Light{
    Vec3 lightPos; //above sphere (and in front?)
    float lightInt;
    float amblightInt;
}l;

struct Sphere{
    Vec3 spherePos;
    float sphereRadius;
    Colour sphereColour;

    Sphere(Vec3 pos, float radius, Colour colour):
        spherePos(pos), sphereRadius(radius), sphereColour(colour)
    {
    }
};

struct Plane{
    Vec3 planePos;
    Vec3 planeNorm;
    Colour planeColour;
}p;

Vec3 raySphere(
    const Vec3 &E,
    const Vec3 &ray,
    const Light &l,
    Sphere &s, //accepts object underlying the iterator
    const Vec3 EsubC,
    const float disc)
{
    Vec3 hitColour;

    float t = -ray.dot(EsubC) - std::sqrtf(disc); //time along vector in which ray intersects with sphere
    Vec3 pos  = E + t*ray; //position where ray intersects

    Vec3 normal = (pos - s.spherePos)/s.sphereRadius; //normal from sphere surface
    normal = normal.normalized(); //n

    Vec3 lightDir = l.lightPos - pos; //direction of light to intersect
    lightDir = lightDir.normalized(); //l

    Vec3 viewDir = E - pos; //vector pointing toward camera
    viewDir = viewDir.normalized(); //v

    Vec3 h = viewDir + lightDir; //half vector
    h = h.normalized();

    //Phong model
    //L = surface color * light Intensity * max(0,n dot l)   -> diffuse
    //  + specular color * light intensity * max(0, n dot h)^shinyness   -> specular
    //  + surface color * ambient light intensity         -> ambient
    return hitColour = s.sphereColour*l.lightInt*std::fmaxf(0.0f, normal.dot(lightDir)) +
                      lightgray()*l.lightInt*std::powf(std::fmaxf(0.0f, normal.dot(h)),10) +
                      s.sphereColour*l.amblightInt; //colour pixel
}

Vec3 rayPlane(
    const Vec3 &E,
    const Vec3 &ray,
    const Light &l,
    const Plane &p,
    std::list<Sphere> sl)
{
   Vec3 hitColour;

   Vec3 PsubL = p.planePos - E; //plane point - line (eye) point
   float t = PsubL.dot(p.planeNorm)/ray.dot(p.planeNorm);
   Vec3 pos = t*ray + E; //point of intersection

   Vec3 normal = p.planeNorm; //normal from sphere surface
   normal = normal.normalized(); //n

   Vec3 lightDir = l.lightPos - pos; //direction of light to intersect
   lightDir = lightDir.normalized(); //l

   Vec3 viewDir = E - pos; //vector pointing toward camera
   viewDir = viewDir.normalized(); //v

   Vec3 d = -viewDir;
   Vec3 r = d - 2*(d.dot(normal))*normal; //r for reflection

   Vec3 h = viewDir + lightDir; //half vector
   h = h.normalized();

   std::list<Sphere>::iterator it;
   for(it = sl.begin(); it != sl.end(); it++){
       //Access sphere object through iterator

       //sphere shadows, lightDir sphere intersection
       Vec3 EsubC = pos - it->spherePos; //plane hit pos subtracted by sphere center
       float discShadow = std::powf(lightDir.dot(EsubC),2) - EsubC.dot(EsubC) + it->sphereRadius*it->sphereRadius; //discriminent
       //sphere reflection, r sphere intersection
       float discReflection = std::powf(r.dot(EsubC),2) - EsubC.dot(EsubC) + it->sphereRadius*it->sphereRadius;

       if(discReflection >=0){
           //plane reflects sphere
           hitColour = 0.2*raySphere(pos, r, l, *it, EsubC, discReflection);

           //check if plane also has sphere shadow
           if (discShadow >= 0){
               //Sphere is in the way, plane has shadow
               return hitColour += 0.8*p.planeColour*l.amblightInt; //colour pixel
           }else{
               // no shadow
               return hitColour += 0.8*(p.planeColour*l.lightInt*std::fmaxf(0.0f, normal.dot(lightDir)) +
                                 lightgray()*l.lightInt*std::powf(std::fmaxf(0.0f, normal.dot(h)),1000) +
                                 p.planeColour*l.amblightInt); //colour pixel
           }
       }else if (discShadow >= 0){
               //plane has sphere shadow (no reflection)
               return hitColour = p.planeColour*l.amblightInt; //colour pixel
       }
   }
   // no sphere shadow and no reflection
   return hitColour = p.planeColour*l.lightInt*std::fmaxf(0.0f, normal.dot(lightDir)) +
                     lightgray()*l.lightInt*std::powf(std::fmaxf(0.0f, normal.dot(h)),1000) +
                     p.planeColour*l.amblightInt; //colour pixel
}

Vec3 castRay(
    const Vec3 &E,
    const Vec3 &ray,
    const Light &l,
    std::list<Sphere> sl,
    const Plane &p
        )
{
    Vec3 hitColour;

    ///ray sphere intersection and shading
    std::list<Sphere>::iterator it;
    for(it = sl.begin(); it != sl.end(); it++){
        //Access sphere object through iterator

        //less than 0, no solutions. 0, solution at one point, greater than 0, two points
        Vec3 EsubC = E - it->spherePos; //camera center subtracted by sphere center
        float disc = std::powf(ray.dot(EsubC),2) - EsubC.dot(EsubC) + it->sphereRadius*it->sphereRadius; //discriminent
        if (disc >= 0){ //if hits sphere
            return hitColour = raySphere(E, ray, l, *it, EsubC, disc);
        }
    }


    /// ray plane intersection and shading
    // ray.dot(planeNorm) = 0, then line and plane are parallel , else point of intersection
    float sol = ray.dot(p.planeNorm);
    if(sol < 0){  //if sol == 0, line and plane are parallel. if sol > 0 intersects behind camera, since ray looking up
        return hitColour = rayPlane(E, ray, l, p, sl);
    }

    return hitColour = black(); //colour pixel white if doesn't hit anything
}

int main(int, char**){

    int wResolution = 640;
    int hResolution = 480;
    float aspectRatio =  float(wResolution)/float(hResolution);
    // #rows = hResolution, #cols = wResolution
    Image<Colour> image(hResolution, wResolution);  //creates image object with that resolution

    ///define camera position and sphere position here
    Vec3 W = Vec3(0.0f, 0.0f, -1.0f); //view direction, pointing in -z direction
    Vec3 V = Vec3(0.0f, 1.0f, 0.0f);  //up vector
    Vec3 U = Vec3(1.0f, 0.0f, 0.0f);  //side vector
    float d = 1.0f; //length field of view from camera
    Vec3 E = -d*W;  //camera will be at +1 in Z

    //for grid
    float left = -1.0f*aspectRatio; //since width is longer than height
    float right = 1.0f*aspectRatio;
    float bottom = -1.0f;
    float top = 1.0f;

    //for light source
    l.lightPos = Vec3(-4.0f, 4.0f, -4.0f); //above sphere (and in front?)
    l.lightInt = 1.0f;
    l.amblightInt = 0.75f;

    //list for sphere objects
    std::list<Sphere> sl = { Sphere(Vec3(-2.0f, 0.0f, -4.0f), 1.0f, red()),
                             Sphere(Vec3(2.0f, 1.0f, -4.0f), 2.0f, blue())
                           };

    //for plane object
    p.planePos = Vec3(0.0f, -1.0f, 0.0f); //below sphere
    p.planeNorm = Vec3(0.0f, 1.0f, 0.0f); //flat
    p.planeColour = gray();

    //iterating over every pixel
    for (int row = 0; row < image.rows(); ++row) {
        for (int col = 0; col < image.cols(); ++col) {

            ///build primary rays
            Vec3 pixel = left*U + (col*(right-left)/image.cols())*U;  //col*width/#of columns
            pixel += bottom*V + (row*(top-bottom)/image.rows())*V;

            Vec3 hitColour = black();
            for(int i=0; i < 3; i++){  //each pixel is 2x2
                Vec3 ray = pixel - E;
                ray = ray.normalized(); //normalize the ray vector
                hitColour += castRay(E, ray, l, sl, p);

                if(i==0){
                    pixel += (right-left)/image.cols()*U*2;  //move right
                }else if(i==1){
                    pixel += (top-bottom)/image.rows()*V*2;  //move down
                }else if(i==2){
                    pixel -= (right-left)/image.cols()*U*2; //move left
                }
            }

            image(row,col) = hitColour/4;


       }
    }

    bmpwrite("../../out.bmp", image);
    imshow(image); //shows image

    return EXIT_SUCCESS;
}
