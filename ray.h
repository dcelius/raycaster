#ifndef RAY_H
#define RAY_H

#include "vector3.h"

class Ray {
    private:
        Vector3 origin;
        Vector3 dir;
    
    public:
        Ray(Vector3 origin, Vector3 dir);
        Ray(){};
        void setRay(Vector3 newOrigin, Vector3 newDir);
        Vector3 getOrigin();
        Vector3 getDir();
        Vector3 getPoint(float distance);
};

#endif