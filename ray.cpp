// Written by Dylan Celius

#include "ray.h"

Ray::Ray(Vector3 point, Vector3 direction) : origin(point), dir(direction) {}

void Ray::setRay(Vector3 newOrigin, Vector3 newDir) {
    origin = newOrigin;
    dir = newDir;    
}

Vector3 Ray::getOrigin() { return origin; }
Vector3 Ray::getDir() { return dir; }

Vector3 Ray::getPoint(float distance) {
    return Vector3(origin.addVector(dir.getNormalizedVector().scaleVector(distance)));
}