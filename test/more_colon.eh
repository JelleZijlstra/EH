#!/usr/bin/ehi
$Integer.operator_colon = func: in {
	if (in.length: == self) {
		echo 'Yes, that is the length of the input'
	} else {
		echo 'No, that is not the length of the input'
	}
}
3: "foo"
2: "bar"
