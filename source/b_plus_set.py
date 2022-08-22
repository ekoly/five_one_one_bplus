import five_one_one_bplus.c

class BPlusSet(object):

    def __init__(self, b=8):
        self._tree = five_one_one_bplus.c.BPlusTree(b=b)

    def __contains__(self, o):
        return self._tree.contains(hash(o), o)

    def __len__(self):
        return self._tree.get_size()

    @property
    def b(self):
        return self._tree.get_b()

    def add(self, o):
        return self._tree.insert(hash(o), o)
