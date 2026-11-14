set output 'kddensity-pm3d.png'
set title 'Kernel density heatmap'
unset surface
unset key
unset ztics
set dgrid3d 21, 21
set view map scale 0.9
set pm3d
set xlabel 'Open'
set ylabel 'Close'
splot '-' with lines
0 0 0.03
0 1 0.11
0 2 0.01
1 0 0.12
1 1 0.37
1 2 0.12
2 0 0.01
2 1 0.13
2 2 0.03
e
