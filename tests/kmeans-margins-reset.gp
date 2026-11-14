# Negative and unset margins restore automatic layout.
set terminal pngcairo size 800,600
set output 'kmeans-margins-reset.png'
set tmargin 1.0
set tmargin -1
set bmargin at screen 0.2
unset bmargin
set lmargin 4
set lmargin
plot '-' using 1:2 with lines notitle
0 0
1 1
e
