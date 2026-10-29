# A named block may be referenced by multiple plot specifications.
$observations << STOP
1 2 4
2 3 5
3 5 7
STOP
plot $observations using 1:2 with lines title 'first', $observations using 1:3 with points title 'second'
