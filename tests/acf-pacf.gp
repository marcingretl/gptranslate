# General regression test for positioned panels, impulse data and constant functions.
set linetype 1 pt 1 lc rgb "#FF0000"
set linetype 2 pt 2 lc rgb "#0000FF"
set size 1.0,1.0
set multiplot
set size 1.0,0.48
set xzeroaxis
set key right top
set xlabel 'lag'
set yrange [-1.00:1.00]
set origin 0.0,0.50
set title 'ACF for LRM'
set xrange [0:5]
plot \
'-' using 1:2 notitle w impulses lw 5, \
0.264286 title '+- 1.96/T^0.5' lt 2, \
-0.264286 notitle lt 2
1 0.942628
2 0.872521
3 0.781169
4 0.682521
e
set origin 0.0,0.0
set title 'PACF for LRM'
set xrange [0:5]
plot \
'-' using 1:2 notitle w impulses lw 5, \
0.264286 title '+- 1.96/T^0.5' lt 2, \
-0.264286 notitle lt 2
1 0.942628
2 -0.143809
3 -0.220001
4 -0.087798
e
unset multiplot
