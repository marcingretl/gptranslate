# Gnuplot tick-label offsets preserve screen-space x/y components.
set terminal pngcairo size 600,400
set output 'tics-offset.png'
set tics offset 1.8,0.1
# The x-axis nooffset setting must not erase the y-axis offset.
set xtics nooffset
set ytics offset -0.5,0.25 font ",12"
plot '-' using 1:2 with lines notitle
0 1
1 2
e
