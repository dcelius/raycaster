// Written by Dylan Celius
// 2/19/23

#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include "string.h"
#include "color.h"
#include "vector3.h"
#include "sphere.h"
#include "ray.h"
#include <vector>
#include <bits/stdc++.h>

using namespace std;

struct Material {
    Color diffuse;
    Color specular;
    float ambientk;
    float diffusek;
    float speculark;
    float falloff;
};

struct Light {
    Vector3 vec;
    int dirflag;
    Color color;
    float c1, c2, c3;
};

struct rayInfo {
    float distance;
    int id;
};

struct Depth {
    Color depthCol;
    float amin, amax;
    float distMin, distMax;
};

class Raycaster {
    public:
        int width, height, hfov;
        Vector3 eye, viewDir, upDir;
        Color bkgColor;
        vector<Material> materials;
        vector<Sphere> spheres;
        vector<Light> lights;
        Vector3 w, u, v;
        int d = 100;
        float ratio, coordWidth, coordHeight;
        Vector3 normal, viewWidth, viewHeight;
        Vector3 pointUL, pointUR, pointLL, pointLR;
        Vector3 hoffset, voffset, choffset, cvoffset, fullOffset;
        Depth depth = {Color(1, 1, 1), -1, -1, -1, -1};

        // A wrapper that checks for an intersection, then calls shadeRay to apply material prop. and shadows
        Color raycast(Ray viewRay) {
            rayInfo rayGossip = traceRay(viewRay);
            // If no intersection or intersection behind view, return background color
            if (rayGossip.distance < 0) return bkgColor;
            Vector3 intersect = viewRay.getPoint(rayGossip.distance);
            // shadeRay implements material properties and lighting
            return shadeRay(viewRay, intersect, rayGossip.id);
        }

        // Identify the closest intersection - if any - and return the distance to it and the object ID of the intersecting object
        rayInfo traceRay(Ray viewRay, int id = -1) {
            float t, tt;
            float a = 1.0f;
            float min = MAXFLOAT;
            int minSphere = -1;
            // Iterate over all objects
            for(int i = 0; i < spheres.size(); i++) {
                // Calculate B
                float b = 2.0f * (viewRay.getDir().getVectorX() * (viewRay.getOrigin().getVectorX() - spheres[i].getCenter().getVectorX())
                            + viewRay.getDir().getVectorY() * (viewRay.getOrigin().getVectorY() - spheres[i].getCenter().getVectorY())
                            + viewRay.getDir().getVectorZ() * (viewRay.getOrigin().getVectorZ() - spheres[i].getCenter().getVectorZ()));
                // Calculate C
                float c = pow(viewRay.getOrigin().getVectorX() - spheres[i].getCenter().getVectorX(), 2)
                    + pow(viewRay.getOrigin().getVectorY() - spheres[i].getCenter().getVectorY(), 2)
                    + pow(viewRay.getOrigin().getVectorZ() - spheres[i].getCenter().getVectorZ(), 2)
                    - pow(spheres[i].getRadius(), 2);
                // Calculate discriminant
                float discriminant = pow(b, 2) - 4.0f * a * c;
                // Check if there is an intersection point
                if (discriminant > 0.01 && i != id) {
                    // More than 1 solution
                    t = (-b + sqrt(discriminant)) / (2 * a);
                    tt = (-b - sqrt(discriminant)) / (2 * a);
                    if (tt < t && tt > 0) t = tt;
                    if (t < min && t > 0) { 
                        min = t;
                        minSphere = i;
                    }
                }
                else if (discriminant < 0.01 && discriminant > -0.01 && i != id) {
                    // Exactly 1 solution
                    t = (-b) / (2 * a);
                    if (t < min && t > 0) { 
                        min = t;
                        minSphere = i;
                    }
                }
                else {}
            }
            if (minSphere != -1) {
                // Closest intersection t distance from point of ray with object # minSphere
                return {{min},{minSphere}};
            }
            else {
                // No intersection, return sentinel values
                return {{-1},{-1}};
            }
        }

        // Get the unit vector that points from a specific point to a light
        Vector3 getLightDir(Light light, Vector3 surface) {
            if (light.dirflag == 0) return light.vec.scaleVector(-1.0f).getNormalizedVector();
            else return light.vec.subtractVector(surface).getNormalizedVector();
        }

        // Apply material properties and shadows
        Color shadeRay(Ray ray, Vector3 intersection, int objectID) {
            float diffDot, specDot;
            Vector3 ambient, diffuse, specular, diffCon, specCon;
            Vector3 lightDir, shadeNormal, h, v, tempIntensity, intensity;
            intensity = Vector3(0,0,0);
            // Get material of object for reference
            Material activeMtl = materials[spheres[objectID].getMaterial()];
            // Calculate constants for the material
            ambient = activeMtl.diffuse.getAsVector().scaleVector(activeMtl.ambientk);
            diffCon = activeMtl.diffuse.getAsVector().scaleVector(activeMtl.diffusek);
            specCon = activeMtl.specular.getAsVector().scaleVector(activeMtl.speculark);
            // Find normal and get viewing direction (incoming ray)
            shadeNormal = spheres[objectID].getNormal(intersection);
            v = ray.getDir().scaleVector(-1.0f);
            // Sum each lights contribution
            for (int i = 0; i < lights.size(); i++) {
                Vector3 lightColor = lights[i].color.getAsVector();
                lightDir = getLightDir(lights[i], intersection);
                // Calculate diffuse argument - clamps values to prevent negatives
                diffDot = clamp(shadeNormal.dotProduct(lightDir), 0.0f, MAXFLOAT);
                diffuse = diffCon.scaleVector(diffDot);
                // Calculate H unit vector
                h = v.addVector(lightDir);
                h = h.scaleVector(0.5f).getNormalizedVector();
                // Calculate specular argument - clamp values to prevent negatives
                specDot = clamp(shadeNormal.dotProduct(h), 0.0f, MAXFLOAT);
                // Apply falloff value
                specDot = pow(specDot, activeMtl.falloff);
                specular = specCon.scaleVector(specDot);
                // Combine arguments to find final intensity
                tempIntensity = diffuse.addVector(specular).multiplyComponents(lightColor);

                // Cast shadows from the intersection to the light source
                rayInfo rayGossip = traceRay(Ray(intersection, lightDir), objectID);
                // If intersection with shadow ray, evaluate for shadow flag
                bool noShadow = 1;
                // Avoid self intersections by comparing OG intersecting object with shadow intersecting object
                if (0.01 < rayGossip.distance) {
                    if (rayGossip.id != objectID) {
                        // If directional light, if any intersection, there is a shadow
                        if (lights[i].dirflag == 0) noShadow = 0;
                        // If point light, intersection must be within distance from OG intersection to light source
                        else if (rayGossip.distance < lights[i].vec.subtractVector(intersection).magnitude())
                                noShadow = 0;
                    }
                }

                // LIGHT ATTENUATION
                float distance = 1, attenuation = 1;
                if (lights[i].dirflag != 0) {
                    distance = lights[i].vec.subtractVector(intersection).magnitude();
                    attenuation = lights[i].c1 + lights[i].c2 * distance + lights[i].c3 * pow(distance, 2);
                    attenuation = clamp(1.0f / attenuation, 0.0f, 1.0f);
                }

                // Apply shadow flag and add in diffuse/specular arguments
                tempIntensity = tempIntensity.scaleVector(attenuation);
                tempIntensity = tempIntensity.scaleVector(noShadow);
                intensity = intensity.addVector(tempIntensity);
            }
            // Add ambient values
            intensity = intensity.addVector(ambient);

            // DEPTH CUEING
            if (depth.amax != -1) {
                float distance = eye.subtractVector(intersection).magnitude();
                float a;
                if (distance <= depth.distMin) a = depth.amax;
                else if (distance >= depth.distMax) a = depth.amin;
                else {
                    a = depth.amin + (depth.amax - depth.amin) * ((depth.distMax - distance) / (depth.distMax - depth.distMin));
                }
                intensity = intensity.scaleVector(a).addVector(depth.depthCol.getAsVector().scaleVector(1-a));
            }

            // Clamp final values
            intensity = intensity.clampVector(0.0f, 1.0f);
            // Return intensity as a color
            return Color(intensity.getVectorX(), intensity.getVectorY(), intensity.getVectorZ());
        }

};

void printImage(string filename, int ***image, int width, int height){
    // Open file
    ofstream image_file;
    filename = filename.append(".ppm");
    image_file.open(filename);
    string pixel;
    string line;
    // Attach header lines
    image_file << "P3" << endl;
    image_file << width << " " << height << endl;
    image_file << "255" << endl;
    // Append 1 pixel's data to each line
    for(int i = 0; i < height; i++){
        for (int j = 0; j < width; j++){
            pixel = "";
            for (int k = 0; k < 3; k++){
                pixel += to_string(image[i][j][k]);
                // Don't add unnecessary whitespace
                if (k < 2){
                    pixel += " ";
                }
            }
            image_file << pixel << endl;
        }
    }
    cout << "printed image as " << filename << endl;
    // Close the file
    image_file.close();
}

// Break a line down into tokens
vector<string> tokenizer(string line) {
    vector<string> tokens;
    stringstream check1(line);
    string temp;
    while(getline(check1, temp, ' ')) {
        tokens.push_back(temp);
    }
    return tokens;
}

int main(int argc, char *argv[]){
    // Catch for no arguments
    if (argc <= 1) {
        cerr << "Invalid arguments, please enter a image description filename" << endl;
        return -1;
    }

    ifstream image_descriptor;
    image_descriptor.open(argv[1]);

    string line, keyword;
    bool imsizeFound = false, eyeFound = false, viewDirFound = false, upDirFound = false;
    bool hfovFound = false, bkgColorFound = false, mtlColorFound = false, lightFound = false;
    float tempx, tempy, tempz, tempr;
    Raycaster raycaster;

    vector<string> tokens;
    // Catch for invalid filename
    if (image_descriptor.is_open()){
        // Read each line in the file
        while (getline(image_descriptor, line)){
            
            tokens = tokenizer(line);
            // If there is no keyword and value combo, skip
            if (tokens.size() > 0){
                keyword = tokens[0];

                // Image Dimensions
                if (keyword.compare("imsize") == 0){
                    imsizeFound = true;
                    // Try to read in width and height
                    try{
                        raycaster.width = stoi(tokens[1]);
                        raycaster.height = stoi(tokens[2]);
                        if (raycaster.width <= 0 || raycaster.height <= 0) throw invalid_argument("dimensions cannot be 0 or negative");
                    }
                    catch (exception e){
                        cout << "Invalid arguments, please recheck image descriptor guidelines" << endl;
                        image_descriptor.close();
                        return -1;
                    }
                }

                // Eye
                if (keyword.compare("eye") == 0){
                    eyeFound = true;
                    try{
                        tempx = stof(tokens[1]);
                        tempy = stof(tokens[2]);
                        tempz = stof(tokens[3]);
                        raycaster.eye = Vector3(tempx, tempy, tempz);
                    }
                    catch (exception e){
                        cout << "Invalid eye position" << endl;
                        image_descriptor.close();
                        return -1;
                    }
                }

                // Viewdir
                if (keyword.compare("viewdir") == 0){
                    viewDirFound = true;
                    try{
                        tempx = stof(tokens[1]);
                        tempy = stof(tokens[2]);
                        tempz = stof(tokens[3]);
                        raycaster.viewDir = Vector3(tempx, tempy, tempz);
                    }
                    catch (exception e){
                        cout << "Invalid view direction" << endl;
                        image_descriptor.close();
                        return -1;
                    }
                }

                // Updir
                if (keyword.compare("updir") == 0){
                    upDirFound = true;
                    try{
                        tempx = stof(tokens[1]);
                        tempy = stof(tokens[2]);
                        tempz = stof(tokens[3]);
                        raycaster.upDir = Vector3(tempx, tempy, tempz);
                    }
                    catch (exception e){
                        cout << "Invalid up direction" << endl;
                        image_descriptor.close();
                        return -1;
                    }
                }

                // FOV
                if (keyword.compare("hfov") == 0){
                    hfovFound = true;
                    try{
                        raycaster.hfov = stoi(tokens[1]);
                    }
                    catch (exception e){
                        cout << "Invalid hfov" << endl;
                        image_descriptor.close();
                        return -1;
                    }
                }

                // Background Color
                if (keyword.compare("bkgcolor") == 0){
                    bkgColorFound = true;
                    try{
                        tempx = stof(tokens[1]);
                        tempy = stof(tokens[2]);
                        tempz = stof(tokens[3]);
                        raycaster.bkgColor = Color(tempx, tempy, tempz);
                    }
                    catch (exception e){
                        cout << "Invalid color" << endl;
                        image_descriptor.close();
                        return -1;
                    }
                }

                // Attenuated Light
                if (keyword.compare("light") == 0){
                    try{
                        Light templ = { {Vector3(stof(tokens[1]), stof(tokens[2]), stof(tokens[3]))},
                                        {stoi(tokens[4])},
                                        {Color(stof(tokens[5]), stof(tokens[6]), stof(tokens[7]))},
                                        {1}, {0}, {0}
                                        };
                        raycaster.lights.push_back(templ);
                        lightFound = true;
                    }
                    catch (exception e){
                        cout << "Invalid color" << endl;
                        image_descriptor.close();
                        return -1;
                    }
                }

                // Light
                if (keyword.compare("attlight") == 0){
                    try{
                        Light templ = { {Vector3(stof(tokens[1]), stof(tokens[2]), stof(tokens[3]))},
                                        {stoi(tokens[4])},
                                        {Color(stof(tokens[5]), stof(tokens[6]), stof(tokens[7]))},
                                        {stof(tokens[8])}, {stof(tokens[9])}, {stof(tokens[10])}
                                        };
                        raycaster.lights.push_back(templ);
                        lightFound = true;
                    }
                    catch (exception e){
                        cout << "Invalid color" << endl;
                        image_descriptor.close();
                        return -1;
                    }
                }

                // Depth Cueing
                if (keyword.compare("depthcueing") == 0){
                    try{
                        Depth temp = { {Color(stof(tokens[1]), stof(tokens[2]), stof(tokens[3]))},
                                        {stof(tokens[4])}, {stof(tokens[5])},
                                        {stof(tokens[6])}, {stof(tokens[7])}
                                        };
                        raycaster.depth = temp;
                    }
                    catch (exception e){
                        cout << "Invalid depth cueing" << endl;
                        image_descriptor.close();
                        return -1;
                    }
                }

                // Material Color
                if (keyword.compare("mtlcolor") == 0){
                    mtlColorFound = true;
                    try{
                        Material tempm = {  {Color(stof(tokens[1]), stof(tokens[2]), stof(tokens[3]))},
                                            {Color(stof(tokens[4]), stof(tokens[5]), stof(tokens[6]))},
                                            {stof(tokens[7])}, {stof(tokens[8])}, {stof(tokens[9])}, {stof(tokens[10])}
                                            };
                        raycaster.materials.push_back(tempm);
                    }
                    catch (exception e){
                        cout << "Invalid color" << endl;
                        image_descriptor.close();
                        return -1;
                    }
                }

                // Spheres
                if (keyword.compare("sphere") == 0){
                    // Try to save seed
                    try{
                        tempx = stof(tokens[1]);
                        tempy = stof(tokens[2]);
                        tempz = stof(tokens[3]);
                        tempr = stof(tokens[4]);
                        if (mtlColorFound) raycaster.spheres.push_back(Sphere(Vector3(tempx, tempy, tempz), tempr, raycaster.materials.size()-1));
                        else throw invalid_argument("A sphere requires a material color to be set");
                    }
                    catch (exception e){
                        cout << "Invalid sphere" << endl;
                        image_descriptor.close();
                        return -1;
                    }
                }
            }
        }

        // If imsize was not found in file, return with an error
        if (!imsizeFound && !eyeFound && !viewDirFound && !hfovFound && !bkgColorFound && !lightFound){
            cout << "Invalid arguments, please recheck image descriptor guidelines" << endl;
            image_descriptor.close();
            return -1;            
        }
        // Close file
        image_descriptor.close();
    }
    else{
        cout << "Invalid filename" << endl;
        return -1;
    }

    // Set up viewing parameters
    raycaster.w = Vector3(-raycaster.viewDir.getVectorX(), -raycaster.viewDir.getVectorY(), -raycaster.viewDir.getVectorZ());
    raycaster.u = raycaster.viewDir.crossProduct(raycaster.upDir).getNormalizedVector();
    raycaster.v = raycaster.u.crossProduct(raycaster.viewDir).getNormalizedVector();
    raycaster.ratio = (float) raycaster.width / (float) raycaster.height;
    raycaster.coordWidth = 2.0 * raycaster.d * tan(0.5 * ((raycaster.hfov * M_PI) / 180));
    raycaster.coordHeight = raycaster.coordWidth / raycaster.ratio;
    raycaster.normal = raycaster.viewDir.getNormalizedVector().scaleVector(raycaster.d);
    raycaster.normal = raycaster.viewDir.addVector(raycaster.normal);
    raycaster.viewWidth = raycaster.u.scaleVector(raycaster.coordWidth / 2.0f);
    raycaster.viewHeight = raycaster.v.scaleVector(raycaster.coordHeight / 2.0f);
    // Viewing window coordinate calculations
    raycaster.pointUL = raycaster.normal.subtractVector(raycaster.viewWidth).addVector(raycaster.viewHeight);
    raycaster.pointUR = raycaster.normal.addVector(raycaster.viewWidth).addVector(raycaster.viewHeight);
    raycaster.pointLL = raycaster.normal.subtractVector(raycaster.viewWidth).subtractVector(raycaster.viewHeight);
    raycaster.pointLR = raycaster.normal.addVector(raycaster.viewWidth).subtractVector(raycaster.viewHeight);
    // Offsets
    raycaster.hoffset = raycaster.pointUR.subtractVector(raycaster.pointUL).scaleVector(1.0 / raycaster.width);
    raycaster.voffset = raycaster.pointLL.subtractVector(raycaster.pointUL).scaleVector(1.0 / raycaster.height);
    raycaster.choffset = raycaster.pointUR.subtractVector(raycaster.pointUL).scaleVector(1.0 / (2.0 * raycaster.width));
    raycaster.cvoffset = raycaster.pointLL.subtractVector(raycaster.pointUL).scaleVector(1.0 / (2.0 * raycaster.height));
    raycaster.fullOffset = raycaster.pointUL.addVector(raycaster.choffset).addVector(raycaster.cvoffset);

    // Printers
    /*
    cout << "wxh "<< raycaster.width << " " << raycaster.height << endl;
    cout << "eye ";
    raycaster.eye.print();
    cout << "viewDir ";
    raycaster.viewDir.print();
    cout << "upDir ";
    raycaster.upDir.print();
    cout << "hfov " << raycaster.hfov << endl;
    cout << "bkgColor ";
    raycaster.bkgColor.print();
    for (int i = 0; i < raycaster.spheres.size(); i++) {
        cout << "sphere #" << i << " ";
        raycaster.spheres[i].print();
    }
    cout << "w ";
    raycaster.w.print();
    cout << "u ";
    raycaster.u.print();
    cout << "v ";
    raycaster.v.print();
    cout << "ratio " << raycaster.ratio << endl;
    cout << "Coord wxh " << raycaster.coordWidth << " " << raycaster.coordHeight << endl;
    cout << "ul point ";
    raycaster.pointUL.print();
    cout << "ur point ";
    raycaster.pointUR.print();
    cout << "ll point ";
    raycaster.pointLL.print();
    cout << "lr point ";
    raycaster.pointLR.print();
    cout << "hoffset ";
    raycaster.hoffset.print();
    cout << "voffset ";
    raycaster.voffset.print();
    raycaster.fullOffset.print();
    */


    // Intialize memory block for new picture
    int ***newImage = new int**[raycaster.height];
    for(int i = 0; i < raycaster.height; i++){
        newImage[i] = new int*[raycaster.width];
        for (int j = 0; j < raycaster.width; j++){
            newImage[i][j] = new int[3];
            for (int k = 0; k < 3; k++){
                // Instantiate each pixel and rgb value to 255
                newImage[i][j][k] = 255;
            }
        }
    }

    // Run the new picture array through the image creator
    Vector3 point;
    Color pixColor;
    try {
        for(int i = 0; i < raycaster.height; i++){
            for (int j = 0; j < raycaster.width; j++){
                // Find new target point on the coord grid
                point = raycaster.fullOffset.addVector(raycaster.hoffset.scaleVector((float) j)).addVector(raycaster.voffset.scaleVector((float) i));
                pixColor = raycaster.raycast(Ray(raycaster.eye, point.subtractVector(raycaster.eye).getNormalizedVector()));
                newImage[i][j][0] = pixColor.getColorR(true);
                newImage[i][j][1] = pixColor.getColorG(true);
                newImage[i][j][2] = pixColor.getColorB(true);
            }
        }
    }
    catch (exception e){
        cout << "Something went wrong during image manipulation";
        cout << e.what() << endl;
    }

    // Save array to file
    try{
        string filename = argv[1];
        int delimiter = filename.find(".");
        printImage(filename.substr(0, delimiter), newImage, raycaster.width, raycaster.height);
    }
    catch (exception e){
        cerr << "Something went wrong during image saving";
        cerr << e.what() << endl;
    }

    return 0;
}