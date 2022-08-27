import pytest
import random
import sys
from collections import namedtuple

from five_one_one_bplus import BPlusSet

# fixtures
@pytest.fixture(scope="function")
def b(request):
    return request.param

@pytest.fixture(scope="function")
def range_params(request):
    return range(request.param.start, request.param.stop, request.param.step)

@pytest.fixture(scope="function")
def bplusset_empty(b):
    return BPlusSet(b=b)

@pytest.fixture(scope="function")
def bplusset_factory(b):
    return lambda initializer: BPlusSet(initializer, b=b)

@pytest.fixture(scope="function")
def set_from_range(range_params):
    return set(range_params)

@pytest.fixture(scope="function")
def list_from_range(range_params):
    return list(range_params)
