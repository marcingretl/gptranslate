$matrix << EOD
1 2
3 4
EOD
show_labels = 0
if (show_labels) {
plot $undefined matrix with image
} else {
plot $matrix matrix with image
}
