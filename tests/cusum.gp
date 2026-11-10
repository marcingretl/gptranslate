# Real libgretl CUSUM diagnostic: two analytic confidence bounds precede inline data.
# set term pngcairo font "Vera,9" size 680,400 noenhanced
set encoding utf8
# CUSUM test
# gpstyle dark2
set linetype 1 pt 6  lc rgb "#1B9E77" # dark teal
set linetype 2 pt 7  lc rgb "#D95F02" # dark orange
set linetype 3 pt 11 lc rgb "#7570B3" # dark lilac
set linetype 4 pt 13 lc rgb "#E7298A" # dark magenta
set linetype 5 pt 4  lc rgb "#66A61E" # dark lime green
set linetype 6 pt 3  lc rgb "#E6AB02" # dark banana
set linetype 7 pt 5  lc rgb "#A6761D" # dark tan
set linetype 8 pt 2  lc rgb "#666666" # dark gray
set border 3 back lc rgb "#808080"
set tics nomirror
set xlabel 'Observation'
set nokey
set xzeroaxis
set title 'CUSUM plot with 95% confidence band'
plot \
6.77007+1.06197*(x-1975) title '' w lines lt 2, \
-6.77007-1.06197*(x-1975) title '' w lines lt 2, \
'-' using 1:2 w linespoints lt 1
1975 0.246397
1975.25 0.691719
1975.5 1.07255
1975.75 1.81791
1976 1.83599
1976.25 2.60571
1976.5 3.67716
1976.75 5.40406
1977 6.13071
1977.25 6.74342
1977.5 6.53975
1977.75 6.9498
1978 6.56103
1978.25 6.14935
1978.5 4.52615
1978.75 3.91348
1979 2.67051
1979.25 1.45942
1979.5 -0.0875324
1979.75 -1.46357
1980 -3.32614
1980.25 -3.78569
1980.5 -3.85515
1980.75 -3.22909
1981 -3.09723
1981.25 -2.47836
1981.5 -1.78738
1981.75 -1.40788
1982 -1.33178
1982.25 -1.42214
1982.5 -2.37713
1982.75 -3.1106
1983 -4.86917
1983.25 -6.28208
1983.5 -6.44843
1983.75 -6.47241
1984 -5.9233
1984.25 -4.99655
1984.5 -3.86352
1984.75 -0.629559
1985 0.178269
1985.25 1.0148
1985.5 1.0514
1985.75 1.21488
1986 1.25216
1986.25 2.05874
1986.5 2.78776
1986.75 4.33003
1987 5.57566
1987.25 6.81488
1987.5 7.90492
e
