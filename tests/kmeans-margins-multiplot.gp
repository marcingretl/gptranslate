# A numeric top margin in each panel belongs to its own layout cell.
set terminal pngcairo size 800,600
set output 'kmeans-margins-multiplot.png'
set multiplot layout 2,1
set tmargin 1.000000
plot '-' using 1:2 with lines notitle
0 0
1 1
e
plot '-' using 1:2 with lines notitle
0 1
1 0
e
unset multiplot
