# CUB combines numbered ellipse functions and inline observation coordinates.
set term pngcairo size 620,480
set output 'cub-parametric-mixed-inline.png'
set parametric
set trange [0:2*pi]
x1(t) = 0.0473731*cos(t) + (-0.00322292*sin(t)) + 0.319576
y1(t) = 0.01*cos(t) + (0.04*sin(t)) + 0.521
x2(t) = -0.2*cos(t) - (-0.1*sin(t)) + 0.1
y2(t) = 0.3*cos(t) - 0.15*sin(t) - 0.4
plot \
    x1(t), y1(t) notitle, \
    x2(t), y2(t) notitle, \
    '-' using 1:2 with points title 'Observations'
0.3195757409 0.8051050332
0.1396309394 0.8346627095
0.2062699488 0.8190590829
0.1601484840 0.9028796510
0.2204504556 0.8514042370
e
