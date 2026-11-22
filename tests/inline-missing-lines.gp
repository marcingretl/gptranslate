# A gretl time-series plot containing a block of missing observations.
# Connected line styles must not bridge the gap between valid samples.
# A named block with textual column headers must not generate an extra NaN row.
# The real SVAR historical-decomposition package exposed this regression.
$named << EOD
year value
0 1
1 2
2 NA
3 ?
4 5
5 6
EOD
set title 'Missing observations must break connected lines'
plot '-' using 1:2 with lines title 'lines', \
     '-' using 1:($2*1) with lines title 'using-expression', \
     '-' using 1:2 with linespoints title 'linespoints', \
     '-' using 1:2 with steps title 'steps', \
     $named using 1:2 with lines title 'named-header'
0 1
1 2
2 NA
3 ?
4 5
5 6
e
0 1
1 2
2 NA
3 ?
4 5
5 6
e
0 1
1 2
2 NA
3 ?
4 5
5 6
e
0 1
1 2
2 NA
3 ?
4 5
5 6
e
