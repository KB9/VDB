# VDB
A visual debugger (VDB) which uses Qt and the DWARF debugging format to debug ELF executables.

*Disclaimer: This project is a work in progress and, as such, may be subject to performance issues and bugs.*

## Features
* Breakpoints
* Variable watches
* Stack tracing
* Source stepping

# Building

## Dependencies

Dependencies required for building VDB can be installed by executing `dependencies.sh`:
```
./dependencies.sh
```

## Building the Source

The script `build.sh` is responsible for building VDB. It supports the following options:
```
--build-tests     Builds the test suite
--cc=<arg>        Specifies the C compiler to be used when building (default: clang)
--cxx=<arg>       Specifies the C++ compiler to be used when building (default: clang)
```

For example, to build VDB and its tests, the following command should be executed:
```
./build.sh --build-tests
```

# Running

Once built, VDB can be run from the root directory by executing:
```
build/src/ui/VDB
```

# Tests

The [Catch2 test framework](https://github.com/catchorg/Catch2) is integrated with CMake's ctest test driver. To run the tests as a batch, navigate to the `build` directory and execute the following command:
```
make test
```
For more verbose information during test execution, the test programs can be run individually. From the build directory, navigate to `tests` and run the desired executable.
