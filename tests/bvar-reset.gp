set terminal pngcairo size 800,600
set output '/tmp/bvar-reset.png'
set multiplot layout 1,2
set xrange[0:25.2]
set yrange [-2:2]
set title 'First IRF'
plot '-' using 1:2 with lines title 'response'
0 0
1 1
2 0
e
reset
set xrange[0:25.2]
set title 'Second IRF'
plot '-' using 1:2 with lines title 'response'
0 0
1 -1
2 0
e
unset multiplot
