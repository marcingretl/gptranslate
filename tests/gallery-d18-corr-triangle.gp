set title 'Correlation matrix'
set nokey
set tics nomirror
set cbrange [-1:1]
set palette defined (-1 'blue', -0.1755 'white', 0.1755 'white', 1 'red')
set border 3
set x2tics 1 format '' scale 0,0.001
set y2tics 1 format '' scale 0,0.001
set mx2tics 2
set my2tics 2
set ytics ("LRM" 3, "LRY" 2, "IBO" 1, "IDE" 0) out
set xtics ("LRM" 0, "LRY" 1, "IBO" 2, "IDE" 3) out
set xtics rotate by 45 right
set autoscale fix
set link x
set link y
set grid front mx2tics my2tics lw 2 lt -1 lc rgb 'white'
set datafile missing "?"
printcorr = 1
# start inline data
$data << EOD
-0.6592 -0.4720 0.8027 1.0000 
-0.8066 -0.5233 1.0000 NaN 
0.8685 1.0000 NaN NaN 
1.0000 NaN NaN NaN 
EOD
# end inline data
if (printcorr) {
plot $data matrix with image, $data matrix using 1:2:($3!=$3 ? "" : sprintf("%.1f",$3)):($3 < -0.6 ? 0xffffff : 0x000000) with labels tc rgb var
} else {
plot $data matrix with image
}
