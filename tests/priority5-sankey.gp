set output 'sankey.png'
unset key
set xrange [0:1]
set yrange [0:1]
unset xtics
unset ytics
set size 0.666,1
set origin 0.167,0
set title "Three-by-three flow diagram"
set object 1 rect from -0.167,0 to 0, 0.351351 fc rgb "0xaaff0000" noclip fillstyle solid 0.5
set label "Row 1" at -0.157, 0.18
set object 2 rect from 1,0 to 1.167, 0.38 fc rgb "0xaaff0000" noclip fillstyle solid 0.5
set label "Col 1" at 1.05, 0.19
bot_1_1(x) = 0 + (1) * x + (2) * x*x + (-3) * x*x*x
top_1_1(x) = 0.25 + (1) * x + (2) * x*x + (-3) * x*x*x
plot '+' using 1:(bot_1_1($1)):(top_1_1($1)) with filledcurves lc rgb "0xbbff0000"
