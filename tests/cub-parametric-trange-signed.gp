# Signed symbolic pi bounds in parametric plots must be evaluated numerically.
set term pngcairo size 400,300
set output 'cub-parametric-trange-signed.png'
set parametric
set trange [-pi:pi]
x(t) = 1*cos(t)+0*sin(t)+0
y(t) = 0*cos(t)+1*sin(t)+0
plot x(t), y(t) notitle w lines
