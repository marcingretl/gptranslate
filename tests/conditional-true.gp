set title 'Conditional image'
$matrix << EOD
1 2
3 4
EOD
show_labels = 1
if (show_labels) {
plot $matrix matrix with image
} else {
plot $undefined matrix with image
}
