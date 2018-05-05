# Prerequisites

Install the following packages via the APT package manager:
* gcc >= 5.2 (requires C++14 features)
* libelf-dev
* libunwind-dev
* libdwarf-dev
* cmake

# Building
This project uses a CMake build system. To build, navigate to the project's root directory and execute the following:
```
mkdir build
cd build
cmake ..
make
```

# Running
Navigate to the build directory and enter:
```
./src/ui/VDB
```
