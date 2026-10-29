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
set key left top
set xrange [0:13.325]
# start literal lines
set link y2 via y inverse y
set xtics 1
set title "Model Size Distribution"
set xlabel "Model Size"
set ylabel "Density"
set key left top
# end literal lines
plot \
 '-' using 1:2 title "Prior" w linespoints, \
 '-' using 1:2 title "Posterior" w linespoints
0 0.0001220703125
1 0.001586914063
2 0.009521484375
3 0.03491210938
4 0.08728027344
5 0.1571044922
6 0.2094726563
7 0.2094726563
8 0.1571044922
9 0.08728027344
10 0.03491210938
11 0.009521484375
12 0.001586914063
13 0.0001220703125
e
0 0
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0.003555555556
10 0.3943333333
11 0.5364444444
12 0.06044444444
13 0.005222222222
e
