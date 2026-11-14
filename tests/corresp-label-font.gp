# Real correspondence-analysis labels: note the missing space after font.
set output 'corresp-label-font.png'
set size ratio 0.692225
set label "Row 1" at 0.2, 0.342722 font",10"center tc rgb "#FF0000"
set label "Column 1" at 0.4, -0.85727 font ",12" center tc rgb "#0000FF"
set label "Supplement" at -0.3, 0.15center tc rgb "#0000FF"
plot '-' using 1:2 with dots notitle
0 0
e
