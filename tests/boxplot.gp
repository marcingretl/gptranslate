set linetype 1 lc rgb "#0000FF"
set linetype 2 lc rgb "#00CC00"
set nokey
set datafile missing "?"
set xrange [0:4]
set yrange [-0,38141:12,5546]
set noxtics
set bmargin 3
set label "LRM" at 1,-0,860521 center
set label "LRY" at 2,-0,860521 center
set label "IBO" at 3,-0,860521 center
set boxwidth 0.4 absolute
plot \
'-' using 1:3:2:5:4 w candlesticks lw 2 notitle, \
'-' using 1:2:2:2:2 w candlesticks lt 1 notitle, \
'-' using 1:2 w points lt 2 pt 1 notitle
1 11,5815 11,6289 11,8403 12,0755 55
2 5,78676 5,90366 5,98649 6,09899 55
3 0,0977 0,133481 0,180446 0,210767 55
e
1 11,7049
2 5,9396
3 0,1611
e
1 11,7544
2 5,95239
3 0,156356
e
