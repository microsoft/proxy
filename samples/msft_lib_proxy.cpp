// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
// This file contains example code from msft_lib_proxy.md.

#include <cstdio>

#include "proxy.h"

int main() {
#if defined(__msft_lib_proxy) && __msft_lib_proxy >= 202408L
  puts("Compiled with library Proxy 3.0.0 or above.");
#else
  puts("Cannot determine the version of library Proxy.");
#endif
}
