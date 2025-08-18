# Proxy's Frequently Asked Questions

- [**What is "Proxy" and how does it work?**](#what)
- [**Why is "Proxy" so popular?**](#why-popular)
- [**Who is "Proxy" for?**](#who-for)
- [**Why should I use "Proxy"?**](#why-use)
- [**How to learn "Proxy" effectively?**](#how-learn)
- [**How to integrate "Proxy" into my project?**](#how-integrate)
- [**My existing project uses virtual functions. How should I migrate to "Proxy"?**](#how-migrate)
- [**How is the performance compared to virtual functions?**](#performance)
- [**How does "Proxy" compare with CRTP in terms of performance and lifetime management?**](#crtp-performance)
- [**Why is "Proxy" based on pointer semantics rather than value semantics like `std::function`?**](#why-pointer)
- [**Why does "Proxy" define several macros instead of modern C++ facilities?**](#why-macros)
- [**What is the standardization progress of this library?**](#standardization)
- [**How do I upgrade "Proxy" in a large codebase?**](#how-upgrade)
- [**What should I do if I found this library deficient in my scenario?**](#help-needed)

### <a name="what">What is "Proxy" and how does it work?</a>

"Proxy" is a header-only, cross-platform C++20 template library for modern runtime polymorphism based on pointer-semantics. Similar with C++ [virtual functions](https://en.cppreference.com/w/cpp/language/virtual), it generates indirect functions behind the scenes at compile-time but does not require inheritance. It also has [GC](https://en.wikipedia.org/wiki/Garbage_collection_(computer_science))-like behavior that allows different objects of the same `proxy` type to have different lifetime models without requiring runtime GC overhead.

### <a name="why-popular">Why is "Proxy" so popular?</a>

"Proxy" is built by engineers at Microsoft and initially deployed in the Windows operating system. For 40 years, the inheritance-based polymorphism paradigm has been the only scalable solution for runtime polymorphism in C++. However, a "virtual function" is no longer the optimal choice for runtime polymorphism today, and new languages with better paradigms, like [traits in Rust](https://doc.rust-lang.org/book/ch10-02-traits.html), are emerging. "Proxy" is our latest and greatest solution for generic runtime polymorphism in C++. It is easy to integrate and makes C++ feel like a brand new language when dealing with runtime abstractions.

### <a name="who-for">Who is "Proxy" for?</a>

We encourage every C++ developer to use this library for production, including in embedded engineering scenarios. Before deciding to write any virtual function, consider whether using "Proxy" could simplify the architecture.

### <a name="why-use">Why should I use "Proxy"?</a>

While virtual functions have served well for decades, "Proxy" offers modern solutions that can lead to better performance, easier maintenance, and more flexible code design when it comes to runtime abstraction.

### <a name="how-learn">How to learn "Proxy" effectively?</a>

The fundamental abstraction of "Proxy" is called "facade". It is recommended for beginners to start with the examples in the [home page](README.md#quick-start), try to understand the pattern of defining a [`facade`](spec/facade.md) type, using a facade type to specify a [`proxy`](spec/proxy/README.md) type, and creating and using a proxy object at runtime. Don't hesitate to consult the [specifications](spec/README.md) for more details about any facility in the library.

### <a name="how-integrate">How to integrate "Proxy" into my project?</a>

Since "Proxy" is a header-only library, you can simply navigate to the [latest release](https://github.com/microsoft/proxy/releases), download the source code, and include "proxy.h" in your project. Make sure your compiler version meets the [minimum requirements for compilers](README.md#minimum-requirements-for-compilers). If your project has already integrated with [vcpkg](https://vcpkg.io/) or [conan](https://conan.io/), just search for the keyword "proxy" and install it. Thanks to the community that helped port "Proxy" to these platforms!

### <a name="how-migrate">My existing project uses virtual functions. How should I migrate to "Proxy"?</a>

Follow the 4 steps below to upgrade an existing project from using virtual functions to "Proxy":

1. Update the compiler version to meet our [minimum requirements for compilers](README.md#minimum-requirements-for-compilers).
2. Define [`facade`](spec/facade.md) types that match the "base classes with virtual functions" (virtual base classes).
3. Replace all the usage of virtual base classes with [`proxy`](spec/proxy/README.md) from the API boundary.
4. Remove all the definitions and inheritance of virtual base classes.

### <a name="performance">How is the performance compared to virtual functions?</a>

The design of "Proxy" follows the [zero-overhead principle](https://en.cppreference.com/w/cpp/language/Zero-overhead_principle). With general compiler optimizations, "Proxy" is expected to generate high quality code in most scenarios that is not worse than an equivalent hand-written implementation with or without virtual functions. In practice, "Proxy" usually demonstrates better performance in indirect invocations than virtual functions, and in lifetime management than standard smart pointers or polymorphic wrappers. Please refer to our [blog post](https://devblogs.microsoft.com/cppblog/analyzing-the-performance-of-the-proxy-library/) for more details.

### <a name="crtp-performance">How does "Proxy" compare with CRTP in terms of performance and lifetime management?</a>

CRTP (Curiously Recurring Template Pattern) delivers compile-time polymorphism only. With CRTP the compiler sees the exact type of the callee, so it usually emits a direct call that can be inlined and optimized aggressively. `proxy` inserts one level of indirection to obtain runtime polymorphism. In our internal micro-benchmarks a call through CRTP is roughly **1.5x-4x faster** than the same call routed through `proxy`; the exact factor depends on inlining decisions, cache locality, and argument size. If every nanosecond counts and the set of concrete types is known at compile time, CRTP can be the better choice. When you need runtime substitution, heterogeneous containers, or hot-swappable implementations, the slight overhead of `proxy` buys you those features.

CRTP never erases the concrete type, so each object is still manipulated directly and its lifetime is controlled by normal C++ ownership rules (stack, `new`, smart pointers, etc.). `proxy`, on the other hand, is a type-erasure facility: it stores any object behind an indirect interface and lets you pick a lifetime model (raw pointer, unique/shared ownership, arena, etc.) at the call-site. Because CRTP has no unified runtime representation, comparing "lifetime-management performance" with `proxy` is meaningless. The capability simply does not exist in CRTP.

### <a name="why-pointer">Why is "Proxy" based on pointer semantics rather than value semantics like [std::function](https://en.cppreference.com/w/cpp/utility/functional/function)?</a>

At the beginning, we explored the feasibility of designing a general-purpose polymorphic wrapper based on value semantics, just like [`std::function`](https://en.cppreference.com/w/cpp/utility/functional/function) and [`std::move_only_function`](https://en.cppreference.com/w/cpp/utility/functional/move_only_function). It is technically feasible, but not as good as those languages with [GC](https://en.wikipedia.org/wiki/Garbage_collection_(computer_science)) like C# or Java in terms of usability. We had a hard time refining the theory of [OOP](https://en.wikipedia.org/wiki/Object-oriented_programming) and finally realized that indirection is the nature of computer science, and decided to leverage the concept of pointer in C++ as the basis of "Proxy".

### <a name="why-macros">Why does "Proxy" define several macros instead of modern C++ facilities?</a>

"Proxy" defines 4 macros: [`__msft_lib_proxy`](spec/msft_lib_proxy.md), [`PRO_DEF_MEM_DISPATCH`](spec/PRO_DEF_MEM_DISPATCH.md), [`PRO_DEF_FREE_DISPATCH`](spec/PRO_DEF_FREE_DISPATCH.md), and [`PRO_DEF_FREE_AS_MEM_DISPATCH`](spec/PRO_DEF_FREE_AS_MEM_DISPATCH.md). [`__msft_lib_proxy`](spec/msft_lib_proxy.md) is the feature test macro, following the existing practice in the C++20 standard. The other 3 macros are fundamental facilities to define a custom [`dispatch`](spec/ProDispatch.md) type. These macros cannot be replaced by modern C++ facilities because there is no existing language feature prior to C++26 that allows generating a function with an arbitrary name. As a result, "Proxy" does not provide a default interface for [modules](https://en.cppreference.com/w/cpp/language/modules) as of now.

### <a name="standardization">What is the standardization progress of this library?</a>

Currently, there is [an ongoing proposal](https://wg21.link/p3086) being reviewed in the ISO C++ committee. The progress can be tracked [here](https://github.com/cplusplus/papers/issues/1741).

### <a name="how-upgrade">How do I upgrade "Proxy" in a large codebase?</a>

Upgrading a small component is usually straightforward, but migrating a monorepo or multi-module product can be challenging. Follow the guidelines below:

1. **Minor or patch upgrades (e.g. 3.3.0 → 3.4.0)**
    All 3.x.y releases preserve API/ABI compatibility, so different parts of the program may safely depend on different 3.x.y versions. No special action is required.

2. **Major upgrades (e.g. 3.4.0 → 4.0.0)**
  - If your current version is *earlier* than 3.4.0, migrate to 3.4.0 first.
  - Starting with 3.4.0, each major release is placed in a versioned inline namespace (`pro::v3`, `pro::v4`, …).  When a translation unit sees multiple majors, qualify the namespace explicitly:
    ```cpp
    pro::v3::foo(); // Proxy 3 API
    pro::v4::foo(); // Proxy 4 API
    ```
    The newest release re-exports its namespace as the inline (default) namespace, so unqualified calls (`pro::foo()`) resolve to the latest version once the migration is complete.
  - The macros also have major-qualified aliases, e.g. [`PRO4_DEF_MEM_DISPATCH`](spec/PRO_DEF_MEM_DISPATCH.md). Use these forms whenever headers from multiple majors are included in the same translation unit.
  - Upgrade subsystems incrementally, module-by-module or DLL-by-DLL. When every target depends only on the new major, drop the old include path and remove the previous version from your build.

These rules let old and new code coexist during the transition while keeping ODR violations at bay.

### <a name="help-needed">What should I do if I found this library deficient in my scenario?</a>

Please search for your scenario in the existing issues first, and feel free to file an a new one on demand, following the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
