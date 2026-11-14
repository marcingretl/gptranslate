# BACE custom-prior histogram: two clustered series from one named block.
set title 'Inclusion Probabilities'
set boxwidth 0.9 absolute
set style fill solid 1.00 border lt -1
set style histogram clustered gap 1 title textcolor lt -1
set style data histograms
set key outside below center
set xtics border in scale 0,0 nomirror rotate by -45 autojustify
set yrange [0:1.0] noreverse writeback
set grid
$map << EOD
1 0.25 0.65
2 0.40 0.45
3 0.35 0.90
EOD
plot $map using 2:xtic(1) title 'Prior', '' u 3 title 'PIP'
