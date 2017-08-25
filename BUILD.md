# Prerequisites

Install the following packages via the APT package manager:
* gcc >= 5.2 (requires C++14 features)
* libelf-dev
* libunwind-dev
* libdwarf-dev

# Building
## Building the Backend
* Navigate to the project's root directory.
* Execute `make core` to build the backend shared library.
* The backend library file **libvdb.so** can now be located at **bin/core**.

## Building the UI Frontend
* Navigate to **src/ui**.
* Execute `qmake VDB.pro` to generate the frontend's Makefile.
* Execute `make` to build the frontend executable.
* The frontend executable file **VDB** can be located at **bin/ui**.

# Running
* Navigate to **bin/ui**.
* Execute `export LD_LIBRARY_PATH=../core` so the linker can find the backend library.
* Execute `./VDB` to run the application.
