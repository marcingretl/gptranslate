# PanelTools uses a space after the comma in gnuplot font specifications.
# Both single- and double-quoted variants should retain the requested size.
set term pngcairo size 640,480
set output "paneltools-tic-font-spacing.png"
set xtics font ", 8"
set ytics font ', 9'
plot '-' using 1:2 with lines title 'sample'
1 1
2 4
3 9
e
