# libgretl geomap stores coordinates next to the script as <script>.dat.
set terminal pngcairo size 500,300
set output 'geomap.png'
datafile = sprintf("%s.dat", ARG0)
set size ratio -1
unset key
plot datafile using 1:2 with lines notitle
