# Hover labels apply to point markers in each panel, not to connecting lines.
set multiplot layout 1,2
set title 'Markers and connecting lines'
plot '-' using 1:2 with linespoints notitle, \
     '-' using 1:2 with lines notitle
0 0
1 1
2 0
e
0 2
1 2
2 2
e
set title 'Standalone points'
plot '-' using 1:2 with points notitle
1 3
2 4
e
unset multiplot
