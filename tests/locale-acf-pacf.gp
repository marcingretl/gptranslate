# Regression: gretl emits decimal commas in ranges, constants and inline data.
set size 1.0,1.0
set multiplot
set size 1.0,0.48
set xzeroaxis
set key right top
set xlabel 'lag'
set yrange [-1,00:1,00]
set origin 0.0,0.50
set title 'ACF for LRY'
set xrange [0:5]
plot \
'-' using 1:2 notitle w impulses lw 5, \
0,264286 title '+- 1,96/T^0,5' lt 2, \
-0,264286 notitle lt 2
1 0,918362
2 0,804839
3 0,68111
4 0,540721
e
set origin 0.0,0.0
set title 'PACF for LRY'
set xrange [0:5]
plot \
'-' using 1:2 notitle w impulses lw 5, \
0,264286 title '+- 1,96/T^0,5' lt 2, \
-0,264286 notitle lt 2
1 0,918362
2 -0,246155
3 -0,0945168
4 -0,171808
e
unset multiplot
