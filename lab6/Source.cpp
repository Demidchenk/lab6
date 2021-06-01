#include "Header.h"

using namespace std;

#define M_PI 3.141592653589793 
#define INFINITY 1e8 
#define MAX_RAY_DEPTH 5 

typedef Vec3<float> Vec3f;

bool Triangle::intersect(const Vec3f& orig, const Vec3f& dir, const Vec3f& v0, const Vec3f& v1, const Vec3f& v2, float& t)
{
    Vec3f v0v1 = v1 - v0;
    Vec3f v0v2 = v2 - v0;
    Vec3f N = v0v1.cross(v0v2); 
    float area2 = N.length();

    float NdotRayDirection = N.dot(dir);
    if (!fabs(NdotRayDirection))
        return false;

    float d = N.dot(v0);
    t = (N.dot(orig) + d) / NdotRayDirection;
    if (t < 0) return false;

    Vec3f P = orig + (dir * t);
    Vec3f C; 

    Vec3f edge0 = v1 - v0;
    Vec3f vp0 = P - v0;
    C = edge0.cross(vp0);
    if (N.dot(C) < 0) return false; 

    Vec3f edge1 = v2 - v1;
    Vec3f vp1 = P - v1;
    C = edge1.cross(vp1);
    if (N.dot(C) < 0)  return false; 

    Vec3f edge2 = v0 - v2;
    Vec3f vp2 = P - v2;
    C = edge2.cross(vp2);
    if (N.dot(C) < 0) return false; 
    return true; 
}

Vec3f trace(
    const Vec3f& rayorig,
    const Vec3f& raydir,
    const vector<Sphere>& spheres,
    const int& depth)
{
    //if (raydir.length() != 1) std::cerr << "Error " << raydir << std::endl;
    float tnear = INFINITY;
    const Sphere* sphere = NULL;
    // find intersection of this ray with the sphere in the scene
    for (unsigned i = 0; i < spheres.size(); ++i) {
        float t0 = INFINITY, t1 = INFINITY;
        if (spheres[i].intersect(rayorig, raydir, t0, t1)) {
            if (t0 < 0) t0 = t1;
            if (t0 < tnear) {
                tnear = t0;
                sphere = &spheres[i];
            }
        }
    }
    // if there's no intersection return black or background color
    if (!sphere) return Vec3f(2);
    Vec3f surfaceColor = 0; // color of the ray/surfaceof the object intersected by the ray 
    Vec3f phit = rayorig + raydir * tnear; // point of intersection 
    Vec3f nhit = phit - sphere->center; // normal at the intersection point 
    nhit.normalize(); // normalize normal direction 
    // If the normal and the view direction are not opposite to each other
    // reverse the normal direction. That also means we are inside the sphere so set
    // the inside bool to true. Finally reverse the sign of IdotN which we want
    // positive.
    float bias = 1e-4; // add some bias to the point from which we will be tracing 
    for (unsigned i = 0; i < spheres.size(); ++i) {
        if (spheres[i].emissionColor.x > 0) {
            // this is a light
            Vec3f transmission = 1;
            Vec3f lightDirection = spheres[i].center - phit;
            lightDirection.normalize();
            for (unsigned j = 0; j < spheres.size(); ++j) {
                if (i != j) {
                    float t0, t1;
                    if (spheres[j].intersect(phit + nhit * bias, lightDirection, t0, t1)) {
                        transmission = 0;
                        break;
                    }
                }
            }
            surfaceColor += sphere->surfaceColor * transmission * max(float(0), nhit.dot(lightDirection)) * spheres[i].emissionColor;
        }
    }

    return surfaceColor + sphere->emissionColor;
}

void render(const vector<Sphere>& spheres)
{
    unsigned width = 640, height = 480;
    Vec3f* image = new Vec3f[width * height], * pixel = image;
    float invWidth = 1 / float(width), invHeight = 1 / float(height);
    float fov = 30, aspectratio = width / float(height);
    float angle = tan(M_PI * 0.5 * fov / 180.);
    // Trace rays
    for (unsigned y = 0; y < height; ++y) {
        for (unsigned x = 0; x < width; ++x, ++pixel) {
            float xx = (2 * ((x + 0.5) * invWidth) - 1) * angle * aspectratio;
            float yy = (1 - 2 * ((y + 0.5) * invHeight)) * angle;
            Vec3f raydir(xx, yy, -1);
            raydir.normalize();
            *pixel = trace(Vec3f(0), raydir, spheres, 0);
        }
    }
    // Save result to a PPM image (keep these flags if you compile under Windows)
    std::ofstream ofs("./untitled.ppm", std::ios::out | std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (unsigned i = 0; i < width * height; ++i) {
        ofs << (unsigned char)(std::min(float(1), image[i].x) * 255) <<
            (unsigned char)(std::min(float(1), image[i].y) * 255) <<
            (unsigned char)(std::min(float(1), image[i].z) * 255);
    }
    ofs.close();
    delete[] image;
}