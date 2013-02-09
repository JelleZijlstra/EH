#!/usr/bin/ehi

FixedArray.inherit class
	private swap = func: i, j
		private tmp = this->j
		this->j = this->i
		this->i = tmp
	end

	public isSorted = func:
		for i in (this.size()) - 1
			if this->i > this->(i + 1)
				ret false
			end
		end
		true
	end

	public bogoSort = func:
		private size = this.size()
		while !(this.isSorted())
			for i in 0..(size - 1)
				private swap_with = Random.rand() % size
				this.swap(i, swap_with)
			end
		end
		this
	end

	public stoogeSortIndexed(min, size) = if size < 3
		if this->min > this->(min + 1)
			this.swap(min, min + 1)
		end
	else
		private new_size = (size * 2.0 / 3).round()
		this.stoogeSortIndexed(min, new_size)
		this.stoogeSortIndexed(min + (size / 3.0).round(), new_size)
		this.stoogeSortIndexed(min, new_size)
	end

	public stoogeSort () = this.stoogeSortIndexed(0, this.size())

	public bubbleSort = func:
		private comparisons
		while true
			comparisons = 0
			for i in this.size() - 1
				if this->i > this->(i + 1)
					this.swap(i, i + 1)
					comparisons++
				end
			end
			if comparisons == 0
				break
			end
		end
		this
	end

	public insertionSort = func:
		this.insertionSortIndexed(0, this.size())
	end

	public insertionSortIndexed = func: min, size
		for i in 1..(size - 1)
			private to_insert = this->i
			private j = i
			while j > 0 && this->(j - 1) > to_insert
				this->j = this->(j - 1)
				j--
			end
			this->j = to_insert
		end
		this
	end

	public selectionSort = func:
		const size = this.size()
		for i in size - 1
			private smallest = i
			private smallest_value = this->i
			for j in (i + 1)..(size - 1)
				if this->j < smallest_value
					smallest = j
					smallest_value = this->j
				end
			end
			this.swap(i, smallest)
		end
		this
	end

	public combSort = func:
		private const size = this.size()
		private gap = size
		while true
			gap /= 1.247330950103979
			if gap < 1
				gap = 1
			end

			private comparisons = false
			private integerGap = gap.toInteger()
			for i in size - integerGap
				if this->i > this->(i + integerGap)
					this.swap(i, i + integerGap)
					comparisons = true
				end
			end

			if !comparisons && integerGap == 1
				break
			end
		end
		this
	end

	public mergeSort = func:
		this.mergeSortIndexed(0, this.size())
	end

	private mergeSortIndexed = func: min, size
		if size < 2
			ret
		end
		const half = size / 2
		this.mergeSortIndexed(min, half)
		this.mergeSortIndexed(min + half, size - half)

		private merged = FixedArray.new size
		private first_index = 0
		private second_index = half
		for i in size
			if first_index == half
				merged->i = this->(min + second_index)
				second_index++
			elsif (second_index == size) || (this->(min + first_index) < this->(min + second_index))
				merged->i = this->(min + first_index)
				first_index++
			else
				merged->i = this->(min + second_index)
				second_index++
			end
		end
		for i in size
			this->(min + i) = merged->i
		end
		this
	end

	public quickSort = func:
		this.quickSortIndexed(0, this.size())
	end

	private quickSortIndexed = func: min, size
		if size < 2
			ret
		end

		private half = size / 2 + min
		private pivot = this->half

		private storageIndex = min
		this.swap(half, min + size - 1)
		for i in min..(min + size - 2)
			if this->i < pivot
				this.swap(i, storageIndex)
				storageIndex++
			end
		end

		this.swap(storageIndex, min + size - 1)

		this.quickSortIndexed(min, storageIndex - min)
		this.quickSortIndexed(storageIndex + 1, min + size - storageIndex - 1)
		this
	end

	public bucketSort = func:
		const private size = this.size()
		const private arraySize = size * 2
		private sorter = FixedArray.new (arraySize + 1)
		for i in arraySize + 1
			sorter->i = Nil
		end

		private nodeIndex = 0
		private const conversionFactor = Random.max.toFloat() / arraySize + 1
		for i in size
			private n = this->i
			private index = (n / conversionFactor).toInteger()
			sorter->index = n::(sorter->index)
		end

		private index = 0
		for i in arraySize + 1
			for n in sorter->i.sort()
				this->index = n
				index++
			end
		end
		this
	end
end
