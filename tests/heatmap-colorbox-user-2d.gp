# `screen` is the explicit coordinate form for a positioned 2D colorbox.
set output 'heatmap-colorbox-user-2d.png'
set cblabel 'Intensity'
set colorbox user origin screen 0.1,0.6 size screen 0.02,0.3
$values << EOD
1 2 3
4 5 6
7 8 9
EOD
plot $values matrix with image
