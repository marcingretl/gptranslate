# Minimal reproduction of libgretl's DFM_Plot() -> screeplot output.
# The named styles are intentionally not in the older naiveFC token order.
set term pngcairo size 700,450
set output 'dfm-screeplot.png'
set title 'DFM PCA scree'
set xlabel 'Factor'
set ylabel 'Eigenvalue'
set style line 101 lt 1 lw 1 lc rgb "#343434"
set style line 102 lt 1 dt 2 lw 0.6 lc rgb "#808080"
# The full-screen/graph objects are independent background layers.
set object rectangle from screen 0,0 to screen 1,1 behind fillcolor rgb "#C2C2C2" fillstyle solid noborder
set object rectangle from graph 0,0 to graph 1,1 behind fillcolor rgb "#F5F5F5" fillstyle solid noborder
plot '-' using 1:2 with linespoint ls 101 title 'Eigenvalues', \
     '-' using 1:2 with lines ls 102 title 'Reference'
1 4.2
2 2.1
3 0.7
4 0.3
e
1 1
2 1
3 1
4 1
e
