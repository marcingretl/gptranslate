# gretl confidence ellipse plot
set term pngcairo size 800,600
set output 'confidence-ellipse.png'
set title '95% confidence ellipse and 95% marginal intervals'
# literal lines = 9
set parametric
set xzeroaxis
set yzeroaxis
set xlabel 'LRM'
set ylabel 'IBO'
set label '1.5, -0.6' at 1.5,-0.6 point lt 2 pt 1 offset 3,3
x(t) = 0.5*cos(t)-0.12*sin(t)+1.5
y(t) = 0.2*cos(t)+0.35*sin(t)-0.6
plot x(t), y(t) notitle, \
1.2, y(t) notitle w lines lt 2, \
1.8, y(t) notitle w lines lt 2, \
x(t), -0.9 notitle w lines lt 2, \
x(t), -0.3 notitle w lines lt 2
