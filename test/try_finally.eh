try
    echo "in try block"
catch
    echo "in catch block"
end

try
    echo "in try block"
catch
    echo "in catch block"
finally
    echo "in finally block"
end

try
    echo "before throw"
    throw(RuntimeError "hello")
    echo "after throw"
catch if exception.isA TypeError
    echo "in the wrong catch block"
catch if exception.isA RuntimeError
    echo "in catch block"
catch
    echo "in another wrong catch block"
finally
    echo "in finally block"
end

try
    echo "before throw 2"
    throw(RuntimeError "hello 2")
catch if exception.isA TypeError
    echo "in the wrong catch block"
end
