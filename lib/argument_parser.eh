# Parser for command-line arguments. Inspired by Python's argparse module.

class ArgumentParser

	# Public interface

	class ParserException
		this.inherit Exception
	end

	# Constructor takes two arguments:
	# - a description of the program
	# - a list (or anything iterable) of hashes representing individual arguments

	# Arguments contain the following hash fields:
	# - name: name of the argument. If this name starts with -, it is considered to be
	#   a labeled argument that may appear at any place. If it does not, it is positional.
	# - nargs: Number of arguments to expect. If this is '+', an arbitrary number of arguments
	#   is consumed.
	# - type: Type that the argument(s) are cast to.
	# - dflt/default: Default value (implies nargs: 1).
	# - action: Callback function to execute after processing the argument.
	# - desc: Description of the argument.
	public initialize(desc, args) = do
		# set raw information
		this.description = desc
		this.raw_arguments = args

		# set processed information
		this.positional_args = Map.new()
		this.n_positional_args = 0
		this.short_args = {}
		this.long_args = {}

		for arg in args
			this.process_input_argument arg
		end

		this.process_input_argument {name: '--help', \
			synonyms: ['-h'], \
			desc: 'Print a help message', \
			dflt: false, \
			action: (ap, value => if value; ap.usage(); end) \
		}
	end

	# Parse an array of arguments
	public parse argv = do
		if argv == null
			argv = global.argv
		end
		this.output = {}
		this.argv_iterator = ArgumentsIterator.new(argv)
		this.program_name = this.argv_iterator.next()
		this.positional_index = 0

		while this.argv_iterator.hasNext()
			private arg = this.argv_iterator.next()
			this.process_argument arg
		end

		# provide defaults
		for _, arginfo in Iterable.chain(this.positional_args, this.short_args, this.long_args)
			private name = arginfo->'canonical'
			if !(this.output.has name)
				if arginfo.has 'default'
					this.finish_processing(arginfo, arginfo->'default')
				elsif arginfo.has 'nargs' and arginfo->'nargs' == '+'
					this.finish_processing(arginfo, [])
				else
					this.print_error("required argument not provided: " + name)
				end
			end
		end

		this.output
	end

	public usage() = do
		echo(this.description)
		# TODO: print more help information
		exit 1
	end

	# Fields
	private description
	private raw_arguments

	private positional_args
	private short_args
	private long_args

	# parser output
	private output
	private argv_iterator
	private program_name
	private positional_index

	# Member classes
	class ArgumentsIterator
		private it

		public initialize array = (this.it = array.getIterator())

		public hasNext() = this.it.hasNext()

		public next() = this.it.next()

		public peek() = this.it.peek()

		public next_is_named_argument() = (!(this.hasNext())) or Type.is_named_argument(this.peek())
	end

	enum Type
		Short, Long, Positional

		public static from_name name = if name->0 == '-'
			if name->1 == '-'
				Type.Long
			else
				Type.Short
			end
		else
			Type.Positional
		end

		public process_name name = match this
			case Short; name.slice(1, max: null)
			case Long; name.slice(2, max: null)
			case Positional; name
		end

		public static is_named_argument arg = (arg.length() > 0 && arg->0 == '-')
	end

	# Private methods
	private print_error error = do
		echo(this.program_name + ": error: " + error)
		this.usage()
	end

	private next_positional() = do
		private out = this.positional_args->(this.positional_index)
		this.positional_index++
		if this.positional_index > this.positional_args.length()
			this.print_error("unrecognized positional argument")
		end
		out
	end

	private get_short_arg name = if this.short_args.has name
		this.short_args->name
	else
		this.print_error("unrecognized short-form argument: " + name)
	end

	private get_long_arg name = if this.long_args.has name
		this.long_args->name
	else
		this.print_error("unrecognized long-form argument: " + name)
	end

	private process_argument arg = do
		private type = Type.from_name arg
		match type
			case Type.Long
				private long_name = type.process_name arg
				private arginfo = this.get_long_arg long_name
				this.process_named_argument arginfo
			case Type.Short
				private short_name = type.process_name arg
				private arginfo = this.get_short_arg short_name
				this.process_named_argument arginfo
			case Type.Positional
				private arginfo = this.next_positional()
				private name = arginfo->'canonical'
				match arginfo->'nargs'
					case 1
						private output = arg
					case '+'
						private output = this.process_infinite_arguments(arg)
					case @nargs
						private output = this.process_finite_arguments(name, first: arg, nargs)
				end
				this.finish_processing(arginfo, output)
		end
	end

	private process_named_argument arginfo = do
		private name = arginfo->'canonical'
		private output
		match arginfo->'nargs'
			case 0
				output = true
			case 1
				if this.argv_iterator.next_is_named_argument()
					this.print_error("not enough values given for argument " + name + " (expected 1)")
				else
					output = this.argv_iterator.next()
				end
			case '+'
				output = this.process_infinite_arguments()
			case @nargs
				output = this.process_finite_arguments(name, nargs)
		end
		this.finish_processing(arginfo, output)
	end

	private process_finite_arguments(name, nargs, first: false) = do
		private output = if first; [first]; else []; end
		private i = if first; 1; else 0; end
		while i < nargs
			if this.argv_iterator.next_is_named_argument()
				this.print_error("not enough values given for argument " + name + " (expected " + nargs + ")")
			end
			output.append(this.argv_iterator.next())
			i++
		end
		output
	end

	private process_infinite_arguments first = do
		private output = if first; [first]; else []; end
		while !(this.argv_iterator.next_is_named_argument())
			output.append(this.argv_iterator.next())
		end
		output
	end

	private finish_processing(arginfo, output) = do
		if arginfo.has 'type'
			private type = arginfo->'type'
			if output.isA type
				# ignore
			elsif output.isA String or output.isA Bool
				output = type output
			else
				output = output.map type
			end
		end
		this.output->(arginfo->'canonical') = output
		if arginfo.has 'action'
			(arginfo->'action')(this, output)
		end
	end

	private process_input_argument argument = do
		if !(argument.has 'name')
			throw(ParserException.new("Argument name must be given: " + argument))
		end

		private full_name = argument->'name'
		private type = Type.from_name full_name
		private name = type.process_name full_name
		argument->'canonical' = name

		if argument.has 'dflt'
			argument->'default' = argument->'dflt'
		end

		# nargs defaults to 1, but 0 if there is a default
		if !(argument.has 'nargs')
			argument->'nargs' = if argument.has 'default'; 0; else 1; end
		end

		if argument.has 'default'
			if type == Type.Positional
				throw(ParserException.new("Cannot use default on positional argument: " + argument))
			elsif argument->'nargs' != 1 and argument->'nargs' != 0
				throw(ParserException.new("Default value can only be given if nargs is 0 or 1: " + argument))
			end
		end

		if argument->'nargs' == 0 && type == Type.Positional
			throw(ParserException.new("Positional argument cannot have nargs of 0: " + argument))
		end

		this.add_argument(type, name, argument)

		# process synonyms
		if argument.has 'synonyms'
			for synonym in argument->'synonyms'
				private syn_type = Type.from_name synonym
				private syn_name = syn_type.process_name synonym
				this.add_argument(syn_type, syn_name, argument)
			end
		end
	end

	private add_argument(type, name, argument) = match type
		case Type.Short
			this.short_args->name = argument
		case Type.Long
			this.long_args->name = argument
		case Type.Positional
			this.positional_args->(this.n_positional_args) = argument
			this.n_positional_args++
	end
end
