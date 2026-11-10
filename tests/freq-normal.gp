# Reconstructed from libgretl graphing.c plot_freq() (LRM, n=55, 7 bins).
# The histogram represents density, not absolute or relative counts.
set encoding utf8
set linetype 1 lc rgb "#1B9E77"
set linetype 2 lc rgb "#D95F02"
set xrange [11.2:12.4]
set key right top
set xlabel "LRM"
set ylabel "Density"
set style fill solid 0.6
sigma = 0.152357
mu = 11.7544
set label "Test statistic for normality:" at graph .03, graph .97 front
plot \
'-' using 1:2 title "relative frequency" w boxes, \
1.0/(sqrt(2.0*pi)*sigma)*exp(-.5*((x-mu)/sigma)**2) title "N(11.7544, 0.152357)" w lines
11.582 2.6607538802660754
11.664 3.547671840354767
11.746 2.6607538802660754
11.828 0.6651884700665189
11.911 0.6651884700665189
11.993 0.8869179600886919
12.075 1.108647450110865
e
