set title 'Axis ticks'
set x2tics 1 format '' scale 0,0.001
set y2tics 1 format '' scale 0,0.001
set mx2tics 2
set my2tics 2
set ytics ("LRY" 2, "LRM" 1, "IBO" 0) out
set xtics ("LRY" 0, "LRM" 1, "IBO" 2) out
set xtics rotate by 45 right
set autoscale fix
set link x
set link y
$data << EOD
1 2 3
4 5 6
7 8 9
EOD
plot $data matrix with image
