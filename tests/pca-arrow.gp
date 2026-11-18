# pcaTools biplot: preserve loading arrows drawn in first-axis coordinates.
# Regression for 'set arrow from ... to ... lt ... lw ... lc rgb ...'.
set terminal pngcairo size 640,480
set output 'pca-arrow.png'
set xrange [-4:2]
set yrange [-2:2]
set arrow from 0,0 to -2.91107,0.308046 lt 1 lw 1 lc rgb 'web-blue'
set arrow from 0,0 to -2.71553,-1.06578 lt 1 lw 1 lc rgb 'web-blue'
plot '-' using 1:2 with points title 'scores'
-2 -1
0 0
1 1
e
