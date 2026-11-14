# CUB emits numbered ellipse functions, including a parenthesized signed sine term.
set term pngcairo size 620,480
set output 'cub-parametric-ellipses.png'
set parametric
set trange [0:6.28]
x1(t) = 0.0473731*cos(t) + (-0.00322292*sin(t)) + 0.319576
y1(t) = 0.01*cos(t) + (0.04*sin(t)) + 0.521
x2(t) = -0.2*cos(t) - (-0.1*sin(t)) + 0.1
y2(t) = 0.3*cos(t) - 0.15*sin(t) - 0.4
plot x1(t), y1(t) notitle w lines lt 1, \
     x2(t), y2(t) notitle w lines lt 2
