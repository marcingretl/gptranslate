# pcaTools creates PCA score plots with RGB loading arrows, data labels and
# independent secondary axes for eigenvector loading scales.
set terminal pngcairo size 640,480
set output 'pca-biplot-styles.png'
set xrange [-4:2]
set yrange [-2:2]
set x2range [-1:1]
set y2range [-1:1]
set arrow from 0,0 to -2.91107,0.308046 lt 1 lw 2 lc rgb 'web-blue'
# Repeating a numbered arrow replaces it, rather than creating a duplicate.
set arrow 7 from 0,0 to -1,1 lt 1 lw 1 lc rgb 'web-blue'
set arrow 7 from 0,0 to -2.71553,-1.06578 lt 1 lw 1 lc rgb 'web-blue'
set arrow 9 from 0,0 to 1,1 lt 1 lw 1 lc rgb 'web-blue'
unset arrow 9
set label 'loading a' at -2.91107,0.308046 tc rgb 'web-blue' font ',12' front
set label 2 'loading b' at -2.71553,-1.06578 tc rgb 'web-blue' font ',10' front
set xtics font ',10'
set ytics font ',10'
set x2tics 0.2 nomirror tc rgb 'web-blue' font ',12'
set y2tics 0.2 nomirror tc rgb 'web-blue' font ',12'
plot '-' using 1:2 with points notitle
-2 -1
0 0
1 1
e
