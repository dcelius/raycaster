# graphics-fundamentals

## Raycaster
raycaster takes in the filename of an input text file to generate a .ppm file with the corresponding specifications. 

### The dimensions of the rendered image (in pixels)
imsize [width] [height]

### The camera origin (a 3D vector)
eye   [eyex] [eyey] [eyez]

### The viewing direction (a 3D vector)
viewdir   [vdirx]  [vdiry]  [vdirz]

### The 'up' direction (a 3D vector)
updir   [upx]  [upy]  [upz]

### The horizontal field of view (in degrees)
hfov   [fovh]

### The background color (values from 0-1)
bkgcolor   [r]  [g]  [b]

### A material color (values from 0-1). All subsequently-defined objects will use the immediately-preceding material color
mtlcolor   [r]  [g]  [b]

### A sphere definition (a 3D vector plus the radius)
sphere   [cx]  [cy]  [cz]  [r]

## Compilation
The program can be compiled using: <br>
make all

## Execution
The program can be run using: <br>
./raycaster [input file]
