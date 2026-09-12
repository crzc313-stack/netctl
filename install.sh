#!/bin/sh

set -e

curl -fsSL https://github.com/crzc313-stack/netctl/releases/latest/download/netctl -o /usr/local/bin/netctl
chmod +x /usr/local/bin/netctl
echo "netctl installed."