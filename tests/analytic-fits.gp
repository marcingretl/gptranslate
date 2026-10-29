# Analytic expressions representative of gretl fit types.
set xrange [1:5]
plot 1 + 2*x title "linear" w lines, \
     2 + -3*x + 4*x**2 title "quadratic" w lines, \
     1 + 2*x - 3*x**2 + 4*x**3 title "cubic" w lines, \
     2 + 3/x title "inverse" w lines, \
     exp(1 + 0.2*x) title "semilog" w lines, \
     2 + 3*log(x) title "linlog" w lines
