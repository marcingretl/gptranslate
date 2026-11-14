set output 'pie.png'
reset
b=0.3; B=0.5; h=1.5*b*1.2
unset border; unset tics; unset key
set angles degrees; set yrange [0:1]; set size ratio 1.0; set style fill solid 1.0 border -1
set obj 1 circle arc [0:115.2] fc lt 1
set obj 1 circle at screen B,B size screen b front
set obj 2 circle arc [115.2:360] fc lt 2
set obj 2 circle at screen B,B size screen b front
set angles radians
set label "Alpha" at graph B+cos(0.32*pi)*h, graph B+sin(0.32*pi)*h center front
set label "32.0%" at graph B+cos(0.32*pi)*b, graph B+sin(0.32*pi)*b center front tc rgb "white"
plot 2
