#! /usr/bin/env python3

import re
import numpy as np
from bitstring import Bits

LINE_RE = re.compile(r"([0-9]+),([0-9]+)")


def parse_line(l):
    m = LINE_RE.fullmatch(l)
    # print(repr(l), m)
    return int(m.group(1)), int(m.group(2))


def get_data(fn):
    with open(fn, "rt") as f:
        return [parse_line(l.strip()) for l in f if l]


data = get_data("/media/router/storage/out.csv")
print(len(data))

d = np.array(data)

def delta(d):
    return d[1:] - d[:-1]

from typing import Set

class Node:
    count: int
    value: int
    all_values: Set[int]

    def __gt__(self, other):
        return self.count > other.count or (self.count == other.count) and self.value > other.value

    def __lt__(self, other):
        return self.count < other.count or (self.count == other.count) and self.value < other.value

    def __eq__(self, other):
        return self.count == other.count or self.value == other.value

class OrderedValue(Node):
    __slots__ = ("value", "count")

    def __init__(self, value, count):
        self.value = value
        self.count = count

    @property
    def all_values(self):
        return {self.value}

    def __repr__(self):
        return "{}×{}".format(self.value, self.count)

import heapq

class CombinedNode(Node):
    __slots__ = ("zero", "one", "value", "count")

    def __init__(self, zero: Node, one: Node):
        self.zero = zero
        self.one = one
        self.count = zero.count + one.count
        self.value = zero.value + one.value
        self.all_values = zero.all_values | one.all_values

    def __repr__(self):
        return "[{!r},{!r}]×{}".format(self.zero, self.one, self.count)


def coding(dd, limit):
    if limit:
        others = np.arange(-limit, limit)
    else:
        others = np.empty(0)
    values = dd.flatten()
    # print(values.shape, others.shape)
    if limit:
        values = np.concatenate((values[abs(values) < limit], others, np.repeat(-limit, len(values[abs(values) >= limit]))))

    values, counts = np.unique(values, return_counts=True)
    order = np.argsort(counts)

    print(values[order], counts[order])

    pq = []

    for i in range(order.shape[0]):
        j = order[i]
        count = counts[j]
        v = OrderedValue(values[j], count)
        heapq.heappush(pq, v)


    while len(pq) > 1:
        a, b = heapq.heappop(pq), heapq.heappop(pq)
        # print(a, b)
        heapq.heappush(pq, CombinedNode(a, b))

    return pq[0]


def traverse(c: Node, code=Bits()):
    if isinstance(c, CombinedNode):
        traverse(c.zero, code + "0b0")
        traverse(c.one, code + "0b1")
    else:
        print("{: 8d}\t{: 8d}\t{}".format(c.value, c.count, code))


traverse(coding(np.array([0, 0, 0, 0, 0, 0, 0, 1, 32, 32, 32]), limit=None))
traverse(coding(delta(d[:, 1]), limit=16))
