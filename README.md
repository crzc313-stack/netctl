# netctl

Fast DNS benchmarking & network diagnostics from your terminal.

[![Build](https://img.shields.io/badge/build-passing-brightgreen)](Makefile) [![License](https://img.shields.io/badge/license-MIT-blue)](LICENSE) [![Language](https://img.shields.io/badge/language-C-00599C)](https://en.wikipedia.org/wiki/C_(programming_language))

`netctl` sends DNS queries to multiple resolvers in parallel and reports their response time in a compact terminal table.

## Install

On a network with access to GitHub:

```sh
curl -fsSL https://raw.githubusercontent.com/crzc313-stack/netctl/main/install.sh | sh
```

If `raw.githubusercontent.com` is blocked or cannot be resolved, use a VPN or
your own HTTPS proxy. As a temporary third-party mirror fallback, you can run:

```sh
curl -fsSL https://gh-proxy.com/https://raw.githubusercontent.com/crzc313-stack/netctl/main/install.sh | sh
```

The mirror is not operated by this project. For a fully local install, clone
the repository, build it, and install the binary:

```sh
git clone https://github.com/crzc313-stack/netctl.git
cd netctl
make
sudo install -m 755 netctl /usr/local/bin/netctl
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
