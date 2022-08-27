import pytest

from five_one_one_bplus import BPlusSet

from tests.utils import parametrized_b

# basic tests!
# no docstrings on these because they should be pretty self explanatory
@parametrized_b
def test_basic(bplusset_empty):
    assert isinstance(bplusset_empty, BPlusSet)

@parametrized_b
def test_basic_initializer_none(bplusset_factory):
    res = bplusset_factory(None)
    assert isinstance(res, BPlusSet)

@parametrized_b
def test_basic_initializer_empty_list(bplusset_factory):
    res = bplusset_factory([])
    assert isinstance(res, BPlusSet)

@parametrized_b
def test_basic_assert_len(bplusset_empty):
    assert len(bplusset_empty) == 0

@parametrized_b
def test_basic_initializer_none_assert_len(bplusset_factory):
    res = bplusset_factory(None)
    assert len(res) == 0

@parametrized_b
def test_basic_initializer_empty_list_assert_len(bplusset_factory):
    res = bplusset_factory([])
    assert len(res) == 0

@parametrized_b
def test_basic_assert_not_contains(bplusset_empty):
    assert "foo" not in bplusset_empty

@parametrized_b
def test_basic_initializer_none_assert_not_contains(bplusset_factory):
    res = bplusset_factory(None)
    assert "foo" not in res

@parametrized_b
def test_basic_initializer_empty_list_assert_not_contains(bplusset_factory):
    res = bplusset_factory([])
    assert "foo" not in res
