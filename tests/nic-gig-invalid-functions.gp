# Reject unknown identifiers rather than injecting unvalidated Python text.
set xrange [-3:3]
malicious(x) = open(x)
plot malicious(x) title 'invalid' w lines
