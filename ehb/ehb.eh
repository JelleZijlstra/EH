# Main entry point for the EH bytecode compiler.

# This should include as little external code as possible.

include 'listify.eh'
include 'opcode.eh'
include 'simplify.eh'

const public VERBOSE = true

private reset_argv() = do
    private new_argv = Array()
    for i in 1..(argc - 1)
        new_argv.push(argv->i)
    end
    global.argc -= 1
    global.argv = new_argv
end

public main argc argv = do
    if argc < 2
        echo "ehb: no filename provided"
        exit 1
    end

    private filename = argv->1
    private code = EH.parse(File.readFile filename)
    code = optimize code
    code = listify code
    bytecode = compile code
    if VERBOSE
        echo(disassemble bytecode)
    end

    reset_argv()
    EH.executeBytecode bytecode
end

main argc argv
