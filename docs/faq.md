# Proxy's Frequently Asked Questions

- [**What is "Proxy" and how does it work?**](#what)
- [**Why is "Proxy" so popular?**](#why-popular)
- [**Who is "Proxy" for?**](#who-for)
- [**Why should I use "Proxy"?**](#why-use)
- [**How to learn "Proxy" effectively?**](#how-learn)
- [**How to integrate "Proxy" into my project?**](#how-integrate)
- [**My existing project uses virtual functions. How should I migrate to "Proxy"?**](#how-migrate)
- [**How is the performance compared to virtual functions?**](#performance)
- [**Why is "Proxy" based on pointer semantics rather than value semantics like `std::function`?**](#why-pointer)
- [**Why does "Proxy" define several macros instead of modern C++ facilities?**](#why-macros)
- [**What is the standardization progress of this library?**](#standardization)
- [**What should I do if I found this library deficient in my scenario?**](#help-needed)

### <a name="what">What is "Proxy" and how does it work?</a>

"Proxy" is a single-header, cross-platform C++20 template library for modern runtime polymorphism based on pointer-semantics. Similar with C++ [virtual functions](https://en.cppreference.com/w/cpp/language/virtual), it generates indirect functions behind the scenes at compile-time but does not require inheritance. It also has [GC](https://en.wikipedia.org/wiki/Garbage_collection_(computer_science))-like behavior that allows different objects of the same `proxy` type to have different lifetime models without requiring runtime GC overhead.

### <a name="why-popular">Why is "Proxy" so popular?</a>

"Proxy" is built by engineers at Microsoft and initially deployed in the Windows operating system. For 40 years, the inheritance-based polymorphism paradigm has been the only scalable solution for runtime polymorphism in C++. However, a "virtual function" is no longer the optimal choice for runtime polymorphism today, and new languages with better paradigms, like [traits in Rust](https://doc.rust-lang.org/book/ch10-02-traits.html), are emerging. "Proxy" is our latest and greatest solution for generic runtime polymorphism in C++. It is easy to integrate and makes C++ feel like a brand new language when dealing with runtime abstractions.

### <a name="who-for">Who is "Proxy" for?</a>

We encourage every C++ developer to use this library for production, including in embedded engineering scenarios. Before deciding to write any virtual function, consider whether using "Proxy" could simplify the architecture.

### <a name="why-use">Why should I use "Proxy"?</a>

While virtual functions have served well for decades, "Proxy" offers modern solutions that can lead to better performance, easier maintenance, and more flexible code design when it comes to runtime abstraction.

### <a name="how-learn">How to learn "Proxy" effectively?</a>

The fundamental abstraction of "Proxy" is called "facade". It is recommended for beginners to start with the examples in the [README](../README.md), try to understand the pattern of defining a [`facade`](ProFacade.md) type, using a facade type to specify a [`proxy`](proxy.md) type, and creating and using a proxy object at runtime. Don't hesitate to consult the [specifications](specifications.md) for more details about any facility in the library.

### <a name="how-integrate">How to integrate "Proxy" into my project?</a>

Since "Proxy" is a single-header library, you can simply navigate to the [latest release](https://github.com/microsoft/proxy/releases), download the source code, and include "proxy.h" in your project. Make sure your compiler version meets the [minimum requirements for compilers](../README.md#compiler-req). If your project has already integrated with [vcpkg](https://vcpkg.io/) or [conan](https://conan.io/), just search for the keyword "proxy" and install it. Thanks to the community that helped port "Proxy" to these platforms!


### <a name="how-migrate">My existing project uses virtual functions. How should I migrate to "Proxy"?</a>

Follow the 4 steps below to upgrade an existing project from using virtual functions to "Proxy":

1. Update the compiler version to meet our [minimum requirements for compilers](../README.md#compiler-req).
2. Define [`facade`](ProFacade.md) types that match the "base classes with virtual functions" (virtual base classes).
3. Replace all the usage of virtual base classes with [`proxy`](proxy.md) from the API boundary.
4. Remove all the definitions and inheritance of virtual base classes.

### <a name="performance">How is the performance compared to virtual functions?</a>

The design of "Proxy" follows the [zero-overhead principle](https://en.cppreference.com/w/cpp/language/Zero-overhead_principle). With general compiler optimizations, "Proxy" is expected to generate high quality code in most scenarios that is not worse than an equivalent hand-written implementation with or without virtual functions. In practice, when a construct of runtime abstraction has ownership of its context, "Proxy" usually has better performance than the inheritance-based approach in lifetime management. When performing an indirect call, "Proxy" usually generates similar code to a virtual function with equal performance. We have observed that when the concrete implementation and the abstraction appear in the same translation unit, a virtual function is more likely to be devirtualized in the generated code, but sometimes the compiler won't do that trick for "Proxy".

### <a name="why-pointer">Why is "Proxy" based on pointer semantics rather than value semantics like [std::function](https://en.cppreference.com/w/cpp/utility/functional/function)?</a>

At the beginning, we explored the feasibility of designing a general-purpose polymorphic wrapper based on value semantics, just like [`std::function`](https://en.cppreference.com/w/cpp/utility/functional/function) and [`std::move_only_function`](https://en.cppreference.com/w/cpp/utility/functional/move_only_function). It is technically feasible, but not as good as those languages with [GC](https://en.wikipedia.org/wiki/Garbage_collection_(computer_science)) like C# or Java in terms of usability. We had a hard time refining the theory of [OOP](https://en.wikipedia.org/wiki/Object-oriented_programming) and finally realized that indirection is the nature of computer science, and decided to leverage the concept of pointer in C++ as the basis of "Proxy".

### <a name="why-macros">Why does "Proxy" define several macros instead of modern C++ facilities?</a>

"Proxy" defines 4 macros: [`__msft_lib_proxy`](msft_lib_proxy.md), [`PRO_DEF_MEM_DISPATCH`](PRO_DEF_MEM_DISPATCH.md), [`PRO_DEF_FREE_DISPATCH`](PRO_DEF_FREE_DISPATCH.md), and [`PRO_DEF_WEAK_DISPATCH`](PRO_DEF_WEAK_DISPATCH.md). [`__msft_lib_proxy`](msft_lib_proxy.md) is the feature test macro, following the existing practice in the C++20 standard. The other 3 macros are fundamental facilities to define a custom [`dispatch`](ProDispatch.md) type. These macros cannot be replaced by modern C++ facilities because there is no existing language feature prior to C++26 that allows generating a function with an arbitrary name. As a result, "Proxy" does not provide a default interface for [modules](https://en.cppreference.com/w/cpp/language/modules) as of now.

### <a name="standardization">What is the standardization progress of this library?</a>

Currently, there is [an ongoing proposal](https://wg21.link/p3086) being reviewed in the ISO C++ committee. The progress can be tracked [here](https://github.com/cplusplus/papers/issues/1741).

### <a name="help-needed">What should I do if I found this library deficient in my scenario?</a>

Please search for your scenario in the existing issues first, and feel free to file an a new one on demand, following the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
