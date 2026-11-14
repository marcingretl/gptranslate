# Real coefcomp writes a 3-column named block: coefficient, row, half-width.
set output 'coefcomp-xerrorbars.png'
set terminal pngcairo size 800,600
set multiplot layout 2,2
unset key
unset border
set xtics nomirror
set ytics nomirror ("Baseline" 3, "With controls" 2, "Robust" 1)
$CCData << EOD
0.5 3 0.2
1.1 2 0.35
2.0 1 0.1
EOD
set title 'coefcomp named block'
plot $CCData with xerrorbars lc black pt 7
# Explicit asymmetric x bounds may exclude the coefficient estimate.
set title 'asymmetric x bounds'
plot '-' using 1:2:3:4 with xerrorbars notitle
2.0 1 0.4 1.1
3.0 2 1.5 2.3
e
# Arithmetic using expressions produce bounds in x-axis coordinates.
set title 'arithmetic x bounds'
plot '-' using 1:2:($1-$3):($1+$3) with xerrorbars notitle
1.0 1 0.2
1.5 2 0.1
e
# Keep existing vertical error-bar behavior unchanged.
set title 'existing y bounds'
plot '-' using 1:2:3:4 with yerrorbars notitle
0 2.0 0.9 1.9
1 1.5 1.2 1.6
e
unset multiplot
