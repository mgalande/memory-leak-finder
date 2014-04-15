Finding memory leaks in C
=============================

This library can be used to find memory leaks in C programs.

Finding memory leak in C programs is difficult task and it gets even more
difficult when the leak observed at the customer is not reproducible in the
labs. And there are not many tools to help in such situations, without making
any changes to the source-code.

This is an attempt to help find memory leaks with no source-code change, not
even recompile.

Supported platform
-------------------

Linux using `glibc` is the supported platform.


How to build
------------

To build the library run `make`.

##### Note

The library must be built for the same architecture as that of the executable that needs to be traced for memory leaks.

Usage
------

Once the library is built, copy the library `libtml.so` to the system that has the executable.


Use the following command to launch the executable:

    LD_PRELOAD=./libtml.so <executable-path> [arguments]


The library will produce a text file `tml.txt` that will
  contain the traced logs of `malloc`, `realloc`, and `free`.


Finding the leaks
--------------------

Once loaded into the executable, the library traces the memory management
calls like `malloc`, `realloc`, and `free`. It records each of these calls
before actually executing them. It also records the stack trace (upto 10
frames) for each of the call. This helps to detect the actual location of the
memory leak.

##### TODO

Developing a scirpt that will produce user friendly memory-leak-output from the logged trace.


Feedback
---------

It will be great to have some feedback from the users. Send in anything that
you think of, something that is working/not working, some improvements etc.
