# fmap

Utils tool to MAP (using Linux `mmap()`) the memory of a file and either:
- Output its content to STDOUT;
- Update its content from STDIN;

## Build

### Dependencies

#### Build

| **NAME**  | **VERSION**                |
|-----------|----------------------------|
| **libc**  | ?? (C 11)                  |
| **linux** | ??                         |
| **CMake** | ?? (tested on 3.12 -> TBD) |

#### Runtime

| **NAME**  | **VERSION** |
|-----------|-------------|
| **libc**  | ?? (C 11)   |
| **linux** | ??          |

### Compile

```sh
cmake -B build -S . -DCMAKE_BUILD_TYPE=RELEASE && cmake --build build
```

Usefull compile config variable (can be update inside `<BUILD_DIR>/CMakeCache.txt`):

| **NAME**                 | **DEFAULT**  | **DESCRIPTION**                             |
|--------------------------|--------------|---------------------------------------------|
| [`CMAKE_BUILD_TYPE`]     | `RELEASE`    | Compilation optimization used               |
| [`CMAKE_INSTALL_PREFIX`] | `/usr/local` | Install location (follows [GNUInstallDirs]) |

[`CMAKE_BUILD_TYPE`]: https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html
[`CMAKE_INSTALL_PREFIX`]: https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html
[GNUInstallDirs]: https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html#module:GNUInstallDirs

### Tests

N/A

### Install

```sh
cmake --build build --target install
```

Installation location is set by `CMAKE_INSTALL_PREFIX` (default to `/usr/local`)
and `CMAKE_INSTALL_BINDIR` (default to `bin`).

You can update those either by:
- (Recommanded) Updating the `CMakeCache.txt` file inside your build directory;
- Specify `-DCMAKE_INSTALL_PREFIX=<PATH TO INSTALL DIR>` during the
  configuration phase (`cmake -B <BUILD_DIR> -S <SRC_DIR>
  -DCMAKE_INSTALL_PREFIX=<PATH TO INSTALL DIR> ...`);
