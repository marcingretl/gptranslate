# Representative Threshold_Panel/plotTpm() script: five numeric dash styles.
set output 'threshold-panel-dash-loop.png'
set key outside below font 'Helvetica,13'
set y2tics
set ytics out
set style line 11 lc rgb "#808080" lt 1
set border 3 back ls 11
set tics nomirror
set style line 12 lc rgb "#808080" lt 0 lw 1
set grid back ls 12
# This bounded loop must not be confused with the pointsize loop.
set for [i=1:5] linetype i dashtype i
set linetype 1 lc rgb "black" lw 2.0
set linetype 2 lc rgb "black" lw 2.0
set linetype 3 lc rgb "black" lw 2.0
set linetype 4 lc rgb "black" lw 2.0
$Data << EOD
1 6 5 4 3 2
2 4 5 3 2 1
3 2 4 2 3 4
EOD
# Gnuplot cycles customized linetypes even without explicit lt modifiers.
plot $Data using 1:2 with lines title 'LR-Ratio', \
     '' using 1:3 with lines title '90pct', \
     '' using 1:4 with lines title '95pct', \
     '' using 1:5 with lines title '99pct', \
     '' using 1:6 with lines title 'Baseline'
