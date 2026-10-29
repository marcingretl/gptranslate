set cbrange [-1:1]
set palette defined (-1 'blue', 0 'white', 1 'red')
$data << EOD
-0.5233 -0.8066 1.0000
0.8685 1.0000 -0.8066
1.0000 0.8685 -0.5233
EOD
plot $data matrix with image, $data matrix using 1:2:(sprintf("%.1f",$3)):($3 < -0.6 ? 0xffffff : 0x000000) with labels tc rgb var
