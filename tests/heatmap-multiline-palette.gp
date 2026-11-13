set terminal pngcairo size 500,400
set output 'pal.png'
set palette defined ( 0 0.0 0.0 0.5, \
  1 0.0 0.0 1.0, \
  2 0.0 0.5 1.0, \
  3 0.0 1.0 1.0, \
  4 0.5 1.0 0.5, \
  5 1.0 1.0 0.0, \
  6 1.0 0.5 0.0, \
  7 1.0 0.0 0.0, \
  8 0.5 0.0 0.0 )
$data << EOD
0 1 2
3 4 5
6 7 8
EOD
plot $data matrix with image
