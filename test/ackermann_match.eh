ackermann(m, n) = match m, n
	case 0, _; n + 1
	case _, 0; ackermann(m - 1, 1)
	case _, _; ackermann(m - 1, ackermann(m, n - 1))
end

echo(ackermann(1, 1))
