set linetype 1 lc rgb "#0000FF"
set linetype 2 lc rgb "#00CC00"
# panel_min 1
set title "Distribution of LF by group"
set nokey
set datafile missing "?"
set xrange [0:7]
set yrange [0,422297:0,686056]
set ylabel "LF"
set xlabel "group"
set xtics ("1" 1, "2" 2, "3" 3, "4" 4, "5" 5, "6" 6) scale 0.0
set xtics nomirror
set boxwidth 0.4 absolute
plot \
'-' using 1:3:2:5:4 w candlesticks lw 2 notitle, \
'-' using 1:2:2:2:2 w candlesticks lt 1 notitle, \
'-' using 1:2:2:2:2 w candlesticks lt 0 notitle, \
'-' using 1:2:2:2:2 w candlesticks lt 0 notitle
1 0,532328 0,547736 0,633366 0,676287 15
2 0,473449 0,512501 0,57207 0,628706 15
3 0,524334 0,56924 0,606592 0,654256 15
4 0,432066 0,488932 0,59457 0,645319 15
5 0,442875 0,529331 0,605868 0,635545 15
6 0,448539 0,500344 0,546723 0,580049 15
e
1 0,597409
2 0,540163
3 0,582119
4 0,55714
5 0,583465
6 0,518296
e
1 0,575417
2 0,526652
3 0,573972
4 0,529925
5 0,556181
6 0,500562
e
1 0,625603
2 0,566782
3 0,589682
4 0,585525
5 0,604723
6 0,537791
e
