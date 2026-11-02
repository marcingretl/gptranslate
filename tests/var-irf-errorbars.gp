# Regression fixture for gretl VAR error-bar confidence intervals.
set terminal pngcairo size 800,600
set multiplot layout 2,2
set xzeroaxis
set xrange [-1:3]
set title 'VAR: asymmetric intervals'
plot \
'-' using 1:2 notitle w lines, \
'-' using 1:2:3:4 notitle w errorbars
0 2.0
1 1.5
2 0.5
e
0 2.0 0.9 1.9
1 1.5 1.2 1.6
2 0.5 0.1 0.6
e
set title 'VAR: symmetric errors'
plot '-' using 1:2:3 notitle w errorbars
0 1.0 0.2
1 1.5 0.1
2 1.2 0.3
e
set title 'VAR: implicit columns'
plot '-' notitle w yerrorbars
0 -1.0 -1.5 -0.8
1 -1.2 -1.4 -0.7
2 -0.8 -0.9 -0.6
e
set title 'VAR: arithmetic using'
plot '-' using 1:2:($2-$3):($2+$3) notitle w errorbars
0 1.0 0.2
1 1.5 0.1
2 1.2 0.3
e
unset multiplot
