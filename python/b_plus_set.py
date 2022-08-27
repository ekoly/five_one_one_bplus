import five_one_one_bplus.c

class BPlusSet(five_one_one_bplus.c.BPlusTree):
    """
    {BPlusSet} is a class designed to be similar to the builtin {set} class.
    "Under the hood" it is implemented as a B Plus Tree in C.
    
    :param iterable: An iterable containing objects to add to the set. May be
        slightly faster than repeated calls to {BPlusSet::add()}.
    :param int b: the maximum number of child nodes per parent nodes in the
        underlying B Plus Tree. Defaults to 16. Must be between 2 and 255
        inclusive. For optimal performance, b<8 is not recommended.
    """

    def __init__(self, *args, b=16):
        if len(args) == 0:
            initializer = None
        elif len(args) == 1:
            initializer = args[0]
        else:
            raise TypeError(
                f"BPlusSet expects at most 1 argument, got {len(args)}.",
            )
        super().__init__(
            initializer=initializer,
            b=b,
        )
