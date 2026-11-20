# Bounded set loops should pass expanded commands through the normal parser.
set output 'set-for-bounded.png'
# Independent loop variables select numeric style IDs without special cases.
set for [s=1:3] style line s lc rgb "#336699" lw 2
# Step and descending ranges must retain the assignment order.
set for [n=1:5:2] linetype n dt 3
set for [n=5:3:-2] linetype n dt 2
# Explicit values are applied in source order after expanded assignments.
set linetype 3 dt 4
# A quoted loop variable remains literal, whereas a coordinate is replaced.
set for [q=1:2] label 'q' at q, 1
$Data << EOD
1 2 3 4
2 3 4 5
3 4 5 6
EOD
plot $Data using 1:2 with lines ls 1 title 'A', \
     '' using 1:3 with lines ls 2 title 'B', \
     '' using 1:4 with lines ls 3 title 'C', \
     '' using 1:2 with lines lt 1 title 'D', \
     '' using 1:3 with lines lt 3 title 'E', \
     '' using 1:4 with lines lt 5 title 'F'
