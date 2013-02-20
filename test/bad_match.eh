#!/usr/bin/ehi

match (1, 2, 3, 4)
	case (1, 2, 3, 4, 5)
		echo "too long!"
	case (1, 2, 3)
		echo "too short!"
	case (1, 2, 3, 4)
		echo "just right!"
end
