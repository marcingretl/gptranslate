# Reproduce gretl --band recession shading used by ARprobit.
# Horizontal coordinates are data years; vertical coordinates are graph fractions.
set term pngcairo size 950,550
set output 'arprobit-recession-bands.png'
set title 'ARprobit recession bands'
set xlabel 'Year'
set ylabel 'Probability'
set xrange [1955:2004]
set yrange [0:1]
set object 1 rectangle from 1957.75, graph 0 to 1958.25, graph 1 back fillstyle solid 0.5 noborder fc rgb "#dddddd"
set object 2 rectangle from 1960.25, graph 0 to 1961.25, graph 1 back fillstyle solid 0.5 noborder fc rgb "#dddddd"
set object 3 rectangle from 1970, graph 0 to 1971, graph 1 back fillstyle solid 0.5 noborder fc rgb "#dddddd"
set object 4 rectangle from 1974, graph 0 to 1975.25, graph 1 back fillstyle solid 0.5 noborder fc rgb "#dddddd"
set object 5 rectangle from 1980, graph 0 to 1980.5, graph 1 back fillstyle solid 0.5 noborder fc rgb "#dddddd"
set object 6 rectangle from 1981.5, graph 0 to 1983, graph 1 back fillstyle solid 0.5 noborder fc rgb "#dddddd"
set object 7 rectangle from 1990.5, graph 0 to 1991.25, graph 1 back fillstyle solid 0.5 noborder fc rgb "#dddddd"
set object 8 rectangle from 2001.25, graph 0 to 2002, graph 1 back fillstyle solid 0.5 noborder fc rgb "#dddddd"
plot '-' using 1:2 with lines lw 2 title 'Fitted probability'
1955 0.18
1960 0.40
1965 0.24
1970 0.63
1975 0.35
1980 0.72
1985 0.22
1990 0.56
1995 0.36
2000 0.77
2004 0.42
e
