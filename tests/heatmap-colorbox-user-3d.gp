# Gnuplot 3D pm3d colorbox uses figure-relative origin and size.
set terminal pngcairo size 600,450
set output 'heatmap-colorbox-user-3d.png'
set xlabel 'X'
set ylabel 'Y'
set zlabel 'Height'
set cblabel 'Intensity'
set colorbox user origin 0.1, 0.6 size 0.02, 0.3
set dgrid3d 5,5 qnorm 2
set pm3d at s hidden3d
$data << EOD
0 0 0
2 0 2
0 2 4
2 2 6
1 1 3
EOD
splot $data using 1:2:3 with pm3d palette
