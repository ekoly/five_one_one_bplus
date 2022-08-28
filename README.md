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

### Performance

BPlusSet creation from a list has similar performance to the builtin set:
```
>>> from tests.utils import get_randostrs
>>> from five_one_one_bplus import BPlusSet
>>> import timeit
>>> timeit.timeit("set(get_randostrs(num=4096))", setup="from __main__ import get_randostrs", number=128)
10.611642378993565
>>> timeit.timeit("BPlusSet(get_randostrs(num=4096))", setup="from __main__ import BPlusSet, get_randostrs", number=128)
10.733390275010606
```

The BPlusSet's `in` keyword has worse performance than the builtin set,
especially for items that are in the BPlusSet. This is probably because if the
hash of the item is found in the BPlusSet, a RichCompare operation is performed
to verify that it is not the result of a hash collision. It is hoped that this
will be improved upon in a future iteration.
```
>>> from tests.utils import get_randostrs
>>> from five_one_one_bplus import BPlusSet
>>> import timeit
>>> randostrs = get_randostrs(num=4096)
>>> test_set = BPlusSet(randostrs)
>>> control_set = set(randostrs)
>>> # test for strings that are in the set.
>>> timeit.timeit("[x in control_set for x in randostrs]", setup="from __main__ import randostrs, control_set", number=128)
0.023006275005172938
>>> timeit.timeit("[x in test_set for x in randostrs]", setup="from __main__ import randostrs, test_set", number=128)
0.08527157898060977
>>> # test for strings that are not in the set.
>>> randostrs2 = get_randostrs(num=4096)
>>> timeit.timeit("[x in control_set for x in randostrs2]", setup="from __main__ import randostrs2, control_set", number=128)
0.05459781602257863
>>> timeit.timeit("[x in test_set for x in randostrs2]", setup="from __main__ import randostrs2, test_set", number=128)
0.06610740601900034
```

### TODOs

* implement item deletion
* add support for key/value pairs (for map type)
