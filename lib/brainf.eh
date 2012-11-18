#!/usr/bin/ehi
include 'library.eh'

class BrainInterpreter
	class BrainException
		this.inherit Exception
	end

	private data
	private dataptr

	private program
	private programLength
	private rip

	private input = File.new "/dev/stdin"

	public initialize = func: program
		this.data = FixedArray.fill(30000, (() => 0))
		this.dataptr = 0
		this.program = program
		this.programLength = program.length()
		this.rip = 0
	end

	public run = func:
		while true
			given this.program->(this.rip)
				case '>'
					this.dataptr += 1
				case '<'
					this.dataptr -= 1
				case '+'
					this.data->(this.dataptr) += 1
				case '-'
					this.data->(this.dataptr) -= 1
				case '.'
					put(this.data->(this.dataptr).toChar())
				case ','
					this.data->(this.dataptr) = input.getc().charAtPosition 0
				case '['
					if this.data->(this.dataptr) == 0
						this.rip = this.findClosingBracket()
					end
				case ']'
					if this.data->(this.dataptr) != 0
						this.rip = this.findOpeningBracket()
					end
				default
					# ignore it
			end
			this.rip += 1
			if this.rip >= this.programLength
				break
			end
		end
	end

	public findOpeningBracket = func:
		private out = this.rip
		private depth = 0
		while out >= 0
			if this.program->out == ']'
				depth += 1
			elsif this.program->out == '['
				depth -= 1
				if depth == 0
					ret out
				end
			end
			out -= 1
		end
		throw(BrainException.new "Could not find matching opening bracket")
	end

	public findClosingBracket = func:
		private out = this.rip
		private depth = 0
		while out < this.programLength
			if this.program->out == '['
				depth += 1
			elsif this.program->out == ']'
				depth -= 1
				if depth == 0
					ret out
				end
			end
			out += 1
		end
		throw(BrainException.new "Could not find matching closing bracket")
	end

	public runWithFile = func: file
		bi = BrainInterpreter.new(File.readFile file)
		bi.run()
	end

	public runWithString = func: str
		bi = BrainInterpreter.new str
		bi.run()
	end
end
