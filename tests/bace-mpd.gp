# gpstyle classic
set linetype 1 pt 1 lc rgb "#FF0000"
set linetype 2 pt 2 lc rgb "#0000FF"
set linetype 3 pt 3 lc rgb "#00CC00"
set linetype 4 pt 4 lc rgb "#BF25B2"
set linetype 5 pt 5 lc rgb "#8FAAB3"
set linetype 6 pt 6 lc rgb "#FFA500"
set linetype 7 pt 7 lc rgb "#E51E10"
set linetype 8 pt 8 lc rgb "#000000"
set xlabel 'Size'
set xzeroaxis
set datafile missing "?"
set ylabel 'Binomial'
set nokey
set xrange [0:13.325]
set format y "% .13g"
# start literal lines
set link y2 via y inverse y
set xtics 1
set title "Prior Model Probabilities"
set xlabel "Model Size"
set ylabel "Value"
set key left top
# end literal lines
plot \
 '-' using 1:2 notitle w linespoints
0 0.0001220703125
1 0.0001220703125
2 0.0001220703125
3 0.0001220703125
4 0.0001220703125
5 0.0001220703125
6 0.0001220703125
7 0.0001220703125
8 0.0001220703125
9 0.0001220703125
10 0.0001220703125
11 0.0001220703125
12 0.0001220703125
13 0.0001220703125
e
