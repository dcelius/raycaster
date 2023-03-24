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
    float alpha;
    float eta;
};

struct Light {
    Vector3 vec;
    int dirflag;
    Color color;
    float c1, c2, c3;
};

struct rayInfo {
    float distance;
    int id, objType;
    float baryx, baryy, baryz;
};

struct Depth {
    Color depthCol;
    float amax, amin;
    float distMax, distMin;
};

const int SPHERE_OBJ_TYPE = 0;
const int TRIANGLE_OBJ_TYPE = 1;

class Raycaster {
    public:
        int width, height, hfov;
        Vector3 eye, viewDir, upDir;
        Color bkgColor;
        vector<Material> materials;
        vector<Sphere> spheres;
        vector<Light> lights;
        vector<Vector3> vertices;
        vector<Vector3> vnormals;
        vector<Vector3> vtextures;
        vector<array<int, 11>> faces;
        vector<vector<vector<Color>>> textures;
        Vector3 w, u, v;
        int d = 250;
        float ratio, coordWidth, coordHeight;
        Vector3 normal, viewWidth, viewHeight;
        Vector3 pointUL, pointUR, pointLL, pointLR;
        Vector3 hoffset, voffset, choffset, cvoffset, fullOffset;
        Depth depth = {Color(1, 1, 1), -1, -1, -1, -1};

        float clip(float n, float lower, float upper) {
            return max(lower, min(n, upper));
        }

        // A wrapper that checks for an intersection, then calls shadeRay to apply material prop. and shadows
        Color raycast(Ray viewRay) {
            rayInfo rayGossip = traceRay(viewRay);
            // If no intersection or intersection behind view, return background color
            if (rayGossip.distance < 0) return bkgColor;
            Vector3 intersect = viewRay.getPoint(rayGossip.distance);
            // shadeRay implements material properties and lighting
            return shadeRay(viewRay, intersect, rayGossip);
        }

        // Identify the closest intersection - if any - and return the distance to it and the object ID of the intersecting object
        rayInfo traceRay(Ray viewRay, int id = -1, int objType = -1) {
            float t, tt;
            float a = 1.0f;
            float min = MAXFLOAT;
            int minObj = -1;
            int intersectObjType = -1;
            float baryx = -1, baryy = -1, baryz = -1;
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
                if (discriminant > 0.01 && (i != id || objType != SPHERE_OBJ_TYPE)) {
                    // More than 1 solution
                    t = (-b + sqrt(discriminant)) / (2 * a);
                    tt = (-b - sqrt(discriminant)) / (2 * a);
                    if (tt < t && tt > 0) t = tt;
                    if (t < min && t > 0) { 
                        min = t;
                        minObj = i;
                        intersectObjType = SPHERE_OBJ_TYPE;
                    }
                }
                else if (discriminant < 0.01 && discriminant > -0.01 && (i != id || objType != SPHERE_OBJ_TYPE)) {
                    // Exactly 1 solution
                    t = (-b) / (2 * a);
                    if (t < min && t > 0) { 
                        min = t;
                        minObj = i;
                        intersectObjType = SPHERE_OBJ_TYPE;
                    }
                }
                else {}
            }
            
            for(int i = 0; i < faces.size(); i++) {
                // ABC from Normal
                Vector3 e1 = vertices[faces[i][1]].subtractVector(vertices[faces[i][0]]);
                Vector3 e2 = vertices[faces[i][2]].subtractVector(vertices[faces[i][0]]);
                Vector3 normal = e1.crossProduct(e2);
                float a = normal.getVectorX();
                float b = normal.getVectorY();
                float c = normal.getVectorZ();
                // D calculated from point on triangle/plane
                float d = -1.0f * (a * vertices[faces[i][0]].getVectorX() +
                            b * vertices[faces[i][0]].getVectorY() +
                            c * vertices[faces[i][0]].getVectorZ());
                // Calc denominator
                float denominator = a * viewRay.getDir().getVectorX() + b * viewRay.getDir().getVectorY() + c * viewRay.getDir().getVectorZ();
                // No solution if denom == 0
                if (denominator < -0.00001 || denominator > 0.00001) t = (a * viewRay.getOrigin().getVectorX() + b * viewRay.getOrigin().getVectorY() +
                    c * viewRay.getOrigin().getVectorZ() + d) / denominator * -1;
                else t = -1;
                // Only care about positive distance and new min distances
                if (t > 0 && t < min  && (i != id || objType != TRIANGLE_OBJ_TYPE)) {
                    // Find barycentric coordinates
                    Vector3 ep = viewRay.getPoint(t).subtractVector(vertices[faces[i][0]]);                    
                    float d11 = e1.dotProduct(e1);
                    float d22 = e2.dotProduct(e2);
                    float d12 = e1.dotProduct(e2);
                    float det = (d11 * d22) - (d12 * d12);

                    // if det == 0, no solution
                    //cout << d11 << " " << d22 << " " << d12 << endl;
                    if (det < 0.00001 && det > -0.00001) t = -1;
                    float beta = (d22 * e1.dotProduct(ep)
                                    - d12 * e2.dotProduct(ep)) / det;
                    float lambda = (d11 * e2.dotProduct(ep)
                                    - d12 * e1.dotProduct(ep)) / det;
                    // If within these bounds, point is in triangle
                    if (beta + lambda < 1 && beta > 0 && lambda > 0 && t != -1) {
                        min = t;
                        minObj = i;
                        intersectObjType = TRIANGLE_OBJ_TYPE;
                        baryx = 1-beta-lambda;
                        baryy = beta;
                        baryz = lambda;
                    }
                }

            }

            if (minObj != -1) {
                // Closest intersection t distance from point of ray with object # minObj
                return {{min}, {minObj}, {intersectObjType}, {baryx}, {baryy}, {baryz}};
            }
            else {
                // No intersection, return sentinel values
                return {{-1}, {-1}, {-1}};
            }
        }

        // Get the unit vector that points from a specific point to a light
        Vector3 getLightDir(Light light, Vector3 surface) {
            if (light.dirflag == 0) return light.vec.scaleVector(-1.0f).getNormalizedVector();
            else return light.vec.subtractVector(surface).getNormalizedVector();
        }

        // Apply material properties and shadows
        Color shadeRay(Ray ray, Vector3 intersection, rayInfo rayGossip) {
            int objectID = rayGossip.id;
            int objType = rayGossip.objType;
            float diffDot, specDot;
            Vector3 ambient, diffuse, specular, diffCon, specCon;
            Vector3 lightDir, shadeNormal, h, v, tempIntensity, intensity;
            intensity = Vector3(0,0,0);
            // Get material of object for reference
            Material activeMtl;
            if(objType == SPHERE_OBJ_TYPE) activeMtl = materials[spheres[objectID].getMaterial()];
            else if(objType == TRIANGLE_OBJ_TYPE) activeMtl = materials[faces[objectID][9]];
            // Find normal and get viewing direction (incoming ray)
            if(objType == SPHERE_OBJ_TYPE) shadeNormal = spheres[objectID].getNormal(intersection);
            else if(objType == TRIANGLE_OBJ_TYPE) {
                // Flat triangles can just use the plane normal
                if(faces[objectID][6] == -1) {
                    Vector3 e1 = vertices[faces[objectID][1]].subtractVector(vertices[faces[objectID][0]]);
                    Vector3 e2 = vertices[faces[objectID][2]].subtractVector(vertices[faces[objectID][0]]);
                    Vector3 normal = e1.crossProduct(e2);
                    shadeNormal = normal.getNormalizedVector();
                }
                else {
                    // If smooth shading, then calc weighted normal
                    shadeNormal = vnormals[faces[objectID][6]].scaleVector(rayGossip.baryx).addVector(
                                    vnormals[faces[objectID][7]].scaleVector(rayGossip.baryy).addVector(
                                    vnormals[faces[objectID][8]].scaleVector(rayGossip.baryz))).getNormalizedVector();
                }
            }

            // Set default colors
            diffuse = activeMtl.diffuse.getAsVector();
            specular = activeMtl.specular.getAsVector();

            // Handle textures
            double psi, theta, texu, texv;
            bool texUsed = false;
            int texID;
            if(objType == SPHERE_OBJ_TYPE) {
                texID = spheres[objectID].getTexture();
                if (texID != -1) {
                    texUsed = true;
                    psi = acos(shadeNormal.getVectorZ());
                    texv = psi / M_PI;
                    theta = atan2(shadeNormal.getVectorY(), shadeNormal.getVectorX());
                    // Rotate to meet 0 < theta < 2pi range
                    if (theta < 0) theta = theta + (2 * M_PI);
                    texu = theta / (2 * M_PI);
                }
            }
            else if(objType == TRIANGLE_OBJ_TYPE) {
                texID = faces[objectID][10];
                if (texID != -1 && faces[objectID][3] != -1) {                    texUsed = true;
                    texu = vtextures[faces[objectID][3]].getVectorX() * rayGossip.baryx +
                            vtextures[faces[objectID][4]].getVectorX() * rayGossip.baryy +
                            vtextures[faces[objectID][5]].getVectorX() * rayGossip.baryz;
                    texv = vtextures[faces[objectID][3]].getVectorY() * rayGossip.baryx +
                            vtextures[faces[objectID][4]].getVectorY() * rayGossip.baryy +
                            vtextures[faces[objectID][5]].getVectorY() * rayGossip.baryz;
                }
            }
            if (texUsed) {
                // Bilinearaly interpolate value
                float x, y;
                int i, j;
                float a, b;
                x = texu * (textures[texID][0].size() - 1);
                y = texv * (textures[texID].size() - 1);
                i = floor(x); j = floor(y);
                a = x - i; b = y - j;
                diffuse = textures[texID][j][i].getAsVector().scaleVector((1-a)*(1-b)).addVector(
                    textures[texID][j][i+1].getAsVector().scaleVector((a)*(1-b)).addVector(
                    textures[texID][j+1][i].getAsVector().scaleVector((1-a)*(b)).addVector(
                    textures[texID][j+1][i+1].getAsVector().scaleVector((a)*(b)))));
            }

            // Calculate constants for the material
            ambient = diffuse.scaleVector(activeMtl.ambientk);
            diffCon = diffuse.scaleVector(activeMtl.diffusek);
            specCon = activeMtl.specular.getAsVector().scaleVector(activeMtl.speculark);

            v = ray.getDir().scaleVector(-1.0f);
            // Sum each lights contribution
            for (int i = 0; i < lights.size(); i++) {
                Vector3 lightColor = lights[i].color.getAsVector();
                lightDir = getLightDir(lights[i], intersection);
                // Calculate diffuse argument - clamps values to prevent negatives
                diffDot = clip(shadeNormal.dotProduct(lightDir), 0.0f, MAXFLOAT);
                diffuse = diffCon.scaleVector(diffDot);
                // Calculate H unit vector
                h = v.addVector(lightDir);
                h = h.scaleVector(0.5f).getNormalizedVector();
                // Calculate specular argument - clamp values to prevent negatives
                specDot = clip(shadeNormal.dotProduct(h), 0.0f, MAXFLOAT);
                // Apply falloff value
                specDot = pow(specDot, activeMtl.falloff);
                specular = specCon.scaleVector(specDot);
                // Combine arguments to find final intensity
                tempIntensity = diffuse.addVector(specular).multiplyComponents(lightColor);

                // Cast shadows from the intersection to the light source
                rayInfo newRayGossip = traceRay(Ray(intersection, lightDir), objectID, objType);
                // If intersection with shadow ray, evaluate for shadow flag
                bool noShadow = 1;
                // Avoid self intersections by comparing OG intersecting object with shadow intersecting object
                if (newRayGossip.distance > 0.01) {
                    if (newRayGossip.id != objectID  || newRayGossip.objType != objType) {
                        // If directional light, if any intersection, there is a shadow
                        if (lights[i].dirflag == 0) noShadow = 0;
                        // If point light, intersection must be within distance from OG intersection to light source
                        else if (newRayGossip.distance < lights[i].vec.subtractVector(intersection).magnitude())
                                noShadow = 0;
                    }
                }

                // LIGHT ATTENUATION
                float distance = 1, attenuation = 1;
                if (lights[i].dirflag != 0) {
                    distance = lights[i].vec.subtractVector(intersection).magnitude();
                    attenuation = lights[i].c1 + lights[i].c2 * distance + lights[i].c3 * pow(distance, 2);
                    attenuation = clip(1.0f / attenuation, 0.0f, 1.0f);
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

// Trim function - https://stackoverflow.com/questions/216823/how-to-trim-an-stdstring
const char* WS_CHARACTERS = " \t\n\r\f\v";

// trim from end of string (right)
inline std::string& rtrim(std::string& s, const char* t = WS_CHARACTERS)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from beginning of string (left)
inline std::string& ltrim(std::string& s, const char* t = WS_CHARACTERS)
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from both ends of string (right then left)
inline std::string& trim(std::string& s, const char* t = WS_CHARACTERS)
{
    return ltrim(rtrim(s, t), t);
}

// Break a line down into tokens
vector<string> tokenizer(string line) {
    vector<string> tokens;
    stringstream check1(line);
    string temp;
    while(getline(check1, temp, ' ')) if (temp.size() != 0) tokens.push_back(trim(temp));
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
    bool textureFound = false;
    float tempx, tempy, tempz, tempr;
    Raycaster raycaster;

    vector<string> tokens;
    // Catch for invalid filename
    if (image_descriptor.is_open()){
        // Ensure vertices are aligned starting at 1
        raycaster.vertices.push_back(Vector3(0,0,0));
        raycaster.vnormals.push_back(Vector3(0,0,0));
        raycaster.vtextures.push_back(Vector3(0,0,0));
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
                        float eta = 0;
                        if (tokens.size() > 4) eta = stof(tokens[4]);
                        raycaster.bkgColor = Color(tempx, tempy, tempz, eta);
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
                                            {stof(tokens[7])}, {stof(tokens[8])}, {stof(tokens[9])}, {stof(tokens[10])},
                                            {stof(tokens[11])}, {stof(tokens[12])}
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
                        int tex = -1;
                        tempx = stof(tokens[1]);
                        tempy = stof(tokens[2]);
                        tempz = stof(tokens[3]);
                        tempr = stof(tokens[4]);
                        if (textureFound) tex = raycaster.textures.size()-1;
                        if (mtlColorFound) raycaster.spheres.push_back(Sphere(Vector3(tempx, tempy, tempz), tempr, raycaster.materials.size()-1, tex));
                        else throw invalid_argument("A sphere requires a material color to be set");
                    }
                    catch (exception e){
                        cout << "Invalid sphere" << endl;
                        image_descriptor.close();
                        return -1;
                    }
                }

                // Vertices
                if (keyword.compare("v") == 0) {
                    try{
                        tempx = stof(tokens[1]);
                        tempy = stof(tokens[2]);
                        tempz = stof(tokens[3]);
                        raycaster.vertices.push_back(Vector3(tempx, tempy, tempz));     
                    }
                    catch (exception e){
                        cout << "Invalid vertex" << endl;
                        image_descriptor.close();
                        return -1;
                    }          
                }

                // Vertex Normals
                if (keyword.compare("vn") == 0) {
                    try{
                        tempx = stof(tokens[1]);
                        tempy = stof(tokens[2]);
                        tempz = stof(tokens[3]);
                        raycaster.vnormals.push_back(Vector3(tempx, tempy, tempz).getNormalizedVector());     
                    }
                    catch (exception e){
                        cout << "Invalid vertex normal" << endl;
                        image_descriptor.close();
                        return -1;
                    }          
                }

                // Texture Coordinates
                if (keyword.compare("vt") == 0) {
                    try{
                        tempx = stof(tokens[1]);
                        tempy = stof(tokens[2]);
                        raycaster.vtextures.push_back(Vector3(tempx, tempy, 0));     
                    }
                    catch (exception e){
                        cout << "Invalid texture coordinate" << endl;
                        image_descriptor.close();
                        return -1;
                    }          
                }

                // Triangles
                if (keyword.compare("f") == 0) {
                    if (tokens.size() < 4) {
                        cout << "Invalid face description" << endl;
                        return -1;
                    }
                    const char* vertex1str = tokens[1].c_str();
                    array<int, 3> vertex1data = {-1, -1, -1};
                    if (sscanf(vertex1str, "%d/%d/%d", &vertex1data[0], &vertex1data[1], &vertex1data[2]) == 3) {
                    //success reading a vertex in v/t/n format; proceed accordingly
                    } else if (sscanf(vertex1str, "%d//%d", &vertex1data[0], &vertex1data[2]) == 2) {
                    //success reading a vertex in v//n format; proceed accordingly
                    } else if (sscanf(vertex1str, "%d/%d", &vertex1data[0], &vertex1data[1]) == 2) {
                    //success reading a vertex in v/t format; proceed accordingly
                    } else if (sscanf(vertex1str, "%d", &vertex1data[0]) == 1) {
                    //success reading a vertex in v format; proceed accordingly
                    } else{
                    //error reading face data
                        cout << "Invalid face description" << endl;
                        image_descriptor.close();
                        return -1;
                    }

                    for (int i = 2; i < tokens.size() - 1; i++) {
                        // First 3 for vertices, then normals, then texture coordinates, then 1 id for mtl and 1 id for texture
                        array<int, 11> face = {vertex1data[0], -1, -1, vertex1data[1], -1, -1, vertex1data[2], -1, -1, -1, -1};
                        for (int j = 0; j < 2; j++) {
                            const char* faceline = tokens[i+j].c_str();
                            if (sscanf(faceline, "%d/%d/%d", &face[1+j], &face[4+j], &face[7+j]) == 3) {
                            //success reading a vertex in v/t/n format; proceed accordingly
                            } else if (sscanf(faceline, "%d//%d", &face[1+j], &face[7+j]) == 2) {
                            //success reading a vertex in v//n format; proceed accordingly
                            } else if (sscanf(faceline, "%d/%d", &face[1+j], &face[4+j]) == 2) {
                            //success reading a vertex in v/t format; proceed accordingly
                            } else if (sscanf(faceline, "%d", &face[1+j]) == 1) {
                            //success reading a vertex in v format; proceed accordingly
                            } else{
                            //error reading face data
                                cout << "Invalid face description" << endl;
                                image_descriptor.close();
                                return -1;
                            }
                        }
                        if (mtlColorFound) face[9] = raycaster.materials.size()-1;
                        else {
                            cout << "No color has been defined yet!" << endl;
                            image_descriptor.close();
                            return -1;
                        }
                        if (textureFound) face[10] = raycaster.textures.size()-1;
                        //cout << "Face #" << raycaster.faces.size();
                        //for (int k = 0; k < face.size(); k++) {
                        //    cout << " " << face[k];
                        //}
                        //cout << endl;
                        raycaster.faces.push_back(face);
                    }
                }

                if (keyword.compare("texture") == 0) {
                    ifstream texture_file;
                    try {
                        texture_file.open(tokens[1]);
                        if (texture_file.is_open()) {
                            // Create variables
                            vector<vector<Color>> texture;
                            vector<Color> row;
                            vector<string> ppm_tokens;
                            string tex_line;
                            int u = 0, v = 0;
                            int tex_width, tex_height;
                            // Set initial parameters
                            getline(texture_file, tex_line);
                            ppm_tokens = tokenizer(tex_line);
                            tex_width = stoi(ppm_tokens[1]);
                            tex_height = stoi(ppm_tokens[2]);
                            int color_max = stoi(ppm_tokens[3]);
                            // Loop through all lines
                            while (getline(texture_file, tex_line)){
                                // Break line into parts
                                ppm_tokens = tokenizer(tex_line);
                                // For every 3 numbers, make a color and add it to the row vector - every 4th token is a space, so skip it
                                for (int i = 0; i < ppm_tokens.size()-3; i = i + 3) {
                                    row.push_back(Color(stof(ppm_tokens[i]) / color_max, stof(ppm_tokens[i+1]) / color_max, stof(ppm_tokens[i+2]) / color_max));
                                    u++;
                                    // next row!
                                    if (u >= tex_width) {
                                        u = 0;
                                        v++;
                                        texture.push_back(row);
                                        row.clear();
                                    }
                                }
                            }
                            texture_file.close();
                            raycaster.textures.push_back(texture);
                            textureFound = true;
                        }
                    }
                    catch (exception e) {
                        cout << "Error in reading texture file" << endl;
                        texture_file.close();
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
    /*
    for (int i = 1; i < raycaster.vertices.size(); i++) {
        cout << "Vertex #" << i << endl;
        raycaster.vertices[i].print();
    }
    for (int i = 1; i < raycaster.vnormals.size(); i++) {
        cout << "Vertex normal #" << i << endl;
        raycaster.vnormals[i].print();
    }
    for (int i = 1; i < raycaster.vtextures.size(); i++) {
        cout << "Texture Coordinate #" << i << endl;
        raycaster.vtextures[i].print();
    }
    for (int i = 0; i < raycaster.faces.size(); i++) {
        cout << "Face #" << i << endl;
        raycaster.vertices[raycaster.faces[i][0]].print();
        raycaster.vertices[raycaster.faces[i][1]].print();
        raycaster.vertices[raycaster.faces[i][2]].print();
        if (raycaster.faces[i][3] != -1) {
            cout << "Texture Coordinates" << endl;
            raycaster.vtextures[raycaster.faces[i][3]].print();
            raycaster.vtextures[raycaster.faces[i][4]].print();
            raycaster.vtextures[raycaster.faces[i][5]].print();
        }
        if (raycaster.faces[i][6] != -1) {
            cout << "Vertex Normals" << endl;
            raycaster.vnormals[raycaster.faces[i][6]].print();
            raycaster.vnormals[raycaster.faces[i][7]].print();
            raycaster.vnormals[raycaster.faces[i][8]].print();
        }
    }
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