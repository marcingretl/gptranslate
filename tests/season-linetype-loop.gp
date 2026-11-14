# libgretl gridplot assigns point size to multiple linetypes in one command.
set output 'season-linetype-loop.png'
set pointsize 1.5
# Size-only updates must preserve colors established for the linetype.
set linetype 1 lc rgb "#00ff00"
set linetype 2 lc rgb "#ff0000"
set for [i=1:40] linetype i ps 1
set for [i=1:40] linetype i ps 2
# A later single-linetype override must take precedence over the loop.
set linetype 2 ps 3
$Data << EOD
1 2
2 3
EOD
plot $Data using 1:2 with points lt 1 notitle, \
     '' using 1:2 with points lt 2 notitle, \
     '' using 1:2 with points lt 3 ps 4 notitle
