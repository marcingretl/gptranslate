# Screen margins fix the absolute plot-box rectangle.
set terminal pngcairo size 800,600
set output 'kmeans-margins-screen.png'
set lmargin at screen 0.1
set rmargin at screen 0.9
set bmargin at screen 0.2
set tmargin at screen 0.85
plot '-' using 1:2 with lines notitle
0 0
1 1
e
