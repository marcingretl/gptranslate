# The contributed roc package emits data and graph-coordinate labels.
# `left front` must not be swallowed by the numeric coordinate parser.
set terminal pngcairo size 640,480
set output 'roc-label-alignment.png'
set xrange [0:1]
set yrange [0:1]
set label "Area = 0.73" at 0.6, 0.35 left front
set label "std. error = 0.05" at 0.6, 0.30 left front
set label "95% C.I. = [0.63, 0.83]" at 0.6, 0.25 left front
# The coordinate mode can be written only before X or before both axes.
set label "Max correct" at graph 0.1, 0.05 left front
set label "Center" at graph 0.4, graph 0.85 center front
set label "Right" at 0.8, 0.8 right front
plot '-' using 1:2 with lines notitle
0 0
0.5 0.7
1 1
e
