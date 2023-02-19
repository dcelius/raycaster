#ifndef COLOR_H
#define COLOR_H

#include <cmath>
#include <iostream>
#include "vector3.h"

class Color {
    private:
        float r;
        float g;
        float b;
        float a;
    public:
        Color(float r, float g, float b, float a = 0);
        Color(){};
        float getColorR(bool eightBit = false);
        float getColorG(bool eightBit = false);
        float getColorB(bool eightBit = false);
        float getColorA(bool eightBit = false);
        Vector3 getAsVector();
        void setColor(float newr, float newg, float newb, float newa);
        void print();
};

#endif