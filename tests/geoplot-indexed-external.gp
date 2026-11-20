# Geoplot also writes its coordinates in an ARG0-relative external file.
set terminal pngcairo size 640,360
set output 'geoplot-indexed.png'
datafile = sprintf("%s.dat", ARG0)
set cbrange [0:3]
set size ratio -1
unset key
plot for [i=0:*] datafile index i with filledcurves fc palette, \
     datafile using 1:2 with lines lc 'white' lw 1
