import pytest

from five_one_one_bplus import BPlusSet

from tests.utils import get_randostrs

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
