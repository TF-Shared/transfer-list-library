# Transfer List Library (LibTL)

The Transfer List Library (LibTL) implements the [Firmware
Handoff](https://github.com/FirmwareHandoff/firmware_handoff) specification,
providing a streamlined interface for managing transfer lists. LibTL offers a
user-friendly interface for:

- Creating transfer lists
- Reading and extracting data from transfer lists
- Manipulating and updating transfer lists

The library supports building with host tools such as Clang and GCC, and cross
compilation with the Aarch64 GNU compiler.

## Building with CMake

To configure the project, use the following command. This will default to using
GCC as the toolchain and create a build directory named `build/`:

```sh
cmake -B build
```

To build the project, use:

```sh
cmake --build build
```

This will output libtl.a in the build directory.

For cross-compilation or selecting a different compiler, specify the target
toolchain file using the `CMAKE_TOOLCHAIN_FILE` option and the target with
`CROSS_COMPILE`:

```sh
export CROSS_COMPILE=aarch64-none-elf-
cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchains/host/gnu.cmake
```

## Testing with CTest

Tests for LibTL are provided in the folder `test`, to configure the project for
test builds, run:

```sh
cmake -B build -DTARGET_GROUP=test
cmake --build build
```

Then, to run the tests with `ctest`, use:

```sh
ctest --test-dir build/
```
