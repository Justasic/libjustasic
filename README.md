libjustasic
===========

A library which has all my C++ scraps and misc code as an actual useful library.
I have collected a lot of various interesting bits of source code and reused much
of it in projects I have worked on. Many of my projects I like to keep nearly dependency
free so I end up statically linking a lot of things.
This library lets me update all those little pieces of code in one spot and simply
make it a dependency upon said project.
Some of this code may be very C-like since I have a C-like coding style but some stuff
does in fact require C++ (and even C++11).


Portibility
-----------

Currently I only plan to make it linux-compatible but most posix systems should be easy
to support with little or no changes. Windows is very different however and will require
a lot of work.



Features
=========

- A DynamicLibrary class for shared-object files and DLLs, full plugin/module handling systems
- A multiplatform socket engine (I hope)
- A dynamically adjusting thread pooling engine
- Filesystem and memory management utilities
- String and byte manipulation stuff
- A set of secure functions (like what is seen in the OpenBSD libc)
- Interprocess communication and shared memory utilities
- Time and randomness functions
- Application event handling utilities which allow for dynamic expansion and contraction
- Much, much more I am sure (I just have to think of it) :D


Building
========

I am lazy and most people should be able to figure this out so here is the short instructions:

Download and install your favorite POSIX compiler that is C++11 compatible, make a new directory somewhere
run cmake <your source dir> from your new directory, run make or whatever in your new directory, run make install
or whatever in your new directory and TADA! yer done.


License
=======

Read the license file, licenses of external source code compatible with this license will have their
credits here as well as in a new License directory.