// Written by Dylan Celius
// 2/19/23

#include "vector3.h"

Vector3::Vector3(float newx, float newy, float newz) : x(newx), y(newy), z(newz) {
    length = magnitude();
} 

void Vector3::setVector(float newx, float newy, float newz) {
    x = newx;
    y = newy;
    z = newz;
    length = magnitude();
}

float Vector3::getVectorX() { return x; }
float Vector3::getVectorY() { return y; }
float Vector3::getVectorZ() { return z; }

Vector3 Vector3::getNormalizedVector() {
    if (length > 0.001 || length < -0.001) {
        return Vector3(x / length, y / length, z / length);
    }
    else {
        std::cerr << "length is 0 or negative, cannot proceed" << std::endl; 
        throw "Divide by 0 Error";
    }
}

float Vector3::magnitude() {
    return sqrt((x * x) + (y * y) + (z * z));
}

float Vector3::dotProduct(Vector3 other) {
    return x * other.x + y * other.y + z * other.z;
}

Vector3 Vector3::crossProduct(Vector3 other) {
    float crossx = y * other.z - z * other.y;
    float crossy = -(x * other.z - z * other.x);
    float crossz = x * other.y - y * other.x;
    return Vector3(crossx, crossy, crossz);
}

Vector3 Vector3::scaleVector(float scalar) {
    return Vector3(x * scalar, y * scalar, z * scalar);
}

Vector3 Vector3::addVector(Vector3 other) {
    return Vector3(x + other.x, y + other.y, z + other.z);
}

Vector3 Vector3::subtractVector(Vector3 other) {
    return Vector3(x - other.x, y - other.y, z - other.z);
}

Vector3 Vector3::project(Vector3 other) {
    float scalar = dotProduct(other) / other.dotProduct(other);
    return other.scaleVector(scalar);
}

Vector3 Vector3::clampVector(float low, float high) {
    float tempx, tempy, tempz;
    (x < low) ? tempx = low : tempx = x;
    (y < low) ? tempy = low : tempy = y;
    (z < low) ? tempz = low : tempz = z;
    (x > high) ? tempx = high : tempx = x;
    (y > high) ? tempy = high : tempy = y;
    (z > high) ? tempz = high : tempz = z;
    return Vector3(tempx, tempy, tempz);
}

Vector3 Vector3::multiplyComponents(Vector3 other) {
    return Vector3(x * other.x, y * other.y, z * other.z);
}

void Vector3::print() {
    std::cout << "(" << x << ", " << y << ", " << z << ")" << std::endl;
}