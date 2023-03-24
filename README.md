# graphics-fundamentals

## Raycaster
raycaster takes in the filename of an input text file to generate a .ppm file with the corresponding specifications. 

### The dimensions of the rendered image (in pixels) - required
imsize [width] [height]

### The camera origin (a 3D point) - required
eye [eyex] [eyey] [eyez]

### The viewing direction (a 3D vector) - required
viewdir [vdirx] [vdiry] [vdirz]

### The 'up' direction (a 3D vector)
updir [upx] [upy] [upz]

### The horizontal field of view (in degrees) - required
hfov [fovh]

### The background color (values from 0-1) along with an optional refraction index
bkgcolor [r] [g] [b] [eta]

### The depth cueing definition (values of *a* must be between 0-1)
depthcueing [r] [g] [b] [a-max] [a-min] [dist-max] [dist-min]

### A light definition (the dir flag controls if the light is a point light source (1) or a directional light source (0). x,y,z are a position if it is a point light source, and a vector otherwise)
light [x] [y] [z] [dirflag] [r] [g] [b]

### A definition of a light source that is attenuated (follows the same rules as a light definition with the addition of 3 constant values)
attlight [x] [y] [z] [dirflag] [r] [g] [b] [c1] [c2] [c3]

### A material definition following an extended Phong illumination model (values from 0-1). All subsequently-defined objects will use the immediately-preceding material color. 
mtlcolor [Odr] [Odg] [Odb] [Osr] [Osg] [Osb] [ka] [kd] [ks] [falloff] [alpha] [eta]

### A texture definition (must be a valid .ppm file)
texture [tex].ppm

### A sphere definition (a 3D vector plus the radius)
sphere [cx] [cy] [cz] [r]

### A vertex defintion (a 3D point)
v [x] [y] [z]

### A vertex normal definition (a 3D vector)
vn [x] [y] [z]

### A texture coordinate definition (a 2D point, values from 0-1)
vt [u] [v]

### A triangle definition (values are indices into vertex arrays, starting from 1, not 0). Requires a minimum of 3 values, but should be able to handle any number of vertex sets (automatically breaks complex faces into smaller triangles)
f [v1/vn1/vt1] [v2/vn2/vt2] [v3/vn3/vt3] ... <br>
f [v1//vt1] [v2//vt2] [v3//vt3] ... <br>
f [v1/vt1] [v2/vt2] [v3/vt3] ... <br>
f [v1] [v2] [v3] ...

## Compilation
The program can be compiled using: <br>
make all

## Execution
The program can be run using: <br>
./raycaster1d [input file]
