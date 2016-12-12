Fractal Terrain

to compile the computation part, type make

for help with args type ./compute.o (with no args)


to render, be on a linux machine with OpenGL (not linab or hpc)
compile the render part with make render
type ./render or ./render [some other directory] to render the generated data

render controls;
w,a,s,d: move terrain
[spacebar],c: move terrain up and down
i,j,k,l: rotate terrain
r,g,b:toggle red,green, blue color map respectively
-,(+ or =): render lower/higher layer respectively (only renders up to generated layers)
