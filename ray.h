#ifndef RAY_H
#define RAY_H

#include "vector3.h"

class Ray {
    private:
        Vector3 point;
        Vector3 dir;
    
    public:
        Ray(Vector3 origin, Vector3 goal);
        Ray(){};
        void setRay(Vector3 newOrigin, Vector3 newGoal);
        Vector3 getOrigin();
        Vector3 getDir();
};

#endif