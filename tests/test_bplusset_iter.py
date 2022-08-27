import pytest

from tests.utils import (
    parametrized_b,
    parametrized_range,
)

@parametrized_b
def test_iter_empty(bplusset_empty):
    """
    Tests that a BPlusSet is able to:
        1. be initialized as empty
        2. be converted to a list
    """
    assert list(bplusset_empty) == []

@parametrized_b
@parametrized_range
def test_iter_add_to_empty(bplusset_empty, list_from_range):
    """
    Tests that a BPlusSet is able to:
        1. be initialized as empty
        2. have elements added
        3. be converted to a list
    """
    s = bplusset_empty
    for x in list_from_range:
        s.add(x)

    res = list(s)
    
    assert sorted(res) == sorted(list_from_range)

@parametrized_b
@parametrized_range
def test_iter_initializer(bplusset_factory, list_from_range):
    """
    Tests that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. be converted to a list
    """
    s = bplusset_factory(list_from_range)

    res = list(s)
    
    assert sorted(res) == sorted(list_from_range)

@parametrized_b
@parametrized_range
def test_iter_initializer_with_duplicates(bplusset_factory, list_from_range):
    """
    Tests that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. have elements added that were already in the set
        2. be converted to a list
    """
    s = bplusset_factory(list_from_range)

    for x in list_from_range:
        s.add(x)

    res = list(s)
    
    assert sorted(res) == sorted(list_from_range)
