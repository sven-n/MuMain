# object-pool
C++17 implementation of an object pool pattern

#### Description


 This code is a fork & extension from the following code:

  http://media.wiley.com/product_ancillary/50/11188580/DOWNLOAD/c25_code.zip

 From the original source file:
 
**--- cit ON**

*Provides an object pool that can be used with any class that provides a default constructor.*

*The object pool constructor creates a pool of objects, which it hands out
to clients when requested via the acquireObject() method. acquireObject()
returns an Object which is a std::shared_ptr with a custom deleter that
automatically puts the object back into the object pool when the shared_ptr
is destroyed and its reference reaches 0.*

*The constructor and destructor on each object in the pool will be called only
once each for the lifetime of the program, not once per acquisition and release.*

*The primary use of an object pool is to avoid creating and deleting objects
repeatedly. The object pool is most suited to applications that use large 
numbers of objects with expensive constructors for short periods of time, if
a profiler tells you that allocating and deallocating these objects is a
bottleneck.*

**--- cit OFF**


It is also possible to create an object pool providing a non-default ctor for
the objects that is registered and used at any new allocation when the pool
is empty


Objects returned to the pool are reset by default


See: https://en.wikipedia.org/wiki/Object_pool_pattern


See the unit tests for examples of use.


#### Requirements

`cmake` is used to compile the sources.

The default compiler used is `clang++-9.0.0`.

The cmake files compile with `-std=c++17`.

The unit tests are implemented in `googletest`: be sure you have installed `googletest` to compile.


#### Install

```bash
$ git clone https://github.com/massimo-marino/object-pool.git
$ cd object-pool
$ mkdir build
$ cd build
$ cmake ..
$ make
```

#### Run Unit Tests

Unit tests are implemented with `googletest`.

Install `googletest` to compile and run them.


```bash
$ cd src/unitTests
$ ./unitTests
```


#### Run Example [==Not Yet Implemented==]

Remove `-DDO_LOGS` from `CMAKE_CXX_FLAGS` in the cmake file to see no logs printed at run-time.

```bash
$ cd ../example
$ ./object-pool-example
```
