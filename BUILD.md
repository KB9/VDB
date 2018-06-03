# Prerequisites

Install the following packages via the APT package manager:
* gcc-7 (uses C++17 features)
* libelf-dev
* libunwind-dev
* libdwarf-dev
* cmake
* libqt4-dev

# Building
This project uses a CMake build system. To build, navigate to the project's root directory and execute the following:
```
mkdir build
cd build
cmake ..
make
```

## Including the Test Suite in the Build
When building the project normally, simply add the following CMake cache entry in order to include the tests in the build:
```
cmake -DBUILD_TESTS=ON ..
```

# Running
Navigate to the build directory and enter:
```
./src/ui/VDB
```
## Running the Test Suite
The [Catch2 test framework](https://github.com/catchorg/Catch2) is integrated with CMake's ctest test driver. To run the tests as a batch, navigate to the `build` directory and execute the following command:
```
make test
```
For more verbose information during test execution, the test programs can be run individually. From the build directory, navigate to `tests/core` and run the desired executable.
