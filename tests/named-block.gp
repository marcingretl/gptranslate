# A named block can be plotted without consuming inline data.
$data << EOD
1 3
2 5
3 4
EOD
set title 'Named data block'
plot $data using 1:2 with lines title 'example'
