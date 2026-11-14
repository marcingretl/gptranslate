# Numeric gnuplot margins are given in terminal character units.
set terminal pngcairo size 800,600
set output 'kmeans-margins.png'
set tmargin 1.000000
set bmargin 3
plot '-' using 1:2 with lines notitle
0 0
1 1
e
