# Five One One BPlus

### Overview

This is a practice project to familiarize myself with Python's C API. The goal
is to implement a B-Plus Tree in C that is usable in Python.

### Status

The project is in an alpha phase. PyObject insertion functionality has been
implemented. Support for `iter()` has been implemented, however it currently
involves placing the contents of the tree in a list and then returning a
`list_iterator` (it is hoped that this will be improved upon). Item deletion
functionality has not yet been started. Support for union and intersection is
hoped to be added in the near future.

The project will be labeled as being in a beta phase when deletion
functionality is implemented.

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

To run the tests:
```
make test
```

### TODOs

* implement item deletion
* add support for key/value pairs (for map type)
