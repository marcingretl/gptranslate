# roots plot
# libgretl gretl_VAR_roots_plot: the columns are atan2(im,re) and hypot(re,im).
set title 'VAR inverse roots in relation to the unit circle'
unset border
unset key
set xzeroaxis
set yzeroaxis
unset xtics
unset ytics
set size square
set polar
plot 1 notitle w lines, \
'-' using 1:2 w points pt 7
0.78539816 0.70710678 # 0.5000,0.5000
3.14159265 0.70000000 # -0.7000,0.0000
-0.98279372 0.36055513 # 0.2000,-0.3000
0.00000000 1.00000000 # 1.0000,0.0000
e
