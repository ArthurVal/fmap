# fmap

Utils tool to MAP (using Linux `mmap()`) the memory of a file and either:
- Output its content to STDOUT;
- Update its content from STDIN;

## Usage

`fmap` only require a file name `FILE` (file must be 'mappable', i.e. doesn't
work with FIFO) and will, by default, **ouput the file content through STDOUT**
(**READING MODE**).

```sh
$ echo "Hello" > test.txt; fmap test.txt
Hello
$ echo "World" > test.txt; fmap test.txt
World
```

You can specify a start `OFFSET` (`-o OFFSET`, in byte) and a size `SIZE` (`-s
SIZE`, in byte) for the underlying memory span.

```sh
$ echo "Hello World" > test.txt; fmap test.txt -o 0x6 -s 2
Wo
```

> [!IMPORTANT]
> The ouput of `fmap` is **NOT** prepended by a NEWLINE if the mem span doesn't
> end with one.

If not specified or set to `-1`, the `SIZE` will match the file size.

```sh
$ echo "Hello World" > test.txt; fmap test.txt -o 0x6
World
```

When any data is present on **STDIN**, `fmap` will **copy those data** into the
memory span of the file (**WRITING MODE**).

```sh
$ echo "Hello World" > test.txt; cat test.txt
Hello World
$ echo -n "World" | fmap test.txt; cat test.txt
World World
$ echo -n "Hello" | fmap test.txt -o 0x6; cat test.txt
World Hello
```

> [!IMPORTANT]
> In **WRITING MODE**, the data written into the file is capped by `SIZE` (if
> there are more data in STDIN, they will be **silently ignored**).

```sh
$ echo "Hello World" > test.txt; fmap test.txt
Hello World
$ echo -e "Hello\nThis is ignored ..." | fmap test.txt -o 0x6; cat test.txt
Hello Hello
```

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
