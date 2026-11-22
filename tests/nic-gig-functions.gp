# Representative output from nic_gig 1.0, including both NIC curves.
# Named functions are defined before the plot and remain data-free sources.
set encoding utf8
set linetype 1 lc rgb '#0072B2' lw 2.5
set linetype 2 lc rgb '#D55E00' lw 2.5
set title 'News Impact Curves - GARCH(1,1) vs GJR-GARCH(1,1)'
set xlabel 'Standardised Residual (e_{t-1})'
set ylabel 'Conditional Variance h_t'
set xzeroaxis lt -1 lw 1 lc rgb '#808080'
set yzeroaxis lt -1 lw 1.25 lc rgb '#447799'
set border 15
set grid
set key left top
set xrange [-3:3]
# Force zero into both viewports to verify the reference-axis styling.
set yrange [-0.1:6]
garch_nic(x) = 0.00041496 + 0.111398*x**2
gjr_nic(x) = (x < 0) ? 0.000276527 + 0.562529*x**2 : 0.000276527 + 0.0857941*x**2
plot garch_nic(x) title 'GARCH(1,1)' w lines lt 1, \
     gjr_nic(x) title 'GJR-GARCH(1,1)' w lines lt 2
