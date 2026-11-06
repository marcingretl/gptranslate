# periodogram
set x2label 'periods'
set x2range [0:96]
set xtics nomirror
set x2tics ("48.0" 4, "9.6" 20, "5.3" 36, "3.7" 52, "2.8" 68, "2.3" 84, "" 96)
set xlabel 'scaled frequency'
set xzeroaxis
set nokey
set title 'Residual spectrum'
set xrange [0:24]
set yrange [0:1.2]
plot '-' using 1:2 w lines
1 0.5
6 0.2
12 0.9
18 0.1
24 0.7
e
