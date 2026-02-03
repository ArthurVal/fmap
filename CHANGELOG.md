# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.3.1] - 2026-02-03

### Fixed

- RELEASE builds symlink issue

## [1.3.0] - 2026-02-03

### Changed

- Do not output USAGE when wrong args is detected
- Log formats:
  - Add `[fmap]`
  - Update stamp to `[YYYY-MM-DD HH:MM:SS,XXX]`

### Fixed

- Minor typos in logs
- `--help/--version` not ignored when FILE is missing

## [1.2.0] - 2026-01-18

### Changed

- Improved `README.md`
- Refactoring:
  - No more forward declaration of static functions in `fmap.c`
  - `OFFSET`: Can now be NEGATIVE
  - Logs updated

### Fixed

- `--version` missing trailing NEWLINE
- Missing `static inline` ([0eec733](https://github.com/ArthurVal/fmap/commit/0eec733a7367024f9be1ff63b66162228ad40d9a))

## [1.1.1]

### Fixed

- Issue when compiling on MacOS (`__mode_t`)
- STDIN detection (use `isatty()`)

## [1.1.0]

### Fixed

- Bug that output an ERROR when `FILE` is not a REGULAR FILE (like `/dev/mem`)
- `README.md`: Typos + Phrases

## [1.0.2] - 2026-01-12

### Added

- `README.md` content

### Fixed

- Typo in CHANGELOG

## [1.0.1] - 2026-01-12

### Added

- LICENSE (GPL3)
- CHANGELOG

### Remove

- Old `RD_RAW`/`RD_HEX` args mentions in Usage / ARG_TYPE enum

## [1.0.0] - 2026-01-11

### Added

- First running from `fmap.c` containing:
  - Arguments to set: FILE / [OFFSET] / [SIZE]
  - Logging
  - READ a file to STDOUT;
  - WRITE into a file from STDIN;

[unreleased]: https://github.com/ArthurVal/fmap/compare/v1.3.1...HEAD
[1.3.0]: https://github.com/ArthurVal/fmap/compare/v1.3.0...v1.3.1
[1.3.0]: https://github.com/ArthurVal/fmap/compare/v1.2.0...v1.3.0
[1.2.0]: https://github.com/ArthurVal/fmap/compare/v1.1.1...v1.2.0
[1.1.1]: https://github.com/ArthurVal/fmap/compare/v1.1.0...v1.1.1
[1.1.0]: https://github.com/ArthurVal/fmap/compare/v1.0.2...v1.1.0
[1.0.2]: https://github.com/ArthurVal/fmap/compare/v1.0.1...v1.0.2
[1.0.1]: https://github.com/ArthurVal/fmap/compare/v1.0.0...v1.0.1
[1.0.0]: https://github.com/ArthurVal/fmap/compare/v0.0.0...v1.0.0
