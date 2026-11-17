# fsboost emits size-only named styles for coefficient paths.
# Named-style pointsize is independent of the global set pointsize.
set output 'fsboost-style-pointsize.png'
set pointsize 2
set style line 1 ps 0.02
set style line 2 pointsize 0.15
set style line 3 lc rgb '#aa0055' lt 1 lw 2 ps 0.25
plot '-' using 1:2 with linespoints ls 1 title 'first', \
     '-' using 1:2 with linespoints ls 2 title 'second', \
     '-' using 1:2 with linespoints ls 3 title 'third', \
     '-' using 1:2 with linespoints ls 1 ps 0.3 title 'explicit'
0 0
1 1
e
0 1
1 0
e
0 2
1 0
e
0 3
1 1
e
