set title 'Matrix image'
set nokey
$data << EOD
1 2 3
4 5 6
EOD
plot $data matrix with image
