# Real 3D pm3d must not be lowered into the existing 2D contour path.
set terminal pngcairo size 620,480
set output 'heatmap-surface-3d.png'
set title "Matrix surface"
set xlabel "x"
set ylabel "y"
set zlabel "z" font ",12" rotate by 90
set zrange [:]
set pm3d at s hidden3d
set hidden3d
# Place 3D tick labels using Gnuplot's default character coordinates.
set tics offset 1.8,0.1
set view 35, 50
set dgrid3d 25,25 qnorm 2
set palette defined ( 0 "blue", 1 "red", 2 "yellow" )
$data << EOD
0 0.5 1
0.5 1 1.5
1 1.5 2
EOD
splot $data matrix with pm3d
