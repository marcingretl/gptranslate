# Real random_forest rf_plot constructs, with deterministic sample importances.
set linetype 1 lc rgb "#5F6B84"
set linetype 2 lc rgb "#000000"
set title 'Relative importance (fraction of total)'
unset key
set yrange [0:*]
set xrange [0:0.65]
set style fill solid 0.6
bw = 0.6
set offsets 0, 0, 0.5-bw/2.0, 0.5
$data << _EOD
Feature_A 0.2
Feature_B 0.4
Feature_C 0.6
_EOD
plot $data using (0.5*$2):0:(0.5*$2):(bw/2.):ytic(1) with boxxy
