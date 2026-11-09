set term x11 font "Vera,9" size 640,480 noenhanced
set encoding utf8
# user-defined plot
# gpstyle classic
set linetype 1 pt 1 lc rgb "#FF0000"
set linetype 2 pt 2 lc rgb "#0000FF"
set linetype 3 pt 3 lc rgb "#00CC00"
set linetype 4 pt 4 lc rgb "#BF25B2"
set linetype 5 pt 5 lc rgb "#8FAAB3"
set linetype 6 pt 6 lc rgb "#FFA500"
set linetype 7 pt 7 lc rgb "#E51E10"
set linetype 8 pt 8 lc rgb "#000000"
set encoding utf8
set decimalsign '.'
set nokey
set size ratio -1
set palette model RGB
set palette defined (0 1 1 1, 50 0.816 0.910 0.867, 100 0.475 0.745 0.627)
set cbrange [0:100]
set cblabel 'Share of actual class (%)'
set title "Classification accuracy: 65.1%"
set xlabel "Predicted"
set ylabel "Actual"
set xtics ("  1" 0, "  2" 1, "  3" 2)
set ytics ("  1" 2, "  2" 1, "  3" 0)
set xrange [-0.5:2.5]
set yrange [-0.5:2.5]
unset label; set label "128" at 0,0 center front; set label "500" at 1,0 center front; set label "2825" at 2,0 center front; set label "376" at 0,1 center front; set label "610" at 1,1 center front; set label "815" at 2,1 center front; set label "896" at 0,2 center front; set label "376" at 1,2 center front; set label "123" at 2,2 center front;
$colors << EOD
  3.7069 14.4802 81.8129
 20.8773 33.8701 45.2526
 64.2294 26.9534  8.8172

EOD
plot $colors matrix with image
