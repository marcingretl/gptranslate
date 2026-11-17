# The full boxxyerror form specifies explicit xlow/xhigh and ylow/yhigh.
set style fill solid 0.75
$bounds << EOD
3 8 1 5 6 10
7 4 6 8 3 5
EOD
plot $bounds using 1:2:3:4:5:6 with boxxyerror
