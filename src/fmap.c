/* libc  */
#include <assert.h>
#include <errno.h>  /* errno */
#include <stdarg.h> /* va_list */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> /* EXIT_SUCCESS */
#include <string.h> /* strerror */

/* POSIX */
#include <fcntl.h>    /* open */
#include <getopt.h>   /* getopt_long */
#include <sys/mman.h> /* mmap */
#include <sys/stat.h> /* stat */
#include <unistd.h>   /* close */

/* internal */
#include "config.h"
#include "logging.h"

static void Usage(FILE *restrict f, const char *restrict name) {
  assert(name != NULL);

  fprintf(
      f,
      "fmap v" FMAP_VERSION_STR
      "\nUsage: %s"
      "\n       FILE"
      "\n       [-o OFFSET] [-s SIZE]"
      "\n       [-h] [--version] [-v VERBOSE]"
      "\n"
      "\nMap FILE's memory and OUTPUT its content to STDOUT."
      "\nIf STDIN got ANY data, copy STDIN into the mapped memory instead."
      "\n"
      "\nThe mapped region can be customized using OFFSET and SIZE"
      "\nin order to map a file using [OFFSET; OFFSET+SIZE) memory range."
      "\n"
      "\nEffectively does the same as 'cat' but use 'mmap' instead, which can"
      "\nbe used to interact with specific devices (like /dev/mem for "
      "example)."
      "\n"
      "\nPositional arguments (mandatory):"
      "\n FILE       Name of the file we wish to map"
      "\n"
      "\nOptions:"
      "\n -h/--help"
      "\n            Show this help message and exit"
      "\n -o/--offset N"
      "\n            OFFSET of the mapping (in BYTES)"
      "\n            > 0: Relative to the begin of the FILE"
      "\n            < 0: Relative to the end of the FILE (REG FILE only)"
      "\n            (default: 0)"
      "\n -s/--size N"
      "\n            SIZE of the mapping (in BYTES)"
      "\n            < 0: Match the FILE size (REG FILE only)"
      "\n            (default: -1)"
      "\n -v/--verbose [DEBUG, INFO, WARN, ERROR]"
      "\n            Log level"
      "\n            (default: WARN)"
      "\n --version"
      "\n            Print the current script version and exit"
      "\n",
      name);
}

static bool Parse_i64(const char *restrict str, int64_t *d_value,
                      char **restrict d_remaining) {
  assert(str != NULL);
  assert(d_value != NULL);

  errno = 0;
  bool success = true;
  int64_t value = strtol(str, d_remaining, 0);
  if (errno == ERANGE) {
    success = false;
  } else {
    *d_value = value;
  }

  return success;
}

struct Args {
  const char *file;

  ssize_t offset;
  ssize_t size;

  union {
    uint32_t all;
    struct {
      uint32_t help : 1;
      uint32_t version : 1;
    };
  } flags;
};

static bool Args_FromArgv(struct Args *restrict args, int argc, char *argv[]) {
  assert(args != NULL);
  assert(argv != NULL);

  bool success = true;

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
  } arg_id;

  struct option const my_options[] = {
      {"offset", required_argument, NULL, ARG_OFFSET},
      {"size", required_argument, NULL, ARG_SIZE},
      {"verbose", required_argument, NULL, ARG_VERBOSE},
      {"help", no_argument, NULL, ARG_HELP},
      {"version", no_argument, NULL, ARG_VERSION},

      {NULL, 0, NULL, 0}, /* END */
  };

  opterr = 0; /* Disable auto error logs from getopt */
  while ((arg_id = getopt_long(argc, argv, "o:s:v:h", my_options, NULL)) !=
         ARG_END) {
    switch (arg_id) {
      case ARG_HELP:
        args->flags.help = true;
        break;

      case ARG_VERSION:
        args->flags.version = true;
        break;

      case ARG_VERBOSE: {
        LogLevel lvl;
        if (!LogLevel_FromString(optarg, &(lvl))) {
          ERROR("Unknown log level '%s'", optarg);
          success = false;
        } else {
          Logging_Level(&lvl);
        }
      } break;

      case ARG_SIZE:
      case ARG_OFFSET: {
        int64_t *val = arg_id == ARG_SIZE ? &(args->size) : &(args->offset);
        char *suffix;
        if (!Parse_i64(optarg, val, &suffix)) {
          ERROR("'-%c %s': %s", arg_id, optarg, strerror(errno));
          success = false;
        } else if (*suffix != '\0') {
          ERROR("'-%c %s': Unknown int suffix '%s'", arg_id, optarg, suffix);
          success = false;
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
        success = false;
        break;

      default:
        ERROR("'-%c': Not implemented", arg_id);
        success = false;
        break;
    }
  }

  if ((args->file = argv[optind++]) == NULL) {
    ERROR("Missing mandatory FILE argument");
    success = false;
  } else if (optind != argc) {
    ERROR("Unknown positional arguments:");

    for (int i = optind; i < argc; ++i) {
      ERROR("- %s", argv[i]);
    }

    success = false;
  }

  return success;
}

static const char *stat_ModeToString(struct stat const *stat) {
  assert(stat != NULL);

  switch (stat->st_mode & S_IFMT) {
    case S_IFBLK:
      return "BLK DEV";
    case S_IFCHR:
      return "CHR DEV";
    case S_IFDIR:
      return "DIR";
    case S_IFIFO:
      return "PIPE";
    case S_IFLNK:
      return "LNK";
    case S_IFREG:
      return "FILE";
    case S_IFSOCK:
      return "SOCK";
  }

  return "UNKNOWN";
}

static bool File_GetSize(int fd, long *d_size) {
  assert(fd > 0);
  assert(d_size != NULL);

  struct stat stats;
  bool success = true;

  if (fstat(fd, &stats) == -1) {
    ERROR("fstat: %s", strerror(errno));
    success = false;
  } else {
    DEBUG("File size (type: %s): %li bytes", stat_ModeToString(&stats),
          stats.st_size);

    *d_size = stats.st_size;
  }

  return success;
}

static bool File_UpdateRange(int fd, ssize_t offset, ssize_t size,
                             ssize_t *restrict d_offset,
                             ssize_t *restrict d_size) {
  assert(fd > 0);

  assert(d_size != NULL);
  assert(d_offset != NULL);

  bool success = true;
  long f_size;

  if (!File_GetSize(fd, &f_size)) {
    ERROR("Couldn't retreive FILE size");
    success = false;
  } else if (f_size <= 0) {
    ERROR("Wrong FILE size (%li)", f_size);
    ERROR(
        "HINT: FILE may not be a REGULAR FILE and doesn't have a size "
        "(PIPE/...)");
    success = false;
  } else if ((offset > f_size) || (offset < -f_size)) {
    ERROR("Wrong OFFSET (%li) w.r.t. the FILE'size (%li)", offset, f_size);
    success = false;
  } else {
    if (offset < 0) {
      offset = f_size + offset;
    }

    if (size < 0) {
      size = f_size - offset;
    }

    if ((f_size - offset) < size) {
      ERROR(
          "Wrong SIZE (%li) w.r.t the OFFSET (%li) and the FILE'size (%li) "
          "(Size remaining: %li)",
          size, offset, f_size, (f_size - offset));
      success = false;
    } else {
      *d_offset = offset;
      *d_size = size;
    }
  }

  return success;
}

static bool File_ReadInto(int fd, uint8_t *restrict buffer, size_t size,
                          size_t *restrict d_read) {
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

static bool File_Write(int fd, uint8_t const *buffer, size_t size) {
  assert(buffer != NULL);

  ssize_t written = 0;

  while ((size > 0) && ((written = write(fd, buffer, size)) > 0)) {
    buffer += written;
    size -= (size_t)written;
  }

  return (written != -1);
}

struct Mapping {
  uint8_t *data;
  size_t size;
  size_t alignment;
};

static bool Mapping_FromFile(struct Mapping *mapping, int fd, size_t offset,
                             size_t size) {
  assert(mapping != NULL);
  assert(fd > 0);

  bool success = true;

  size_t alignment =
      offset - (offset & ((size_t) ~(sysconf(_SC_PAGE_SIZE) - 1)));

  DEBUG(
      "Mapping:"
      "\n- OFFSET: %zu (Aligned to %li - PAGE: %li)"
      "\n- SIZE  : %zu",
      offset, (offset - alignment), sysconf(_SC_PAGE_SIZE), size);

  uint8_t *mem = mmap(NULL, (size + alignment), PROT_READ | PROT_WRITE,
                      MAP_SHARED, fd, (off_t)(offset - alignment));

  if (mem == MAP_FAILED) {
    ERROR("mmap: %s", strerror(errno));
    success = false;
  } else {
    mapping->data = mem + alignment;
    mapping->size = (size_t)size;
    mapping->alignment = (size_t)alignment;
  }

  return success;
}

static bool Mapping_Unmap(struct Mapping *mapping) {
  assert(mapping != NULL);
  assert(mapping->data != NULL);

  bool success = true;
  void *mem = mapping->data - mapping->alignment;
  if (munmap(mem, (mapping->size + mapping->alignment)) != 0) {
    ERROR("munmap: %s", strerror(errno));
    success = false;
  } else {
    mapping->data = NULL;
    mapping->size = 0;
    mapping->alignment = 0;
  }

  return success;
}

int main(int argc, char *argv[]) {
  int ret = EXIT_SUCCESS;
  int fd = -1;
  struct Args args = {
      .file = NULL,
      .offset = 0,
      .size = -1,

      .flags.all = 0x0,
  };

  struct Mapping mapping = {
      .data = NULL,
      .size = 0,
      .alignment = 0,
  };

  if (!Args_FromArgv(&args, argc, argv)) {
    ret = EXIT_FAILURE;
    goto end;
  }

  if (args.flags.help) {
    Usage(stdout, argv[0]);
    goto end;
  }

  if (args.flags.version) {
    puts(FMAP_VERSION_STR);
    goto end;
  }

  DEBUG(
      "Args:"
      "\n - FILE  : %s"
      "\n - OFFSET: %li bytes"
      "\n - SIZE  : %li bytes",
      args.file, args.offset, args.size);

  INFO("Opening '%s': ...", args.file);
  if ((fd = open(args.file, O_RDWR | O_SYNC)) == -1) {
    ERROR("open(): %s", strerror(errno));
    ERROR("Opening '%s': FAILED", args.file);
    ret = EXIT_FAILURE;
    goto end;
  }

  if (args.offset < 0 || args.size < 0) {
    INFO("Matching OFFSET/SIZE to FILE: ...");
    if (!File_UpdateRange(fd, args.offset, args.size, &(args.offset),
                          &(args.size))) {
      ERROR("Matching OFFSET/SIZE to FILE: FAILED");
      ret = EXIT_FAILURE;
      goto end_after_open;
    }
  }

  INFO("Mapping FILE (Range: [%li; %li)): ...", args.offset, args.size);

  assert(args.offset >= 0);
  assert(args.size >= 0);

  if (!Mapping_FromFile(&mapping, fd, (size_t)args.offset, (size_t)args.size)) {
    ERROR("Mapping FILE (Range: [%li; %li)): FAILED", args.offset, args.size);
    ret = EXIT_FAILURE;
    goto end_after_open;
  }

  if (isatty(fileno(stdin))) {
    INFO("Reading: ...");
    if (!File_Write(fileno(stdout), mapping.data, mapping.size)) {
      ERROR("Reading: FAILED: %s", strerror(errno));
      ret = EXIT_FAILURE;
      goto end_after_mmap;
    }
  } else {
    size_t written = 0;
    INFO("Writing: ...");
    if (!File_ReadInto(fileno(stdin), mapping.data, mapping.size, &written)) {
      ERROR("Writing: FAILED: %s", strerror(errno));
      ret = EXIT_FAILURE;
      goto end_after_mmap;
    } else {
      DEBUG("Wrote: %zu bytes", written);
    }
  }

end_after_mmap:
  INFO("Unmapping memory: ...");
  if (!Mapping_Unmap(&mapping)) {
    WARN("Unmapping memory: FAILED");
  }

end_after_open:
  INFO("Closing file: ...");
  if (close(fd) == -1) {
    ERROR("close(): %s", strerror(errno));
    WARN("Closing file: FAILED");
  }

end:
  return ret;
}
