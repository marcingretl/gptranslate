# naiveFC defines a named line style with a color and line type, but no lw.
# Preserve its default width and keep explicit lw styles unchanged.
set output 'naivefc-style-line.png'
set style line 11 lc rgb "#808080" lt 1
set style line 12 lc rgb '#cc0000' lt 0 lw 2
plot '-' using 1:2 with lines ls 11 title 'Default width', \
     '-' using 1:2 with lines ls 12 title 'Explicit width'
0 0
1 1
e
0 1
1 0
e
