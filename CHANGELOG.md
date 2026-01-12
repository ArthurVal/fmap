# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keppachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).


## [Unreleased]

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

[unreleased]: https://github.com/ArthurVal/fmap/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/ArthurVal/fmap/compare/v0.0.0...v1.0.0
