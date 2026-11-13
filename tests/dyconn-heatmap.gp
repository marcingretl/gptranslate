# DYconn's heatmap package emits these global tick and colorbox settings.
set output 'dyconn-heatmap.png'
unset colorbox
set tics out
set tics nomirror in font ",12"
set colorbox default
$values << EOD
1 2 3
4 5 6
7 8 9
EOD
plot $values matrix with image
