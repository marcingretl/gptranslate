# pcaTools biplot: preserve text color for x and y tick labels.
# Regression for the abbreviated 'tc rgb ...' tic modifier.
set terminal pngcairo size 640,480
set output 'pca-tics.png'
set xrange [-4:2]
set yrange [-2:2]
set xtics tc rgb 'web-blue'
set ytics tc rgb 'web-blue'
plot '-' using 1:2 with points title 'scores'
-2 -1
0 0
1 1
e
