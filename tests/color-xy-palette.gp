# color_xy: palette-colored XY trajectory with its time value in column 1.
unset key
set view map
set size 0.95, 1
set palette negative
set cbrange [2022:2026]
set grid ytics
set grid xtics
set title 'Time-coloured XY trajectory'
set xlabel 'x'
set ylabel 'y'
$DATA << EOD
2022 1.0 0.0
2023 1.5 1.0
2024 1.0 2.0
2025 0.5 3.0
2026 0.0 4.0
EOD
splot $DATA using 3:2:1 with linespoints lw 2 lt palette
