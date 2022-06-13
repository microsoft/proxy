# Proxy: Easy Polymorphism in C++

[![Proxy-CI](https://github.com/microsoft/proxy/actions/workflows/pipeline-ci.yml/badge.svg)](https://github.com/microsoft/proxy/actions/workflows/pipeline-ci.yml)

Do you want to save your effort in lifetime management and architecture maintenance of polymorphic objects in C++?

Do you want to be able to write polymorphic code in C++ as easily as in those languages with GC (like Java or C#), while still having excellent runtime performance?

Have you tried other polymorphic programming libraries in C++ but found them somewhat deficient?

If so, this library is for you. 😉

The "proxy" is a C++ library that Microsoft uses to make runtime polymorphism easier to implement, both in architecture design and runtime performance. Please find the design details at https://wg21.link/p0957.

## Build with CMake

```
git clone https://github.com/microsoft/proxy.git
cd proxy
cmake -S . -B build
cmake --build ./build -j8
cd ./build
ctest -j8
```

## Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## Trademarks

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft 
trademarks or logos is subject to and must follow 
[Microsoft's Trademark & Brand Guidelines](https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks/usage/general).
Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship.
Any use of third-party trademarks or logos are subject to those third-party's policies.
