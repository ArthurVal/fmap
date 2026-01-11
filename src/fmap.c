/* libc  */
#include <assert.h>
#include <errno.h> /* errno */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> /* EXIT_SUCCESS */
#include <string.h> /* strerror */

/* POSIX */
#include <fcntl.h>  /* open */
#include <getopt.h> /* getopt_long */
#include <unistd.h> /* close */

/* Linux */
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>

/* internal */
#include "config.h"
#include "logging.h"

struct Args {
  const char *file;
  off_t offset;
  ssize_t size;
};

static bool Args_FromArgv(int argc, char *argv[], struct Args *d_args);
static bool Args_UpdateFromFd(struct Args *args, int fd);

static bool FD_Read(int fd, uint8_t *buffer, size_t size, size_t *d_written);
static bool FD_Write(int fd, uint8_t const *buffer, size_t size);

static int STDIN_BytesAvailables(void);

int main(int argc, char *argv[]) {
  int ret = EXIT_SUCCESS;

  struct Args args = {
      .file = NULL,
      .offset = 0x0,
      .size = -1,
  };

  if (!Args_FromArgv(argc, argv, &args)) {
    ret = EXIT_FAILURE;
    goto end;
  }

  DEBUG(
      "Args:"
      "\n - FILE: %s"
      "\n - -o  : 0x%zX"
      "\n - -s  : %li",
      args.file, args.offset, args.size);

  int stdin_bytes_count = STDIN_BytesAvailables();
  if (stdin_bytes_count < 0) {
    WARN("Couldn't retreived the STDIN byte count: %s", strerror(errno));
    WARN("-> Default to READING");
    stdin_bytes_count = 0;
  } else {
    DEBUG("STDIN: %d bytes", stdin_bytes_count);
    INFO("MODE: %c", stdin_bytes_count > 0 ? 'W' : 'R');
  }

  DEBUG("Opening '%s': ...", args.file);
  int fd = open(args.file, O_RDWR | O_SYNC);
  if (fd == -1) {
    ERROR("Opening '%s': FAILED: %s", args.file, strerror(errno));
    ret = EXIT_FAILURE;
    goto end;
  } else {
    INFO("Opening '%s': DONE", args.file);
  }

  if (!Args_UpdateFromFd(&args, fd)) {
    ret = EXIT_FAILURE;
    goto end_after_open;
  }

  DEBUG("Mapping %zu bytes @ 0x%zX: ...", args.size, args.offset);

  /* The offset MUST BE aligned to a PAGESIZE (required by mmap) */
  off_t alignment = args.offset - (args.offset & ~(sysconf(_SC_PAGE_SIZE) - 1));
  DEBUG("Align by %li bytes (PAGE: %li)", alignment, sysconf(_SC_PAGE_SIZE));

  uint8_t *mem =
      mmap(NULL, (size_t)(args.size + alignment), PROT_READ | PROT_WRITE,
           MAP_SHARED, fd, (args.offset - alignment));

  if (mem == MAP_FAILED) {
    ERROR("Mapping %li bytes @ 0x%zX: FAILED: %s", args.size, args.offset,
          strerror(errno));

    ret = EXIT_FAILURE;
    goto end_after_open;
  } else {
    INFO("Mapping %li bytes @ 0x%zX: DONE", args.size, args.offset);
    mem += alignment; /* mem needs to be 'un-aligned' */
  }

  if (stdin_bytes_count > 0) {
    size_t written = 0;
    DEBUG("Writing: ...");
    if (!FD_Read(fileno(stdin), mem, (size_t)args.size, &written)) {
      ERROR("Writing: FAILED: %s", strerror(errno));
      ret = EXIT_FAILURE;
      goto end_after_mmap;
    } else {
      INFO("Writing: DONE");
      DEBUG("Wrote: %zu bytes", written);
    }
  } else {
    DEBUG("Reading: ...");
    if (!FD_Write(fileno(stdout), mem, (size_t)args.size)) {
      ERROR("Reading: FAILED: %s", strerror(errno));
      ret = EXIT_FAILURE;
      goto end_after_mmap;
    } else {
      INFO("Reading: DONE");
    }
  }

end_after_mmap:
  DEBUG("Unmapping of memory: ...");
  mem -= alignment; /* Re-aligned the mem to mmap */
  if (munmap(mem, (size_t)(args.size + alignment)) == -1) {
    WARN("Unmapping of memory: FAILED: %s", strerror(errno));
  } else {
    INFO("Unmapping of memory: DONE");
  }

end_after_open:
  DEBUG("Closing file: ...");
  if (close(fd) == -1) {
    WARN("Closing file: FAILED: %s", strerror(errno));
  } else {
    INFO("Closing file: DONE");
  }

end:
  return ret;
}

static void Usage(const char *name) {
  assert(name != NULL);
  fprintf(
      stderr,
      "Usage: %s"
      "\n       FILE"
      "\n       [-o OFFSET] [-s SIZE] [ACTION]"
      "\n       [-h] [--version] [-v VERBOSE]"
      "\n"
      "\nMap FILE's memory (memory starting at OFFSET, of SIZE bytes) and"
      "\nOUTPUT its content to STDOUT."
      "\nIf STDIN got ANY data in it, copy STDIN into the mapped memory."
      "\n"
      "\nEffectively does the same as 'cat' but use 'mmap' instead, which can"
      "\nbe used to interact with specific devices (like /dev/mem for example)."
      "\n"
      "\nPositional arguments (mandatory):"
      "\n FILE       Name of the file we wish to map"
      "\n"
      "\nOptions:"
      "\n -h, --help"
      "\n            Show this help message and exit"
      "\n --version"
      "\n            Print the current script version"
      "\n -o OFFSET, --offset OFFSET"
      "\n            Begin of the address range within the file"
      "\n            Must be >= 0"
      "\n            (default: 0x0 -> Begin of the file)"
      "\n -s SIZE, --size SIZE"
      "\n            Size of the mapping in BYTES"
      "\n            Clamped to the file size"
      "\n            (default: -1 -> Whole file)"
      "\n"
      "\nLogging:"
      "\n All logs go through STDERR."
      "\n"
      "\n -v [DEBUG, INFO, WARN, ERROR], --verbose [DEBUG, INFO, WARN, ERROR]"
      "\n            Verbose log level"
      "\n            (default: WARN)"
      "\n",
      name);
}

static bool StrToInt_i64(const char *restrict str, int base, int64_t *d_int,
                         char **restrict d_remaining) {
  assert(d_int != NULL);
  errno = 0;

  bool success = true;
  int64_t value = strtol(str, d_remaining, base);
  if (errno == ERANGE) {
    success = false;
  } else {
    *d_int = value;
  }

  return success;
}

static bool Args_FromArgv(int argc, char *argv[], struct Args *d_args) {
  assert(argv != NULL);
  assert(d_args != NULL);

  enum {
    ARG_END = -1,
    ARG_UNKNOWN = '?',

    /* Short options */
    ARG_OFFSET = 'o',
    ARG_SIZE = 's',

    ARG_HELP = 'h',
    ARG_VERBOSE = 'v',

    /* Long options only */
    _ARG_LONG_OPT_BEGIN = (1 << 8),
    ARG_VERSION,
    ARG_RD_RAW,
    ARG_RD_HEX,
  } arg_id;

  struct option const my_options[] = {
      {"offset", required_argument, NULL, ARG_OFFSET},
      {"size", required_argument, NULL, ARG_SIZE},

      {"help", no_argument, NULL, ARG_HELP},
      {"version", no_argument, NULL, ARG_VERSION},

      {"verbose", required_argument, NULL, ARG_VERBOSE},

      /* {"rd-raw", no_argument, NULL, ARG_RD_RAW}, */
      /* {"rd-hex", no_argument, NULL, ARG_RD_HEX}, */
      /* END */
      {NULL, 0, NULL, 0},
  };

  opterr = 0; /* Disable auto error logs from getopt */
  while ((arg_id = getopt_long(argc, argv, "ho:s:v:", my_options, NULL)) !=
         ARG_END) {
    switch (arg_id) {
      case ARG_HELP:
        Usage(argv[0]);
        return false;

      case ARG_VERSION:
        fputs(FMAP_VERSION_STR, stderr);
        return false;

      case ARG_VERBOSE: {
        LogLevel lvl;
        if (!LogLevel_FromString(optarg, &(lvl))) {
          ERROR("'-%c %s': Unknown log level '%s'", arg_id, optarg, optarg);
          return false;
        } else {
          Logging_Level(&lvl);
        }
      } break;

      case ARG_OFFSET: {
        char *suffix;
        if (!StrToInt_i64(optarg, 0, &(d_args->offset), &suffix)) {
          ERROR("'-%c %s': %s", arg_id, optarg, strerror(errno));
          return false;
        } else if (*suffix != '\0') {
          ERROR("'-%c %s': Unknown int suffix '%s'", arg_id, optarg, suffix);
          return false;
        } else if (d_args->offset < 0) {
          ERROR("'-%c %s': Must be POSITIVE INTEGER", arg_id, optarg);
          return false;
        }

      } break;

      case ARG_SIZE: {
        char *suffix;
        if (!StrToInt_i64(optarg, 0, &(d_args->size), &suffix)) {
          ERROR("'-%c %s': %s", arg_id, optarg, strerror(errno));
          return false;
        } else if (*suffix != '\0') {
          ERROR("'-%c %s': Unknown int suffix '%s'", arg_id, optarg, suffix);
          return false;
        } else if (!((d_args->size == -1) || (d_args->size >= 0))) {
          ERROR("'-%c %s': Must be '-1' OR POSITIVE INTEGER", arg_id, optarg);
          return false;
        }
      } break;

      case ARG_UNKNOWN:
        switch (optopt) {
          case ARG_OFFSET:
          case ARG_SIZE:
          case ARG_VERBOSE:
            ERROR("'-%c': Missing mandatory argument", optopt);
            break;
          case 0:
            ERROR("'%s': Unknown option", argv[optind - 1]);
            break;
          default:
            ERROR("'-%c': Unknown option", optopt);
            break;
        }
        return false;

      default:
        ERROR("'-%c': Not implemented", arg_id);
        break;
    }
  }

  if ((d_args->file = argv[optind++]) == NULL) {
    ERROR("Missing mandatory FILE argument");
    return false;
  }

  if (optind != argc) {
    ERROR("Unknown positional arguments:");

    for (int i = optind; i < argc; ++i) {
      ERROR("- %s", argv[i]);
    }

    return false;
  }

  return true;
}

static bool Args_UpdateFromFd(struct Args *args, int fd) {
  assert(args != NULL);

  struct stat stats;
  bool success = true;

  if (fstat(fd, &stats) == -1) {
    ERROR("fstat: %s", strerror(errno));
    success = false;
  } else if (!S_ISREG(stats.st_mode)) {
    ERROR("Not a regular file (doesn't have a size)");
    success = false;
  } else {
    DEBUG("File: %li bytes", stats.st_size);
    if (args->offset > stats.st_size) {
      ERROR("Offset (0x%zX) is too big (file is 0x%zX bytes)", args->offset,
            stats.st_size);
      success = false;
    } else {
      if (args->size < 0) {
        args->size = stats.st_size;
      }

      if ((stats.st_size - args->offset) < args->size) {
        args->size = (stats.st_size - args->offset);
        INFO("Set size to %zu bytes (match the file size)", args->size);
      }
    }
  }

  return success;
}

static bool FD_Read(int fd, uint8_t *buffer, size_t size, size_t *d_read) {
  assert(buffer != NULL);

  size_t total = 0;
  ssize_t last_read = 0;

  while ((size > 0) && ((last_read = read(fd, buffer, size)) > 0)) {
    total += (size_t)last_read;

    buffer += last_read;
    size -= (size_t)last_read;
  }

  bool success = (last_read >= 0);
  if (success && (d_read != NULL)) {
    *d_read = total;
  }

  return success;
}

static bool FD_Write(int fd, uint8_t const *buffer, size_t size) {
  assert(buffer != NULL);

  ssize_t written = 0;

  while ((size > 0) && ((written = write(fd, buffer, size)) > 0)) {
    buffer += written;
    size -= (size_t)written;
  }

  return (written != -1);
}

static int STDIN_BytesAvailables(void) {
  int bytes = 0;

  if (ioctl(fileno(stdin), FIONREAD, &bytes) == -1) {
    return -1;
  }

  return bytes;
}
