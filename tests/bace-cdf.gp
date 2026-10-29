# gpstyle classic
set linetype 1 pt 1 lc rgb "#FF0000"
set linetype 2 pt 2 lc rgb "#0000FF"
set linetype 3 pt 3 lc rgb "#00CC00"
set linetype 4 pt 4 lc rgb "#BF25B2"
set linetype 5 pt 5 lc rgb "#8FAAB3"
set linetype 6 pt 6 lc rgb "#FFA500"
set linetype 7 pt 7 lc rgb "#E51E10"
set linetype 8 pt 8 lc rgb "#000000"
set xlabel 'col2'
set xzeroaxis
set datafile missing "?"
set ylabel 'col1'
set nokey
set xrange [0.725:12.275]
# start literal lines
set grid
set xtics 1
set title "CDF of Model Size"
set xlabel "Number of Models"
set ylabel "Posterior Probability Mass"
# end literal lines
plot \
 '-' using 1:2 notitle w lines
1 0.4968888889
2 0.8892222222
3 0.9342222222
4 0.9606666667
5 0.9745555556
6 0.9876666667
7 0.9928888889
8 0.9953333333
9 0.9972222222
10 0.9987777778
11 0.9998888889
12 1
e
