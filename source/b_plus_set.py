import five_one_one_bplus.c

class BPlusSet(object):
    """
    {BPlusSet} is a class designed to be similar to the builtin {set} class.
    "Under the hood" it is implemented as a B Plus Tree in C.
    
    :param iterable: An iterable containing objects to add to the set. May be
        slightly faster than repeated calls to {BPlusSet::add()}.
    :param int b: the maximum number of child nodes per parent nodes in the
        underlying B Plus Tree. Defaults to 8. Must be between 2 and 255
        inclusive. For optimal performance, b<8 is not recommended.
    """

    def __init__(self, *args, b=8):
        if len(args) == 0:
            initializer = None
        elif len(args) == 1:
            initializer = args[0]
        else:
            raise TypeError(
                f"BPlusSet expects at most 1 argument, got {len(args)}.",
            )
        self._tree = five_one_one_bplus.c.BPlusTree(
            initializer=initializer,
            b=b,
        )

    def __contains__(self, o):
        """
        Dunder method for supporting the `in` keyword.
        """
        return self._tree.contains(hash(o), o)

    def __len__(self):
        """
        Returns the total number of items in the set.
        """
        return self._tree.get_size()

    @property
    def b(self):
        """
        Returns the max number of child nodes per parent node in the underlying
        B Plus Tree.
        """
        return self._tree.get_b()

    def add(self, o):
        """
        Add object {o} to the set, where {o} is a hashable object.

        Has no effect if {o} is already in the set.

        On the 1 in 2**64 chance of a hash collision, raises a RuntimeError.
        This is planned to be fixed in a future version.
        """
        return self._tree.insert(hash(o), o)

    def get_indices(self):
        """
        INTENDED FOR DEBUGGING PURPOSES ONLY

        Returns a list of lists of long long ints.
         * Each second-level list represents a BPlusNode in the tree
         * Each integer value is an index in the corresponding BPlusNode.
         * The first second-level list is the root node.
         * The next len(first_list)+1 lists are the second-level nodes.
         * ... and so on
        """
        return self._tree.get_indices()
