import pytest
import random
import sys

from five_one_one_bplus import BPlusSet

from tests.utils import (
    get_randostrs,
    parametrized_range,
)

b1 = pytest.mark.parametrize("b1", [2, 8, 32, 128])
b2 = pytest.mark.parametrize("b2", [2, 8, 32, 128])

@b1
@b2
def test_richcompare_empty(b1, b2):
    """
    Tests that B-Plus Trees with various combinations of values for B are able
    to:
        1: evaluate to equal when empty.
    """
    s1 = BPlusSet(b=b1)
    s2 = BPlusSet(b=b2)

    assert s1 == s2

@b1
@b2
def test_richcompare_empty_inverse(b1, b2):
    """
    Tests that B-Plus Trees with various combinations of values for B are able
    to:
        1: evaluate to not not equal when empty
    """
    s1 = BPlusSet(b=b1)
    s2 = BPlusSet(b=b2)

    assert not (s1 != s2)

@b1
@b2
@parametrized_range
def test_richcompare_ranges(b1, b2, list_from_range):
    """
    Tests that B-Plus Trees with various combinations of values for B are able
    to:
        1: evaluate to not equal when they have unequal length
    """
    initializer1 = list_from_range
    initializer2 = list_from_range

    s1 = BPlusSet(initializer1, b=b1)
    s2 = BPlusSet(initializer2, b=b2)

    assert s1 == s2

@b1
@b2
@parametrized_range
def test_richcompare_ranges_ne1(b1, b2, list_from_range):
    """
    Tests that B-Plus Trees with various combinations of values for B are able
    to:
        1: evaluate to not equal when they have unequal length
    """
    initializer1 = [-1] + list_from_range
    initializer2 = list_from_range

    s1 = BPlusSet(initializer1, b=b1)
    s2 = BPlusSet(initializer2, b=b2)

    assert s1 != s2


# The following checks that collisions are handled correctly
# hash(sys.maxsize) == hash(3) == 3. Brilliant!
@b1
@b2
@parametrized_range
def test_richcompare_ranges_ne2(b1, b2, list_from_range):
    """
    Tests that B-Plus Trees with various combinations of values for B are able
    to:
        1: handle a collision
        2. evaluate to not equal when the have unequal length
    """
    initializer1 = list_from_range + [sys.maxsize]
    initializer2 = list_from_range

    s1 = BPlusSet(initializer1, b=b1)
    s2 = BPlusSet(initializer2, b=b2)

    assert s1 != s2

@b1
@b2
@parametrized_range
def test_richcompare_ranges_ne3(b1, b2, list_from_range):
    """
    Tests that B-Plus Trees with various combinations of values for B are able
    to:
        1: handle a collision
        2. evaluate to not equal when the have equal length but different sets
            of values.
    """
    initializer1 = list_from_range + [sys.maxsize]
    initializer2 = list_from_range + [random.randint(0, sys.maxsize-1)]

    s1 = BPlusSet(initializer1, b=b1)
    s2 = BPlusSet(initializer2, b=b2)

    assert s1 != s2

@b1
@b2
def test_richcompare_unequal_len(b1, b2):
    """
    Tests that B-Plus Trees with various combinations of values for B are able
    to:
        1: evaluate to not equal when they have unequal length
    """
    initializer1 = get_randostrs(num=128)
    initializer2 = initializer1 + get_randostrs(num=1)

    s1 = BPlusSet(initializer1, b=b1)
    s2 = BPlusSet(initializer2, b=b2)

    assert s1 != s2

@b1
@b2
def test_richcompare_unequal_len_inverse(b1, b2):
    """
    Tests that B-Plus Trees with various combinations of values for B are able
    to:
        1: evaluate to not equal when they have unequal length
    """
    initializer1 = get_randostrs(num=128)
    initializer2 = initializer1 + get_randostrs(num=1)

    s1 = BPlusSet(initializer1, b=b1)
    s2 = BPlusSet(initializer2, b=b2)

    assert not (s1 == s2)

@b1
@b2
def test_richcompare_equal(b1, b2):
    """
    Tests that B-Plus Trees with various combinations of values for B that have
    the same set of elements are able to:
        1: evaluate to equal
    """
    initializer1 = get_randostrs(num=128)
    initializer2 = initializer1

    s1 = BPlusSet(initializer1, b=b1)
    s2 = BPlusSet(initializer2, b=b2)

    assert s1 == s2

@b1
@b2
def test_richcompare_equal_inverse(b1, b2):
    """
    Tests that B-Plus Trees with various combinations of values for B that have
    the same set of elements are able to:
        1: evaluate to not not equal
    """
    initializer1 = get_randostrs(num=128)
    initializer2 = initializer1

    s1 = BPlusSet(initializer1, b=b1)
    s2 = BPlusSet(initializer2, b=b2)

    assert not (s1 != s2)
