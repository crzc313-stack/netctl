# netctl

Fast DNS benchmarking & network diagnostics from your terminal.

[![Build](https://img.shields.io/badge/build-passing-brightgreen)](https://github.com/crzc313-stack/netctl)
[![License](https://img.shields.io/badge/license-MIT-blue)](LICENSE)
[![Language](https://img.shields.io/badge/language-C-00599C)](https://en.wikipedia.org/wiki/C_(programming_language))

`netctl` sends DNS queries directly to the servers you choose, measures their response time, and presents the results in a compact, color-coded table.

## Install

```sh
curl -fsSL https://raw.githubusercontent.com/crzc313-stack/netctl/main/install.sh | sh
```

## Usage

Build from source with `make`, then benchmark the built-in resolver list:

```sh
./netctl dns example.com
```

Or provide one or more DNS servers explicitly:

```sh
./netctl dns example.com 1.1.1.1 8.8.8.8
```

Example output:

```text
 DNS benchmark for example.com

 #  SERVER              LATENCY       STATUS
 1  1.1.1.1               8.42 ms     OK
 2  8.8.8.8              12.17 ms     OK
 3  9.9.9.9              24.91 ms     OK
 4  208.67.222.222          --        FAIL (timeout)
```

The fastest successful result is shown in bold and successful or failed rows are colored green or red in a terminal that supports ANSI colors.

## Why netctl?

- **Parallel queries:** compare resolvers in one quick run.
- **Minimal dependencies:** libc and POSIX threads only.
- **Single binary:** easy to build, copy, and run anywhere with a C toolchain.

## Contributing

Bug reports, improvements, and pull requests are welcome. Please keep changes focused, build with `make`, and run `make test` before submitting a pull request.

## License

MIT, 2026, crzc313-stack.
