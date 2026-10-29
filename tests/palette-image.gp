set title 'Palette matrix'
set cbrange [-1:1]
set palette defined (-1 'blue', -0.1755 'white', 0.1755 'white', 1 'red')
$data << EOD
-1 0 1
0.5 -0.5 0
EOD
plot $data matrix with image
