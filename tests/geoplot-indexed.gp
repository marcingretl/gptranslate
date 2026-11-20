# libgretl geoplot emits one palette-filled polygon per indexed data group.
set terminal pngcairo size 640,360
set output 'geoplot-indexed.png'
set cbrange [0:3]
set palette defined (0 '#001B63', 1 '#438FC2', 3 '#CBE6FF')
set size ratio -1
unset key
$MapData << EOD
0 0 1
1 0 1
1 1 1
0 1 1

1.1 0 1
1.3 0 1
1.3 0.2 1
1.1 0.2 1


2 0 3
3 0 3
3 1 3
2 1 3
EOD
plot for [i=0:*] $MapData index i with filledcurves fc palette, \
     $MapData using 1:2 with lines lc 'white' lw 1
