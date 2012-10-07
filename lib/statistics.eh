#!/usr/bin/ehi

Iterable.numericSum = func: -> (this.reduce 0, (func: x, y -> x + y))

class Statistics
	const public static sd = func: list
		const private n = list.length()
		const private sum = list.numericSum()
		const private mean = sum / n
		const private diffs = list.map (func: x -> (x - mean) * (x - mean))
		((diffs.numericSum()) / (n - 1)).sqrt()
	end

	const public static reverseMean = func: list
		const private n = list.length()
		const private sum = (list.map func: x -> 1.0 / x).numericSum()
		sum / n
	end
	
	const public static mean = func: list
		(list.numericSum()) / (list.length())
	end
	
	const public static median = func: list
		const private length = list.length()
		const private half = length / 2
		const private sorted = list.sort()
		if length % 2 == 0
			mean (list.nth (half - 1), list.nth half)
		else
			list.nth half
		end
	end
end
