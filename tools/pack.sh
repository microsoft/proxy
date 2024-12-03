#!/bin/bash
file="CMakeLists.txt"
version=$(grep -oP 'msft_proxy\s+VERSION\s+\K[0-9]+\.[0-9]+\.[0-9]+' "$file")
git tag "$version"
git push origin "$version"
tar -czf "proxy-$version.tgz" "proxy.h"
