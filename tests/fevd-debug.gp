set key outside
set yrange [0:100]
set xrange [0:36]
set xzeroaxis
set title "FEVD for infl"
set style fill solid 0.25
set style histogram rowstacked
set style data histogram
plot 'tests/fevd-debug.txt' using 2 t 'unemp', \
	'' using 3 t 'infl'
