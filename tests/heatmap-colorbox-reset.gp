# Unset/default must clear a previously configured user-positioned colorbox.
set output 'heatmap-colorbox-reset.png'
set colorbox user origin 0.1,0.6 size 0.02,0.3
unset colorbox
set colorbox default
$values << EOD
1 2 3
4 5 6
7 8 9
EOD
plot $values matrix with image
