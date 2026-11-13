# Heatmap correlation labels use sprintf and enhanced-text font size.
set term x11
set link x2
set xrange [-0.5:1.5]
set yrange [-0.5:1.5]
$data << EOD
 0.123 0.456
 -0.789 1.000
EOD
plot $data matrix with image, \
     $data matrix using 1:2:(sprintf("{/=12 \%.1f}",$3)) with labels
