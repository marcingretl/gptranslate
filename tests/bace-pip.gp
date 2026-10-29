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
set datafile missing "nan"
set x2tics 1 format '' scale 0,0.001
set y2tics 1 format '' scale 0,0.001
set mx2tics 2
set my2tics 2
set palette defined (0 'white', 1 'dark-green')
set title "Variables Inclusions Based on Best 5 Models"
set xlabel "Model number"
set ylabel "Variable"
set xtics ("1" 0, "2" 1, "3" 2, "4" 3, "5" 4)
set ytics ("hscath" 12, "hsrural" 11, "grades" 10, "faminc" 9, "famsiz" 8, "parsome" 7, "parcoll" 6, "female" 5, "asian" 4, "hispan" 3, "black" 2, "grants" 1, "loans" 0)
unset colorbox

set autoscale fix
set link x
set link y
set grid front mx2tics my2tics lw 2 lt -1 lc rgb 'white'
$map << EOD
 1 1 1 1 1
 1 1 1 1 1
 1 1 1 1 1
 1 0 1 0 1
 0 0 1 1 0
 0 0 0 0 1
 1 1 1 1 1
 1 1 1 1 1
 1 1 1 1 1
 1 1 1 1 1
 1 1 1 1 1
 1 1 1 1 1
 1 1 1 1 1

EOD
plot $map matrix with image
