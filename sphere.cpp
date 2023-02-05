// Written by Dylan Celius

#include "sphere.h"

Sphere::Sphere(Vector3 newCenter, float newr, int newm) : center(newCenter), r(newr), m(newm) {}

void Sphere::setCenter(Vector3 newCenter) {
    center = newCenter;
}

void Sphere::setSphereRadius(float newr) { r = newr; }
void Sphere::setSphereMaterial(int newm) { m = newm; }

void Sphere::setSphere(Vector3 newCenter, float newr) {
    center = newCenter;
    r = newr;
}

Vector3 Sphere::getCenter() { return center; }
float Sphere::getRadius() { return r; }
int Sphere::getMaterial() { return m; }
void Sphere::print() {
    center.print();
    std::cout << " Color: " << m << std::endl;
}