# Gridplot with a figure title and nonconsecutive occupied cells.
set terminal pngcairo size 900,650
set output '/tmp/gridplot-layout.png'
set multiplot layout 2,3 rowsfirst title 'Gridplot: skipped cells'
set title 'First panel'
plot '-' using 1:2 with lines notitle
1 10
2 20
e
reset
# Skip the second cell in the first row.
set multiplot next
set title 'Third panel'
plot '-' using 1:2 with lines notitle
1 30
2 40
e
reset
# Skip the first two cells in the second row.
set multiplot next
set multiplot next
set title 'Sixth panel'
plot '-' using 1:2 with lines notitle
1 50
2 60
e
unset multiplot
