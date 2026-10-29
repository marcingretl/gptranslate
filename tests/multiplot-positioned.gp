set size 1.0,1.0
set multiplot
set size 0.8,0.4
set xlabel 'lag'
set yrange [-1:1]
set origin 0.1,0.55
set title 'First panel'
plot '-' using 1:2 with lines notitle
1 0.3
2 0.7
e
set origin 0.1,0.05
set title 'Second panel'
plot '-' using 1:2 with lines notitle
1 -0.3
2 -0.7
e
unset multiplot
