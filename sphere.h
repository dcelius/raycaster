#ifndef SPHERE_H
#define SPHERE_H

#include "vector3.h"
#include <iostream>

class Sphere {
    private:
        Vector3 center;
        float r;
        int m;

    public:
        Sphere(Vector3 newCenter, float newr, int newm);
        Sphere(){};
        void setCenter(Vector3 newCenter);
        void setSphereRadius(float newr);
        void setSphereMaterial(int newm);
        void setSphere(Vector3 newCenter, float newr);
        Vector3 getCenter();
        Vector3 getNormal(Vector3 other);
        float getRadius();
        int getMaterial();
        void print();
};

#endif