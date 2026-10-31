# Syllabus map

Every syllabus topic and where it appears. `engine` is the program in `src/`, `demo` is the concepts demo in `src/demo/`, and `concept` is a topic that is explained in the viva and has no single line of code.


## Unit 1

| Topic | Where | File and line | Note |
|---|---|---|---|
| Need of object-oriented programming | concept | - | Explained in the viva |
| Objects | engine | `src/advisor.cpp:67` | A real object created and used |
| Classes | engine | `src/backtester.hpp:43` | Data and behaviour together |
| Data members | engine | `src/backtester.hpp:52` | Each Backtester keeps its own cost |
| Methods | engine | `src/backtester.hpp:48` | Backtester's main member function |
| Namespaces | engine | `src/indicators.hpp:15` | Nested inside namespace quant |
| Data encapsulation | engine | `src/backtester.cpp:94` | The account stays inside run |
| Data abstraction | engine | `src/strategy.hpp:17` | Backtester sees only the interface |
| Information hiding | engine | `src/backtester.hpp:23` | Private fields, public getters |
| Inheritance | engine | `src/strategy.hpp:38` | A strategy is a kind of Strategy |
| Polymorphism | engine | `src/backtester.cpp:110` | One call reaches four strategies |
| Static binding | demo | `src/demo/unit3_inheritance.cpp:35` | Non-virtual, the pointer's type decides |
| Dynamic binding | demo | `src/demo/unit3_inheritance.cpp:36` | Virtual, the real object decides |
| Message passing | engine | `src/advisor.cpp:72` | Advisor calls Backtester, which calls Strategy |
| Benefits of OOP | concept | - | Explained in the viva |
| C++ as an OOP language | concept | - | Explained in the viva |
| C++ basics | engine | `src/indicators.hpp:19` | Functions, loops, arithmetic |
| Data types | engine | `src/candle.hpp:31` | long long, double, int, bool, char |
| Structure | engine | `src/candle.hpp:30` | One hourly bar |
| Enumeration | engine | `src/candle.hpp:14` | Also enum class Verdict |
| Control structures | engine | `src/strategies.cpp:58` | if, else, for, while, switch, try |
| Arrays | engine | `src/strategy.hpp:75` | The last 5 returns |
| Strings | engine | `src/datastore.cpp:28` | std::string joining |
| Access specifiers | engine | `src/strategy.hpp:33` | public, protected, private |

## Unit 2

| Topic | Where | File and line | Note |
|---|---|---|---|
| Introducing functions | engine | `src/advisor.cpp:52` | A small pure function |
| Constructor | engine | `src/candle.hpp:39` | Sets every field to 0 |
| Destructor | engine | `src/fetcher.hpp:28` | Releases the curl handle |
| Objects and memory requirements | demo | `src/demo/unit2_objects.cpp:129` | sizeof of objects |
| Static member variables | demo | `src/demo/unit2_objects.cpp:19` | One count shared by all objects |
| Static member functions | demo | `src/demo/unit2_objects.cpp:16` | Called without an object |
| Inline function | engine | `src/candle.hpp:25` | A one-liner expanded at the call |
| Friend function | engine | `src/candle.hpp:48` | Friend stream operators |
| Characteristics of constructors | concept | - | Explained in the viva |
| Types of constructors | engine | `src/candle.hpp:41` | Default, parameterised; copy in the demo |
| Constructor overloading | engine | `src/candle.cpp:39` | Candle() and Candle(time, ...) |
| Copy constructor | demo | `src/demo/unit2_objects.cpp:44` | A deep copy with its own array |
| Dynamic initialization of an object | demo | `src/demo/unit2_objects.cpp:97` | Size known only at run time |
| Constructor with default arguments | engine | `src/backtester.hpp:46` | Defaults for cash, cost and bars |
| Declaring and initializing pointers | demo | `src/demo/unit2_pointers.cpp:14` | Holds the address of price |
| Indirection operator | demo | `src/demo/unit2_pointers.cpp:15` | Writes through the pointer |
| Memory management: new and delete | engine | `src/strategy.cpp:28` | Deleted by the caller |
| Pointers to objects | engine | `src/advisor.cpp:73` | A pointer to a strategy object |
| this pointer | demo | `src/demo/unit2_objects.cpp:53` | Self-assignment check |
| Arrays of pointers | demo | `src/demo/unit2_pointers.cpp:28` | Three different strategies in one list |
| Function pointers | demo | `src/demo/unit2_pointers.cpp:40` | The caller chooses the ranking |
| Pointers to pointers | demo | `src/demo/unit2_pointers.cpp:20` | Strategy** walks the list |
| Pointer to derived classes | engine | `src/strategy.cpp:25` | A child returned as a Strategy* |
| Passing pointers to functions | demo | `src/demo/unit2_pointers.cpp:32` | The list is passed by pointer |

## Unit 3

| Topic | Where | File and line | Note |
|---|---|---|---|
| Base class and derived class | engine | `src/strategy.hpp:45` | Strategy is the base |
| Protected members | engine | `src/strategy.hpp:34` | Children change it directly |
| Overriding member function | engine | `src/strategy.hpp:86` | KnnStrategy also clears its memory |
| Class hierarchies | engine | `src/strategy.hpp:38` | Strategy and its four children, also errors.hpp |
| Public inheritance | engine | `src/errors.hpp:13` | Everything public stays public |
| Private inheritance | demo | `src/demo/unit3_inheritance.cpp:73` | Hides the vector's interface |
| Single inheritance | demo | `src/demo/unit3_inheritance.cpp:19` | One base class |
| Multilevel inheritance | demo | `src/demo/unit3_inheritance.cpp:26` | Venue, Exchange, SpotExchange |
| Hierarchical inheritance | engine | `src/strategy.hpp:37` | Four children of Strategy |
| Multiple inheritance | demo | `src/demo/unit3_inheritance.cpp:64` | Two parents |
| Hybrid inheritance | demo | `src/demo/unit3_inheritance.cpp:63` | Multiple on top of hierarchical |
| Ambiguity in multiple inheritance | demo | `src/demo/unit3_inheritance.cpp:69` | Names each parent |
| Virtual base class | demo | `src/demo/unit3_inheritance.cpp:53` | One shared Instrument |
| Abstract class | engine | `src/strategy.hpp:16` | Strategy cannot be created |
| Friend class | demo | `src/demo/unit3_inheritance.cpp:86` | Auditor may read the balance |
| Introduction to polymorphism | concept | - | Explained in the viva |
| Types of polymorphism | concept | - | Compile-time and run-time |
| Operator overloading concept | engine | `src/candle.cpp:62` | An operator as a function |
| Overloading unary operators | demo | `src/demo/unit3_inheritance.cpp:99` | Also prefix ++ |
| Overloading binary operators | demo | `src/demo/unit3_inheritance.cpp:106` | Adds two amounts |
| Function overloading | demo | `src/demo/unit3_inheritance.cpp:110` | Picked by the argument type |
| Runtime polymorphism: pointer to base class | engine | `src/advisor.cpp:20` | Base pointer to any child |
| Virtual function and its significance | engine | `src/strategy.hpp:27` | Backtester stays unchanged |
| Pure virtual function | engine | `src/strategy.hpp:23` | Every child must define it |
| Virtual destructor | engine | `src/strategy.hpp:21` | delete through Strategy* is safe |

## Unit 4

| Topic | Where | File and line | Note |
|---|---|---|---|
| Exception handling fundamentals | engine | `src/main.cpp:83` | try and catch around the program |
| Other error handling techniques | engine | `src/commands.cpp:26` | askNumber returns -1 for bad input |
| Simple exception handling: divide by zero | demo | `src/demo/unit4_exceptions.cpp:13` | safeDivide |
| Multiple catching | engine | `src/main.cpp:81` | Also in the demo |
| Rethrowing an exception | engine | `src/fetcher.cpp:91` | Also in the demo |
| Exception specifications | demo | `src/demo/unit4_exceptions.cpp:18` | Also ~DataFetcher |
| User-defined exceptions | engine | `src/errors.hpp:12` | Also StalePriceError in the demo |
| Processing unexpected exceptions | engine | `src/main.cpp:92` | Also in the demo |
| Exceptions in constructors and destructors | demo | `src/demo/unit4_exceptions.cpp:75` | Built members are still destroyed |
| Exceptions and inheritance | demo | `src/demo/unit4_exceptions.cpp:43` | QuantError& catches a child |
| The power of templates | concept | - | Explained in the viva |
| Function template | demo | `src/demo/unit4_templates.cpp:12` | Works for int and double |
| Overloading function templates | demo | `src/demo/unit4_templates.cpp:20` | Pointer and vector versions |
| Class template | demo | `src/demo/unit4_templates.cpp:44` | A generic fixed window |
| Non-type parameters | demo | `src/demo/unit4_templates.cpp:42` | N is the capacity |
| Template and friend generic functions | demo | `src/demo/unit4_templates.cpp:61` | Reads the private array |
| typename keyword | demo | `src/demo/unit4_templates.cpp:37` | Required for a dependent type |
| export keyword | concept | - | Removed from C++ |

## Unit 5

| Topic | Where | File and line | Note |
|---|---|---|---|
| Data hierarchy | concept | - | Explained in the viva |
| Streams and files | engine | `src/datastore.cpp:56` | The CSV cache |
| Stream classes | engine | `src/datastore.cpp:39` | ifstream, ofstream, stringstreams |
| Stream errors | engine | `src/datastore.cpp:20` | Also in the demo |
| Disk file I/O with streams | engine | `src/datastore.cpp:64` | Reads the whole CSV |
| File pointers | demo | `src/demo/unit5_files.cpp:36` | seekg and tellg |
| Error handling in file I/O | engine | `src/datastore.cpp:68` | bad() and eof() after reading |
| File I/O with member functions | engine | `src/datastore.cpp:40` | open and close |
| Overloading extraction and insertion operators | engine | `src/candle.cpp:71` | Candle reads and writes CSV |
| Memory as a stream object | engine | `src/candle.cpp:81` | Also ostringstream for the report |
| Command-line arguments | engine | `src/main.cpp:80` | argc and argv |
| Printer output | engine | `src/report.cpp:36` | setw and setprecision columns |
| File processing | engine | `src/report.cpp:87` | Saves the report file |
| Primitive data processing | engine | `src/datastore.cpp:17` | Text CSV of numbers |
| Object data processing | demo | `src/demo/unit5_files.cpp:86` | A whole record in binary |
