# coefcomp places nomirror before a categorical y-axis tic list.
set output 'coefcomp-named-tics-modifiers.png'
set ytics nomirror ("Baseline" 3, "With controls" 2, "Robust" 1) out
# Modifiers after a tic list and quoted punctuation must work as well.
set xtics ("Zero" 0, "One (sample), more" 1) nomirror
plot '-' using 1:2 with lines notitle
0 1
1 3
e
