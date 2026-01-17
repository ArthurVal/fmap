# 📖 fmap 📖

A simple CLI to **Read/Write a file** content using memory mapping.

> [!IMPORTANT]
> `fmap` uses [`mmap()`](https://fr.wikipedia.org/wiki/Mmap) instead of classic `read()` (like `cat` or `dd`).
> <br>
> It means that the underlying file **MUST BE** 'mappable' (i.e **doesn't work on PIPE/FIFO**).

## 📦 Installation

### From Source

This project use [CMake](https://cmake.org/cmake/help/latest/) as build system.

#### Dependencies

Build:
- [CMake](https://cmake.org/cmake/help/latest/) >= 3.12;

Runtime:
- N/A

#### Compile

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

#### Tests

N/A

TODO: test ...

#### Install

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

## 🚀 Usage

By default, `fmap` only require a file name `FILE` and it will map the
entire file (from the begin to the end).

### Read

When giving any `FILE` (that is 'mappable' using POSIX's `mmap()`), 
**output its content through STDOUT**.

```bash
$ echo "Hello" > test.txt; fmap test.txt
Hello
$ echo "World" > test.txt; fmap test.txt
World
```

### Write

Any data present on **STDIN will be copied** into the file.

```bash
$ echo "Hello World" > test.txt; cat test.txt
Hello World
$ echo -n "World" | fmap test.txt; cat test.txt
World World
$ echo -n "Hello" | fmap test.txt -o 0x6; cat test.txt
World Hello
```

### `OFFSET` and `SIZE`

The memory region mapped can be customized using:
- `-o/--offset N`: Offset to the begin of the span (in bytes, default: 0);
- `-s/--size N`: The size of the span (in bytes, default: -1);

```bash
$ echo "Hello World" > test.txt; fmap test.txt -o 0x6 -s 2
Wo
$ echo -n "Hello" | fmap test.txt -o 0x6; cat test.txt
Hello Hello
```

> [!IMPORTANT]
> Data written into the file are capped by `SIZE` bytes
> (if there are more data in STDIN, they will be **silently ignored**).

```bash
$ echo "Hello World" > test.txt; fmap test.txt
Hello World
$ echo -e "Hello\nThis is ignored ..." | fmap test.txt -o 0x6; cat test.txt
Hello Hello
```

## 📝 License

This project is licensed under the [GNU General Public License v3.0](./LICENSE) License.
