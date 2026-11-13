# PairPlot uses numeric offsets, a small tic font, named styles, and point labels.
set output 'pairplot-compat.png'
set offsets 0.02,0.02,0.02,0.02
set xtics font ',6'
set ytics font ',7'
set pointsize 1.1547
set style line 12 lc rgb '#808080' lt 0 lw 1
set label 1 "" at 0.25,0.75 point ps 2 pt 7 lt 1 lc '0xcc0000' lw 2 front
plot '-' using 1:2 with points notitle, '-' using 1:2 with lines ls 12 notitle
0 0
1 1
e
0 1
1 0
e
