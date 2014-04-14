# Main entry point for the EH bytecode compiler.

# This should include as little external code as possible.

include 'listify.eh'
include 'opcode.eh'
include 'simplify.eh'

const public VERBOSE = false

private slice arr start = do
    private out = []
    for i in 1..(arr.length() - 1)
        out.push(arr->i)
    end
    return out
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
    global.argc -= 1
    global.argv = slice argv 1
    EH.executeBytecode bytecode
end

main argc argv
