import pytest
import sys

from tests.utils import (
    parametrized_b,
    parametrized_range,
    check_contains,
    get_subset,
    get_randints,
    get_randostrs,
)

# range tests: test for `in`

@parametrized_b
def test_initializer_with_collision_then_add_contains(bplusset_factory):
    """
    This is intended to test for a specific bug where len() becomes incorrect.
    Tests that a BPlusSet is able to:
        1. be initialized from an iterable that has a collision
        2. have elements added to it
        3. the `in` keyword works as expected
        4. len() works as expected
    """
    s = bplusset_factory([3, sys.maxsize])
    assert 3 in s
    assert sys.maxsize in s
    assert len(s) == 2
    assert 4 not in s

    s.add(4)
    assert 4 in s
    assert len(s) == 3

@parametrized_b
@parametrized_range
def test_range_initializer_assert_contains(bplusset_factory, set_from_range):
    """
    Tests that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. the `in` keyword works as expected
    """

    s = bplusset_factory(set_from_range)

    check_contains(s, set_from_range, get_subset(set_from_range))

@parametrized_b
@parametrized_range
def test_range_initializer_then_add_assert_contains1(bplusset_factory, set_from_range):
    """
    Tests that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. have elements added
        3. the `in` keyword works as expected
    """

    my_l = list(set_from_range)
    first_half = my_l[:len(my_l)]
    second_half = my_l[len(my_l):]

    s = bplusset_factory(first_half)

    for x in second_half:
        s.add(x)

    check_contains(s, set_from_range, get_subset(my_l))

@parametrized_b
@parametrized_range
def test_range_initializer_then_add_assert_contains2(bplusset_factory, list_from_range):
    """
    Tests that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. have elements added
        3. the `in` keyword works as expected
    """

    control = list_from_range

    first_half = control[:len(control)]
    second_half = control[len(control):]

    s = bplusset_factory(second_half)

    for x in first_half:
        s.add(x)

    check_contains(s, control, get_subset(control))

@parametrized_b
@parametrized_range
def test_range_initializer_then_add_duplicates_assert_contains(bplusset_factory, set_from_range):
    """
    Tests that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. have elements added that were already in the set
        3. the `in` keyword works as expected
    """

    s = bplusset_factory(set_from_range)

    for x in set_from_range:
        s.add(x)
    
    check_contains(s, set_from_range, get_subset(set_from_range))

@parametrized_b
@parametrized_range
def test_range_initializer_assert_not_contains(bplusset_factory, set_from_range):
    """
    Tests that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. the `in` keyword works as expected for variables not in the set
    """

    s = bplusset_factory(set_from_range)

    check_contains(s, set_from_range, get_randints())

@parametrized_b
@parametrized_range
def test_range_initializer_then_add_assert_not_contains1(bplusset_factory, set_from_range):
    """
    Tests that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. have elements added
        3. the `in` keyword works as expected for variables not in the set
    """

    my_l = list(set_from_range)
    first_half = my_l[:len(my_l)]
    second_half = my_l[len(my_l):]

    s = bplusset_factory(first_half)

    for x in second_half:
        s.add(x)

    check_contains(s, set_from_range, get_randints())

@parametrized_b
@parametrized_range
def test_range_initializer_then_add_assert_not_contains2(bplusset_factory, set_from_range):
    """
    Tests that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. have elements added
        3. the `in` keyword works as expected for variables not in the set
    """

    my_l = list(set_from_range)
    first_half = my_l[:len(my_l)]
    second_half = my_l[len(my_l):]

    s = bplusset_factory(second_half)

    for x in first_half:
        s.add(x)

    check_contains(s, set_from_range, get_randints())

@parametrized_b
@parametrized_range
def test_range_initializer_then_add_duplicates_assert_not_contains(bplusset_factory, set_from_range):
    """
    Tests that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. have elements added that were already in the set
        3. the `in` keyword works as expected for variables not in the set
    """

    s = bplusset_factory(set_from_range)

    for x in set_from_range:
        s.add(x)

    check_contains(s, set_from_range, get_randints())

@parametrized_b
def test_range_initializer_random_strings_contains(bplusset_factory):
    """
    Tests that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. the `in` keyword works as expected
    """

    control = get_randostrs(num=1000)

    s = bplusset_factory(control)

    check_contains(s, control, get_subset(control))

@parametrized_b
def test_range_initializer_random_strings_not_contains(bplusset_factory):
    """
    Tests that a BPlusSet is able to:
        1. be initialized from a non-empty iterable
        2. the `in` keyword works as expected for variables not in the set
    """

    control = get_randostrs(num=1000)

    s = bplusset_factory(control)

    check_contains(s, control, get_randostrs())

@parametrized_b
@parametrized_range
def test_range_initializer_with_collision_assert_contains(bplusset_factory, list_from_range):
    """
    Tests that a BPlusSet is able to:
        1. be initialized from a non-empty iterable that has a collision
        2. the `in` keyword works as expected
    """
    control = list_from_range + [sys.maxsize]

    s = bplusset_factory(control)

    check_contains(s, control, [3, sys.maxsize])

@parametrized_b
def test_range_initializer_with_large_number_of_collisions_assert_contains(bplusset_factory):
    """
    Tests that a BPlusSet is able to:
        1. be initialized from a non-empty iterable that has a large number of collisions
        2. the `in` keyword works as expected
    """
    # [hash(x) for x in range(1, 129)] == [hash(x) for x in range(sys.maxsize-2, sys.maxsize+127)]
    control = list(range(1, 129)) + list(range(sys.maxsize-2, sys.maxsize+127))

    s = bplusset_factory(control)

    check_contains(s, control, get_subset(control))

@parametrized_b
def test_range_initializer_with_large_number_of_collisions_assert_not_contains(bplusset_factory):
    """
    Tests that a BPlusSet is able to:
        1. be initialized from a non-empty iterable that has a large number of collisions
        2. the `in` keyword works as expected
    """
    # [hash(x) for x in range(1, 129)] == [hash(x) for x in range(sys.maxsize-2, sys.maxsize+127)]
    control = list(range(1, 129)) + list(range(sys.maxsize-2, sys.maxsize+127))

    s = bplusset_factory(control)

    check_contains(s, control, list(range(500, 555)))
