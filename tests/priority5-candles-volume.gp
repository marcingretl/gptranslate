set output '/tmp/p5-candle-volume.png'
set yrange [10:20]
set xrange [-1:3]
set grid
set ytics nomirror
set format y "%.1f"
set format x ("%.0f")
set multiplot layout 2,1
set lmargin at screen 0.08
set size 1, 0.7
set origin 0, 0.3
set bmargin 0
set format x ""
set lt 10 lw 1 lc rgb "black"
set style fill solid border lt 10
set palette defined (-1 'red', 1 'green')
set boxwidth 0.5 relative
set cbrange [-1:1]
plot '-' using 0:1:2:3:4:($4 < $1 ? -1 : 1) notitle with candlesticks lt 10 palette
12 11 15 13
14 13 18 11
e
unset label 1
unset label 2
unset title
set xtics mirror
set ytics nomirror
set bmargin
set format x
set size 1.0, 0.3
set origin 0.0, 0.0
set tmargin 0
unset logscale y
set yrange [0:1000]
set format y "\%.0s \%c"
set ytics 0, 200, 1000
set style line 1 lw 3
set style fill solid border lt 1
unset colorbox
set lt 10 lw 1 lc rgb "black"
set style fill solid border lt 10
set palette defined (-1 'red', 0 'gray', 1 'green')
plot '-' using 0:1:($3 == 0 ? 0 : ($3 < $2 ? 1 : -1)) notitle with boxes palette
500 12 0
1000 14 12
e
unset multiplot
