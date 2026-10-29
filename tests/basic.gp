set term pngcairo size 800,600
set output "example.png"
set title "Example from libgretl"
set xlabel "Time"
set ylabel "Value"
set grid
plot '-' using 1:2 with lines title 'series A', '-' using 1:2 with points title 'series B'
1 2
2 3
3 2
4 5
e
1 1
2 2
3 3
4 4
e
