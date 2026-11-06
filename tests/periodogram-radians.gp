# periodogram
set xtics nomirror
set x2label 'periods'
set x2range [0:96]
set x2tics ("48.0" 4, "9.6" 20, "5.3" 36, "3.7" 52, "2.8" 68, "2.3" 84, "" 96)
set xlabel 'radians'
set xzeroaxis
set nokey
set title 'Residual spectrum'
set xrange [0:3.1416]
set xtics ("0" 0, "π/4" pi/4, "π/2" pi/2, "3π/4" 3*pi/4, "π" pi)
plot '-' using 1:2 w lines
0.1309 0.5
0.7854 0.2
1.5708 0.9
2.3562 0.1
3.1416 0.7
e
