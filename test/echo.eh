f x = x * x
x = 42

echo x
echo (f 42)

nested() = echo x
echo nested
nested()

enum Listx
    Nil, Cons(head, tail)

    echo Nil

    public is_empty() = this == Nil
end

echo(Listx.Nil)
echo(Listx.Nil.is_empty())

public print lst = match lst
    case Listx.Cons(@head, Listx.Nil)
        echo("List of " + String head)
    case Listx.Cons(@head, Listx.Cons(@head2, Listx.Nil))
        echo("List of " + String head + " and " + String head2)
    case Listx.Nil
        echo("Empty list")
end

echo (3, 1)

my_list = Listx.Cons(3, Listx.Nil)
echo my_list

print my_list

try
    echo "in try block"
finally
    echo "in finally block"
end

Listx.hello = 42
echo(Listx.hello)
private Listx.goodbye = 43
try
    echo(Listx.goodbye)
catch if exception.isA VisibilityError
    echo "caught VisibilityError"
end
