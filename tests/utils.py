import pytest
import sys
import random
from collections import namedtuple

# globals
THOUSAND = 1_000
MILLION = THOUSAND*THOUSAND
BILLION = MILLION*THOUSAND
TRILLION = BILLION*THOUSAND

# used for range_params fixture
RangeParam = namedtuple("RangeParam", ("start", "stop", "step"))

# helper functions
def check_contains(test_s, control_s, vars_to_check):
    for x in vars_to_check:
        assert (x in test_s) == (x in control_s)

def get_subset(s, num=100):
    my_l = list(s)
    num = min(num, len(my_l))
    return [random.choice(my_l) for _ in range(num)]

def get_randints(num=100):
    return [random.randint(0, sys.maxsize) for _ in range(num)]

def randostr():
    return "".join([chr(random.randint(ord("A"), ord("Z"))) for _ in range(32)])

def get_randostrs(num=100):
    return [randostr() for _ in range(num)]

# parametrizeds
parametrized_b = pytest.mark.parametrize("b", [2, 8, 32, 128], indirect=True)
parametrized_range = pytest.mark.parametrize(
    "range_params",
    (
        [RangeParam(0, 1<<x, 1) for x in range(3, 12, 2)]
        + [RangeParam(1<<x, 0, -1) for x in range(3, 12, 2)]
        + [RangeParam(TRILLION, TRILLION+(1<<x), 1) for x in range(3, 12, 2)]
        + [RangeParam(TRILLION+(1<<x), TRILLION, -1) for x in range(3, 12, 2)]
    ),
    indirect=True
)
