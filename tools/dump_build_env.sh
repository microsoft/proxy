#!/usr/bin/env bash

if [ $# -ne 2 ]; then
  echo "Usage: $0 <compiler> <output path>"
  exit 1
fi

COMPILER_INFO="$("$1" --version 2>/dev/null | awk 'NF {print; exit}')"
if [ -z "$COMPILER_INFO" ]; then
  echo "Unable to retrieve compiler info for '$1'."
  exit 1
fi

OS_NAME="$(uname -s)"
if [ "$OS_NAME" = "Linux" ]; then
  if [ -f /etc/os-release ]; then
    OS_NAME="$(grep ^PRETTY_NAME= /etc/os-release | cut -d= -f2 | tr -d '"')"
  fi
elif [ "$OS_NAME" = "Darwin" ]; then
  OS_NAME="$(sw_vers -productName) $(sw_vers -productVersion)"
fi

cat <<EOF > $2
{
  "OS": "$OS_NAME",
  "KernelVersion": "$(uname -r)",
  "Architecture": "$(uname -m)",
  "Compiler": "$COMPILER_INFO"
}
EOF
