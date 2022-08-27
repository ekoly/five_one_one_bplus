# Five One One BPlus

### Overview

This is a practice project to familiarize myself with Python's C API. The goal
is to implement a B Plus Tree in C that is usable in Python. Eventually types
such as sets and maps that make use of B Plus Trees "under the hood" may be
added.

### Status

The project is in a pre-alpha phase. PyObject insertion functionality has been
implemented, however it occassionally segfaults for unknown reasons. Deletion
and iteration functionality has not been started yet.

### Usage

It is recommended to activate a virtual environment before installing.

To install:
```
make install
```

To make use of the BPlusSet class:
```
>>> from five_one_one_bplus import BPlusSet
>>> s = BPlusSet([511, "mel ott"], b=16)
>>> 511 in s
True
>>> "mel ott" in s
True
>>> "foo" in s
False
>>> s.add("foo")
>>> "foo" in s
True
```

### TODOs

* unit tests!
* implement iteration
* implement item deletion
* add support for key/value pairs (for map type)
