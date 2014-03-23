frisbee
=======

frisbee is a C++ wrapper library to the Wandbox online compiler(http://melpon.org/wandbox/). It can compile/eval multi language.

requirement
----
- curl
- boost

example
----

```cpp
#include <iostream>
#include "frisbee.h"

using namespace std;
using namespace frisbee;

int main(void)
{
    try {
        // eval    
        cout << eval<string>("C++", "#include <iostream>\n int main() { std::cout << \"C++\" << std::endl; }");
        cout << eval<string>("C#", "class Test { static void Main(string[] args) { System.Console.WriteLine(\"C#\"); } }");
        cout << eval<string>("haskell", "fib = 1:1:zipWith (+) fib (tail fib)\nmain = do print $ take 5 $ fib");
        cout << eval<string>("perl", "print (1..10)");
        cout << eval<int>("ruby", "$,=\" \"; p (1..10).inject(0) {|sum, i| sum + i }");
        
        // compile check
        auto result = compile("C++", "#include <iostream>");
        if (result.is_error())
            cout << result.compiler_error();        
    }
    catch (const exception& e)
    {
        cout << e.what();
    }
}
```
