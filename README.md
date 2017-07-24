# Multi-Threaded Fractal Terrain

to compile the computation part, run "make"

for help with args type ./compute.o (with no args)


To render the generated fractal terrain, run on a linux machine with OpenGL (not linab or hpc)
To compile the render part, run "make render"
run ./render or ./render [directory with data] to render the generated data

The algorithm used for generation is the diamond square algorithm.
To generate the colors; a red, green and blue colormap are generated seperately 
and then used as the rgb values for the respective polygon. 

## Render Controls;
w,a,s,d: move terrain

[spacebar],c: move terrain up and down

i,j,k,l: rotate terrain

r,g,b: toggle red,green, blue color map respectively

-,(+ or =): render lower/higher layer respectively (only renders up to generated layers)
