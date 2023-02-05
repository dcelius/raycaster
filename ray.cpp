// Written by Dylan Celius

#include "ray.h"

Ray::Ray(Vector3 origin, Vector3 goal) {
    point = origin;
    dir = goal.subtractVector(origin).getNormalizedVector();
}

void Ray::setRay(Vector3 origin, Vector3 goal) {
    point = origin;
    dir = goal.subtractVector(origin).getNormalizedVector();    
}

Vector3 Ray::getOrigin() { return point; }
Vector3 Ray::getDir() { return dir; }