# pcaTools biplot: preserve colored and sized loading labels.
# Regression for 'set label ... at x,y tc rgb ... font ...,12'.
set terminal pngcairo size 640,480
set output 'pca-label.png'
set xrange [-4:2]
set yrange [-2:2]
set label 1 'a' at -2.91107,0.308046 tc rgb 'web-blue' font ',12'
set label 2 'b' at -2.71553,-1.06578 tc rgb 'web-blue' font ',12'
plot '-' using 1:2 with points title 'scores'
-2 -1
0 0
1 1
e
