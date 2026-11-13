# Explicit 1:2:3 columns preserve data coordinates and qnorm interpolation.
set terminal pngcairo size 600,450
set output 'heatmap-surface-xyz.png'
set title 'XYZ pm3d'
set xlabel 'Longitude'
set ylabel 'Latitude'
set zlabel 'Height'
set view 35, 50
set pm3d at s hidden3d
set dgrid3d 5,5 qnorm 2
set palette defined (0 'blue', 6 'red')
$data << EOD
# x y z, with a blank scan separator
0 0 0
2 0 2

0 2 4
2 2 6
1 1 3
EOD
splot $data using 1:2:3 with pm3d palette
