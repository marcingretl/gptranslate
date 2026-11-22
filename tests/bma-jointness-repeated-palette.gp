# BMA jointness maps can have no positive values and missing diagonal cells.
# The repeated zero stop is a valid sharp transition, not an invalid palette.
set title 'BMA jointness with repeated palette stop'
set datafile missing ""
set palette defined (-0.75 'blue', 0 'white', 0 'red')
set xtics ("LRY" 0, "IBO" 1, "IDE" 2)
set ytics ("IDE" 0, "IBO" 1, "LRY" 2)
$map << EOD
NA -0.25 -0.75
-0.25 NA -0.50
-0.75 -0.50 NA
EOD
plot $map matrix with image
