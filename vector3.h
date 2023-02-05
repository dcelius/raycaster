#ifndef VECTOR3_H
#define VECTOR3_H

#include <cmath>
#include <iostream>

class Vector3 {
    private:
        float x;
        float y;
        float z;
        float length;
    public:
        Vector3(float newx, float newy, float newz);
        Vector3(){};
        void setVector(float newx = 0, float newy = 0, float newz = 0);
        float getVectorX();
        float getVectorY();
        float getVectorZ();
        Vector3 getNormalizedVector();
        float magnitude();
        float dotProduct(Vector3 other);
        Vector3 crossProduct(Vector3 other);
        Vector3 scaleVector(float scalar);
        Vector3 addVector(Vector3 other);
        Vector3 subtractVector(Vector3 other);
        Vector3 project(Vector3 other);
        void print();
};

#endif