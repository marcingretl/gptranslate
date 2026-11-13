# Heatmap's correlation example sets an identity link between x and x2.
set link x2
set xrange [0:3]
set x2tics ("A" 0.5, "B" 1.5, "C" 2.5)
plot '-' using 1:2 with lines title 'linked x2'
0 0
1 0.5
2 1
3 0
 e
