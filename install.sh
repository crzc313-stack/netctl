#!/bin/sh

set -e

target=/usr/local/bin/netctl
temporary_directory=$(mktemp -d)
trap 'rm -rf "$temporary_directory"' EXIT

if curl -fsSL https://github.com/crzc313-stack/netctl/releases/latest/download/netctl -o "$temporary_directory/netctl"; then
	install -m 755 "$temporary_directory/netctl" "$target"
else
	curl -fsSL https://github.com/crzc313-stack/netctl/archive/refs/heads/main.tar.gz -o "$temporary_directory/netctl.tar.gz"
	tar -xzf "$temporary_directory/netctl.tar.gz" -C "$temporary_directory"
	make -C "$temporary_directory/netctl-main"
	install -m 755 "$temporary_directory/netctl-main/netctl" "$target"
fi

echo "netctl installed."