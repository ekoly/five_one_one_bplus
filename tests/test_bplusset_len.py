import pytest
import sys

from tests.utils import (
    parametrized_b,
    parametrized_range,
    get_randostrs,
)


# range tests: test for len()
@parametrized_b
@parametrized_range
def test_range_add_to_empty_assert_len(bplusset_empty, set_from_range):
    """
    Test that a BPlusSet is able to:
        1. be initialized as empty
        2. have a number of elements added
        3. ends up with the correct len()
    """

    s = bplusset_empty

    for x in set_from_range:
        s.add(x)

    assert len(s) == len(set_from_range)

@parametrized_b
@parametrized_range
def test_range_initializer_assert_len(bplusset_factory, set_from_range):
    """
    Test that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. ends up with the correct len()
    """

    s = bplusset_factory(set_from_range)

    assert len(s) == len(set_from_range)

@parametrized_b
@parametrized_range
def test_range_initializer_then_add_assert_len1(bplusset_factory, set_from_range):
    """
    Test that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. have a number of elements added
        3. ends up with the correct len()
    """

    my_l = list(set_from_range)
    first_half = my_l[:len(my_l)]
    second_half = my_l[len(my_l):]

    s = bplusset_factory(first_half)

    for x in second_half:
        s.add(x)

    assert len(s) == len(set_from_range)

@parametrized_b
@parametrized_range
def test_range_initializer_then_add_assert_len2(bplusset_factory, set_from_range):
    """
    Test that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. have a number of elements added
        3. ends up with the correct len()
    """

    my_l = list(set_from_range)
    first_half = my_l[:len(my_l)]
    second_half = my_l[len(my_l):]

    s = bplusset_factory(second_half)

    for x in first_half:
        s.add(x)

    assert len(s) == len(set_from_range)

@parametrized_b
@parametrized_range
def test_range_initializer_then_add_duplicates_assert_len(bplusset_factory, set_from_range):
    """
    Test that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. have a number of elements added that were already in the set
        3. ends up with the correct len()
    """

    s = bplusset_factory(set_from_range)

    for x in set_from_range:
        s.add(x)

    assert len(s) == len(set_from_range)

@parametrized_b
@parametrized_range
def test_range_initializer_contains_duplicates_len(bplusset_factory, list_from_range):
    """
    Test that a BPlusSet is able to:
        1. be initialized from a non-empty iterable that contains duplicates
        2. ends up with the correct len()
    """

    s = bplusset_factory(list_from_range + list_from_range)

    assert len(s) == len(list_from_range)

@parametrized_b
def test_range_initializer_random_strings_len(bplusset_factory):
    """
    Tests that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. ends up with the correct len()
    """

    control = get_randostrs(num=1000)

    s = bplusset_factory(control)

    assert len(s) == len(control)

@parametrized_b
def test_range_initializer_random_strings_duplicates_len(bplusset_factory):
    """
    Tests that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. have a number of elements added that are already in the set
        3. ends up with the correct len()
    """

    control = get_randostrs(num=1000)

    s = bplusset_factory(control)

    for x in control:
        s.add(x)
    
    assert len(s) == len(control)

@parametrized_b
@parametrized_range
def test_range_initializer_contains_collisions(bplusset_factory, list_from_range):
    """
    Test that a BPlusSet is able to:
        1. be initialized from a non-empty iterable that contains collisions
        2. ends up with the correct len()
    """
    control = list_from_range + [sys.maxsize]
    s = bplusset_factory(control)

    assert len(s) == len(control)
