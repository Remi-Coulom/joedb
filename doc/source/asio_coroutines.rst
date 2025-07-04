Using asio with Coroutines
==========================

Joedb was my first experience of writing networking code in C++, and I found it
very difficult. Asio itself does not provide good introductory documentation
for beginners. All the blog posts and conference videos I watched about async
programming made me feel stupid, because I could not make any sense of them. I
eventually managed to understand what I needed, but it took a lot of time.

This document summarizes what I learnt. It tries to be the document I would
have needed when I started. I hope it can be useful to other C++ programmers.

Asynchronous Programming
------------------------

The need for asynchronous programming arises as soon as a program wants to do
something else while waiting for the completion of time-consuming operations
that take place outside of the CPU. Typical examples are user input, file i/o,
network i/o, or executing calculations on a co-processor, such as a GPU.

Handling multiple parallel operations can be implemented with threads, but
threads are expensive. Asynchronous programming is an alternative to threads,
that provides much better scalability.

Parallelism between i/o operations can be implemented without threads by using
callbacks. This is the basic mechanism provided by low-level OS APIs.

TODO: example code

Using callbacks can provide better performance than threads, but they make
programming a lot more cumbersome. In particular, lifetimes become more
complex: objects have to be constructed in one function, and deleted in another
one. Smart pointers help, but are not as convenient as a simple scope-based
RAII around the i/o operation.

Coroutines
----------

Coroutines are a C++ language feature introduced in C++20 that allow writing
asynchronous code almost as conveniently as synchronous code: instead of ending
an operation in a separate callback, the code before and after the operation
can be written sequentially, in the same local scope.

Coroutines are described in `coroutine page of cppreference.com
<https://en.cppreference.com/w/cpp/language/coroutines.html>`_.

Using Coroutines with asio
--------------------------

The documentation of asio provides simple coroutine examples: an echo server,
and an http server. Both are extremely easy to implement with coroutines,
because sessions are a linear sequence of read/write, and do not interact with
each other.

Avoiding Template Bloat
-----------------------

Asio and Beast are header-only template libraries, which is convenient in some
ways, but has two major drawbacks:

 - compilation is very slow
 - compilation generates a lot of code, and can result in large binaries

For instance, the `secure websocket client example of boost::beast
<https://github.com/boostorg/beast/blob/develop/example/websocket/client/sync-ssl/websocket_client_sync_ssl.cpp>`_
takes 13 seconds to compile, and produces a 800kB binary. Adding websocket
support to joedb doubled the size of the library, and tripled the compilation
time.
