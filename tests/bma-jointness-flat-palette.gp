# BMA may also produce a degenerate all-zero jointness palette.
# This must not create a division by zero in the Matplotlib backend.
set title 'BMA jointness with constant palette'
set datafile missing ""
set palette defined (0 'white', 0 'red')
$map << EOD
NA 0 0
0 NA 0
0 0 NA
EOD
plot $map matrix with image
