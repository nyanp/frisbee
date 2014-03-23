frisbee
=======

compile/eval muti language from C++


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
        cout << eval<string>("C++", "#include <iostream>\n int main() { std::cout << \"C++\" << std::endl; }");
        cout << eval<string>("C#", "class Test { static void Main(string[] args) { System.Console.WriteLine(\"C#\"); } }");
        cout << eval<string>("haskell", "fib = 1:1:zipWith (+) fib (tail fib)\nmain = do print $ take 5 $ fib");
        cout << eval<string>("perl", "print (1..10)");
        cout << eval<int>("ruby", "$,=\" \"; p (1..10).inject(0) {|sum, i| sum + i }");
    }
    catch (const exception& e)
    {
        cout << e.what();
    }
}
```
