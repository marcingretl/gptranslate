set output '/tmp/p5-candle.png'
set format x ("%.0f")
set lt 10 lw 1 lc rgb "black"
set style fill solid border lt 10
set palette defined (-1 'red', 1 'green')
set boxwidth 0.5 relative
set cbrange [-1:1]
plot '-' using 0:1:2:3:4:($4 < $1 ? -1 : 1) notitle with candlesticks lt 10 palette
12 11 15 13
14 13 18 11
e
