#include <Python.h>
#include "structmember.h"
#include <stdio.h>


#define DEBUG 0

static PyObject
    *_builtins = NULL,
    *_iter = NULL,
    *_next = NULL,
    *_hash = NULL,
    *_print = NULL;

typedef long long int l64;

typedef struct Array32 {
    void *arr;
    int size;
} Array32;

typedef struct BPlusNode {
    Array32 *indices;
    Array32 *values;
    Array32 *children;
    struct BPlusNode *parent;
    struct BPlusNode *prev;
    struct BPlusNode *next;
} BPlusNode;

// sets builtin functions.
// returns 1 on success, 0 otherwise.
int setBuiltins() {
    
    if (_builtins == NULL) {
        _builtins = PyEval_GetBuiltins();
        if (_builtins == NULL) {
            return 0;
        }
        _iter = PyDict_GetItemString(_builtins, "iter");
        if (_iter == NULL) {
            return 0;
        }
        _next = PyDict_GetItemString(_builtins, "next");
        if (_next == NULL) {
            return 0;
        }
        _hash = PyDict_GetItemString(_builtins, "hash");
        if (_hash == NULL) {
            return 0;
        }
        _print = PyDict_GetItemString(_builtins, "print");
        if (_print == NULL) {
            return 0;
        }
    }
    
    return 1;

}

// debugging function for printing reference count.
void printRefCount(char *label, PyObject *x) {

    PyObject
        *py_label = PyUnicode_FromString(label),
        *py_refcnt = PyLong_FromLong(x->ob_refcnt);

    setBuiltins();

    PyObject_CallFunctionObjArgs(
        _print,
        py_label,
        x,
        py_refcnt,
        NULL
    );

    Py_DECREF(py_label);
    Py_DECREF(py_refcnt);

}

int bisect_left(Array32 *a, l64 x) {

    int
        lo = 0,
        hi = a->size,
        mid;

    while (lo < hi) {
        mid = (lo + hi) / 2;
        if (((l64 *)a->arr)[mid] < x) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }

    return lo;

}

int bisect_right(Array32 *a, l64 x) {

    int
        lo = 0,
        hi = a->size,
        mid;

    while (lo < hi) {
        mid = (lo + hi) / 2;
        if (x < ((l64 *)a->arr)[mid]) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }

    return lo;

}

int insert_l64(Array32 *a, int index, l64 x) {
    l64 *arr = (l64 *)a->arr;
    for (int i = a->size - 1; i >= index; i--) {
        arr[i+1] = arr[i];
    }
    arr[index] = x;
    a->size++;

    return 1;
}

int insert_PyObject(Array32 *a, int index, PyObject *x) {
    PyObject **arr = (PyObject **)a->arr;
    for (int i = a->size - 1; i >= index; i--) {
        arr[i+1] = arr[i];
    }
    arr[index] = x;
    a->size++;
    
    #if DEBUG >= 2
    printRefCount("insert_PyObject(): x before Py_INCREF:", x);
    #endif

    Py_INCREF(x);

    #if DEBUG >= 2
    printRefCount("insert_PyObject(): x after Py_INCREF:", x);
    #endif

    return 1;
}

int insert_BPlusNode(Array32 *a, int index, BPlusNode *x) {
    BPlusNode **arr = (BPlusNode **)a->arr;
    for (int i = a->size - 1; i >= index; i--) {
        arr[i+1] = arr[i];
    }
    arr[index] = x;
    a->size++;

    return 1;
}

int insort(Array32 *a, l64 x) {
    int ix = bisect_left(a, x);
    return insert_l64(a, ix, x);
}

// basic BPlusNode constructor for common elements
BPlusNode *BPlusNode_init(int b) {
    // construct the node
    BPlusNode *node = (BPlusNode *)malloc(sizeof(BPlusNode));

    // construct the indices
    node->indices = (Array32 *)malloc(sizeof(Array32));
    node->indices->size = 0;
    node->indices->arr = (l64 *)malloc(sizeof(l64) * (b+1));

    // initialize everything else to 0
    node->values = NULL;
    node->children = NULL;
    node->parent = NULL;
    node->prev = NULL;
    node->next = NULL;

    return node;
}

// Leaf Constructor
// construct a node that will have pointers to PyObjects
BPlusNode *BPlusLeaf_init(int b) {
    // construct the node
    BPlusNode *node = BPlusNode_init(b);
    node->values = (Array32 *)malloc(sizeof(Array32));
    node->values->size = 0;
    node->values->arr = (PyObject **)malloc(sizeof(PyObject *) * (b+1));

    return node;
}

// Branch Constructor
// construct a node that will have leafs or other nodes beneath it
BPlusNode *BPlusBranch_init(int b) {
    // construct the node
    BPlusNode *node = BPlusNode_init(b);
    node->children = (Array32 *)malloc(sizeof(Array32));
    node->children->size = 0;
    node->children->arr = (BPlusNode **)malloc(sizeof(BPlusNode *) * (b+1));

    return node;
}

void BPlusNode_dealloc(BPlusNode *node) {

    int sz, i;
    PyObject **values_arr = NULL;
    BPlusNode **children_arr = NULL;

    if (node->children != NULL) {
        sz = node->children->size;
        children_arr = (BPlusNode **)node->children->arr;
        for (i = 0; i < sz; i++) {
            BPlusNode_dealloc(children_arr[i]);
        }
        free(node->children->arr);
        free(node->children);
    }

    if (node->values != NULL) {
        sz = node->values->size;
        values_arr = (PyObject **)node->values->arr;
        for (i = 0; i < sz; i++) {
            Py_DECREF(values_arr[i]);
        }
        free(node->values->arr);
        free(node->values);
    }
    
    free(node->indices->arr);
    free(node->indices);

    free(node);

}

// helper function that takes root node {root} and index value {key}, and returns
// a pointer to the leaf node that either:
//  1. contains {key}
//  2. would contain {key} if it existed in the tree
BPlusNode *BPlusNode_search(BPlusNode *root, l64 key) {

    BPlusNode *current = root;
    int ix;

    while (current->children != NULL) {
        ix = bisect_right(current->indices, key);
        current = ((BPlusNode **)current->children->arr)[ix];
    }

    return current;

}

// helper function for determining if {leaf} contains the index {key} and
// PyObject {o} combination.
// Returns either:
//  1. If {key}/{o} are not in {leaf}, an integer value {ix} representing the
//      index at which they should be inserted.
//  2. If {key}/{o} are in {leaf}, returns -1 indicating {o} is already
//      contained.
//  3. If {key} is in leaf but not {o}, returns -2 indicating a collision.
int BPlusLeaf_search(BPlusNode *leaf, l64 key, PyObject *o) {

    int ix = bisect_left(leaf->indices, key);

    if (ix >= leaf->indices->size) {
        return ix;
    }

    if (((l64 *)leaf->indices->arr)[ix] != key) {
        return ix;
    }

    if (PyObject_RichCompareBool(o, ((PyObject **)leaf->values->arr)[ix], Py_EQ) == 1) {
        // {o} is already contained in the BPlusTree
        return -1;
    }

    // We have a collision!
    // TODO figure out a better way of handling this
    return -2;
 
}

// start defining our python type
typedef struct BPlusTree {
    PyObject_HEAD
    BPlusNode *root;
    int b;
    int size;
    BPlusNode *iter_current_node;
    int iter_current_index;
} BPlusTree;

// BEGIN helper functions

// helper function for splitting a saturated branch into 2 branches
// expects that {branch} has {self->b}+1 children
void BPlusBranch_split(BPlusTree *self, BPlusNode *branch) {

    BPlusNode *left, *right;
    // children size, children mid, indices size, indices mid
    int csize, cmid, isize, imid, ix;
    l64 new_parent_ix;

    // initialize our 2 new leaves
    left = BPlusBranch_init(self->b);
    right = BPlusBranch_init(self->b);

    csize = branch->children->size;
    cmid = csize / 2;
    isize = branch->indices->size;
    imid = cmid - 1;

    // copy half of {branch->children} each to left and right respectively
    memcpy(left->children->arr, branch->children->arr, sizeof(PyObject *)*cmid);
    left->children->size = cmid;

    memcpy(right->children->arr, ((BPlusNode **)branch->children->arr)+cmid, sizeof(BPlusNode *) * (csize - cmid));
    right->children->size = csize - cmid;

    // copy half of {branch->indices} each to left and right respectively
    memcpy(left->indices->arr, branch->indices->arr, sizeof(l64)*imid);
    left->indices->size = imid;

    memcpy(right->indices->arr, ((l64 *)branch->indices->arr)+imid+1, sizeof(l64) * (isize - imid - 1));
    right->indices->size = isize - imid - 1;

    // set parent of children
    for (ix = 0; ix < left->children->size; ix++) {
        ((BPlusNode **)left->children->arr)[ix]->parent = left;
    }
    for (ix = 0; ix < right->children->size; ix++) {
        ((BPlusNode **)right->children->arr)[ix]->parent = right;
    }

    // set parent of left and right
    if (branch == self->root) {
        self->root = BPlusBranch_init(self->b);
        left->parent = self->root;
        right->parent = self->root;
    } else {
        left->parent = branch->parent;
        right->parent = branch->parent;
    }

    // reset attributes of parent
    new_parent_ix = ((l64 *)branch->indices->arr)[imid];
    ix = bisect_left(left->parent->indices, new_parent_ix);
    if (left->parent->children->size == 0) {
        // parent is a new node, use insert() to handle size
        insert_BPlusNode(left->parent->children, 0, right);
        insert_BPlusNode(left->parent->children, 0, left);
    } else {
        // parent is an existing node
        ((BPlusNode **)left->parent->children->arr)[ix] = right;
        insert_BPlusNode(left->parent->children, ix, left);
    }
    insert_l64(left->parent->indices, ix, new_parent_ix);

    if (left->parent->children->size > self->b) {
        BPlusBranch_split(self, left->parent);
    }

    // free branch indices
    free(branch->indices->arr);
    free(branch->indices);

    // free branch children
    free(branch->children->arr);
    free(branch->children);

    // free branch
    free(branch);

}

// helper function for splitting a saturated leaf into 2 leaves
// expects that {leaf} has {self->b}+1 values
void BPlusLeaf_split(BPlusTree *self, BPlusNode *leaf) {

    BPlusNode *left, *right;
    // values size, values mid, indices size, indices mid
    int vsize, vmid, isize, imid, ix;
    l64 new_parent_ix;

    // initialize our 2 new leaves
    left = BPlusLeaf_init(self->b);
    right = BPlusLeaf_init(self->b);

    vsize = leaf->values->size;
    vmid = vsize / 2;
    isize = leaf->indices->size;
    imid = vmid;

    // copy half of {leaf->values} each to left and right respectively
    memcpy(left->values->arr, leaf->values->arr, sizeof(PyObject *)*vmid);
    left->values->size = vmid;

    memcpy(right->values->arr, ((PyObject **)leaf->values->arr)+vmid, sizeof(PyObject *) * (vsize - vmid));
    right->values->size = vsize - vmid;

    // copy half of {leaf->indices} each to left and right respectively
    memcpy(left->indices->arr, leaf->indices->arr, sizeof(l64)*imid);
    left->indices->size = imid;

    memcpy(right->indices->arr, ((l64 *)leaf->indices->arr)+imid, sizeof(l64) * (isize - imid));
    right->indices->size = isize - imid;

    // set parents
    left->parent = leaf->parent;
    right->parent = leaf->parent;

    // set {next} of all relevant nodes
    if (leaf->prev != NULL) {
        leaf->prev->next = left;
    }
    left->next = right;
    right->next = leaf->next;

    // set {prev} of all relevant nodes
    if (leaf->next != NULL) {
        leaf->next->prev = right;
    }
    right->prev = left;
    left->prev = leaf->prev;

    // reset attributes of parent
    new_parent_ix = ((l64 *)right->indices->arr)[0];
    ix = bisect_left(leaf->parent->indices, new_parent_ix);
    ((BPlusNode **)leaf->parent->children->arr)[ix] = right;
    insert_BPlusNode(leaf->parent->children, ix, left);
    insert_l64(leaf->parent->indices, ix, new_parent_ix);

    if (leaf->parent->children->size > self->b) {
        BPlusBranch_split(self, leaf->parent);
    }

    // free leaf indices
    free(leaf->indices->arr);
    free(leaf->indices);

    // free leaf values
    free(leaf->values->arr);
    free(leaf->values);

    // free leaf
    free(leaf);

}

PyObject *BPlusTree_insert(BPlusTree *tree, l64 key, PyObject *o) {

    BPlusNode *leaf = NULL;
    int ix;

    leaf = BPlusNode_search(tree->root, key);

    ix = BPlusLeaf_search(leaf, key, o);

    if (ix == -1) {
        // The object already exists in the tree.
        // Do nothing.
        Py_RETURN_NONE;
    } else if (ix == -2) {
        // We have a collision!
        // TODO figure out a better way of handling this
        Py_INCREF(PyExc_RuntimeError);
        PyErr_SetString(PyExc_RuntimeError, "Collision!");
        return NULL;
    }

    insert_l64(leaf->indices, ix, key);
    insert_PyObject(leaf->values, ix, o);

    if (leaf->values->size > tree->b) {
        BPlusLeaf_split(tree, leaf);
    }

    tree->size++;
    Py_RETURN_NONE;

}

// comparison helper function
// currently does not check for hash collisions!
int BPlusTree_cmp(BPlusTree *tree1, BPlusTree *tree2) {

    BPlusNode 
        *curr1 = tree1->root,
        *curr2 = tree2->root;
    l64
        item1,
        item2;
    int 
        ix1 = 0,
        ix2 = 0;

    if (tree1->size != tree2->size) {
        return 0;
    }

    while (curr1->children != NULL) curr1 = ((BPlusNode **)curr1->children->arr)[0];
    while (curr2->children != NULL) curr2 = ((BPlusNode **)curr2->children->arr)[0];

    // breaking is done during the loop
    while (1) {

        if (ix1 >= curr1->indices->size) {
            curr1 = curr1->next;
            ix1 = 0;
        }
        if (ix2 >= curr2->indices->size) {
            curr2 = curr2->next;
            ix2 = 0;
        }

        if (curr1 == NULL || curr2 == NULL) {
            break;
        }

        item1 = ((l64 *)curr1->indices->arr)[ix1];
        item2 = ((l64 *)curr2->indices->arr)[ix2];

        if (item1 != item2) {
            return 0;
        }

        ix1++;
        ix2++;

    }

    if (curr1 != NULL || curr2 != NULL) {
        return 0;
    }

    return 1;

}


// BEGIN tp methods
static PyObject *BPlusTree_tp_new(PyTypeObject *subtype, PyObject *args, PyObject *kwargs) {
    BPlusTree *self;

    self = (BPlusTree *)subtype->tp_alloc(subtype, 0);

    return (PyObject *)self;
}

static void BPlusTree_tp_clear(BPlusTree *self) {
    // TODO
    return;
}

static void BPlusTree_tp_dealloc(BPlusTree *self) {
    BPlusTree_tp_clear(self);
    BPlusNode_dealloc(self->root);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int BPlusTree_tp_init(BPlusTree *self, PyObject *args, PyObject *kwargs) {

    // set _builtins, _hash, _iter, _next, etc (builtin methods)
    if (setBuiltins() == 0) {
        
        Py_INCREF(PyExc_RuntimeError);
        PyErr_SetString(PyExc_RuntimeError, "Got error setting builtins.");

        return -1;

    }

    #if DEBUG >= 1

    PyObject
        *debug = PyLong_FromLong(DEBUG),
        *msg1 = PyUnicode_FromString("WARNING: this module was compiled in debug mode:"),
        *msg2 = PyUnicode_FromString("; there will be extraneous print statements.");

    if (PyObject_CallFunctionObjArgs(_print, msg1, debug, msg2, NULL) == NULL) {
        
        Py_INCREF(PyExc_RuntimeError);
        PyErr_SetString(PyExc_RuntimeError, "Got error calling print statement.");

        return -1;

    }

    Py_DECREF(debug);
    Py_DECREF(msg1);
    Py_DECREF(msg2);

    #endif

    int b;
    PyObject *initializer;
    static char *kwlist[] = {"initializer", "b", NULL};
    BPlusNode *firstleaf;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Oi", kwlist, &initializer, &b)) {
        return -1;
    }

    #if DEBUG >= 2
    printRefCount("constructor: object directly after argparse:", initializer);
    #endif

    if (b < 2 || 255 < b) {
        Py_INCREF(PyExc_TypeError);
        PyErr_SetString(PyExc_TypeError, "BPlusTree Constructor got out of bounds b: needs to be in [2, 255].");
        return -1;
    }

    self->root = BPlusBranch_init(b);

    firstleaf = BPlusLeaf_init(b);
    firstleaf->parent = self->root;

    insert_BPlusNode(self->root->children, 0, firstleaf);

    self->b = b;

    if (initializer == Py_None) {

        // no need to decref because ref count is not increased by call to
        // constructor
        return 0;

    }

    PyObject
        *iterable = PyObject_GetIter(initializer),
        *current_object,
        *insert_result;
    l64 hash;

    // we're done with our initializer
    // again, no need to decref because ref count is not increased by call to
    // constructor

    #if DEBUG >= 2
    printRefCount("constructor: iterable directly after creation:", iterable);
    #endif

    while ((current_object = PyIter_Next(iterable)) != NULL) {
        
        // calculate hash
        if ((hash = PyObject_Hash(current_object)) == -1) {

            Py_DECREF(iterable);

            BPlusTree_tp_dealloc(self);

            return -1;

        }

        // call self.insert(hash(o), o)
        if ((insert_result = BPlusTree_insert(self, hash, current_object)) == NULL) {

            Py_DECREF(iterable);

            BPlusTree_tp_dealloc(self);

            return -1;

        }

        #if DEBUG >= 2
        printRefCount("constructor: insert_result directly after creation", insert_result);
        #endif

        Py_DECREF(insert_result);

        #if DEBUG >= 2
        printRefCount("constructor: insert_result directly after decref", insert_result);
        #endif

    }

    // clear stop iteration exception
    PyErr_Clear();

    #if DEBUG >= 2
    printRefCount("constructor: iterable directly before call to decref:", iterable);
    #endif

    Py_DECREF(iterable);

    #if DEBUG >= 2
    printRefCount("constructor: iterable directly before returning:", iterable);
    #endif

    return 0;

}

// Currently does not check for hash collisions!
static PyObject *BPlusTree_tp_richcompare(PyObject *o1, PyObject *o2, int op) {

    if (op == Py_EQ) {
        if (BPlusTree_cmp((BPlusTree *)o1, (BPlusTree *)o2) == 1) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    } else if (op == Py_NE) {
        if (BPlusTree_cmp((BPlusTree *)o1, (BPlusTree *)o2) == 0) {
            Py_RETURN_TRUE;
        } else {
            Py_RETURN_FALSE;
        }
    }

    Py_RETURN_NOTIMPLEMENTED;

}

// return a {list_iterator} of the items in the tree
// There are several reasons for this being implemented this way:
//  1. to support safely modifying the tree during iteration
//  2. to avoid needing to store information such as {current BPlusNode} and/or
//      {current index} in the tree (which would be needed to support native
//      iteration).
//  3. to avoid needing to solve the problem of what to do when the user uses
//      a {break} statement during iteration.
static PyObject *BPlusTree_tp_iter(PyObject *self) {

    BPlusTree *tree = (BPlusTree *)self;
    BPlusNode *current = tree->root;
    PyObject *container_list = PyList_New(tree->size),
             *iterator;
    int ix = 0;

    #if DEBUG >= 2
    printRefCount("BPlusTree_tp_iter: container_list directly after creation:", container_list);
    #endif

    if (setBuiltins() == 0) {

        Py_DECREF(container_list);
        Py_INCREF(PyExc_RuntimeError);
        PyErr_SetString(PyExc_RuntimeError, "Error setting builtins.");

        return NULL;

    }

    while (current->children != NULL) current = ((BPlusNode **)current->children->arr)[0];

    while (current != NULL) {
        for (int jx = 0; jx < current->values->size; jx++) {
            // update reference to item because PyList_SetItem steals a reference
            Py_INCREF(((PyObject **)current->values->arr)[jx]);
            if (PyList_SetItem(container_list, ix, ((PyObject **)current->values->arr)[jx]) == -1) {
                Py_DECREF(container_list);
                return NULL;
            };
            ix++;
        }
        current = current->next;
    }

    iterator = PyObject_GetIter(container_list);
    Py_DECREF(container_list);

    if (iterator == NULL) {

        Py_INCREF(PyExc_RuntimeError);
        PyErr_SetString(PyExc_RuntimeError, "Error calling iter() on list object.");

        return NULL;

    }

    #if DEBUG >= 2
    printRefCount("BPlusTree_tp_iter: container_list directly after decref:", container_list);
    printRefCount("BPlusTree_tp_iter: iterator directly after creation:", iterator);
    #endif

    return iterator;

}

Py_ssize_t BPlusTree_sq_length(PyObject *self) {
    return ((BPlusTree *)self)->size;
}

int BPlusTree_sq_contains(PyObject *self, PyObject *value) {

    BPlusTree *tree = (BPlusTree *)self;
    l64 key;
    BPlusNode *leaf = NULL;
    int ix;

    key = PyObject_Hash(value);

    leaf = BPlusNode_search(tree->root, key);

    ix = BPlusLeaf_search(leaf, key, value);

    // see comments above BPlusLeaf_search for info about what -1 means
    if (ix == -1) {
        return 1;
    }
 
    return 0;

}

// BEGIN object methods
static PyObject *BPlusTree_method_get_b(PyObject *self, PyObject *args) {
    return PyLong_FromLong(((BPlusTree *)self)->b);
}

static PyObject *BPlusTree_method_add(PyObject *self, PyObject *args) {

    BPlusTree *tree = (BPlusTree *)self;
    l64 key;
    PyObject *o;

    if (!PyArg_ParseTuple(args, "O", &o)) {
        return NULL;
    }

    if ((key = PyObject_Hash(o)) == -1) {
        Py_INCREF(PyExc_TypeError);
        PyErr_SetString(PyExc_TypeError, "Got unhashable object.");
        return NULL;
    }

    return BPlusTree_insert(tree, key, o);

}

// following is for debugging purposes and is not expected to be useful generally.
static PyObject *BPlusTree_method_get_indices(PyObject *self, PyObject *args) {

    BPlusTree *tree = (BPlusTree *)self;
    // stack is to be the top level list, indices is the current second level list
    PyObject *stack, *indices;
    // ix is the index of the current BPlusNode, max_ix is the number of BPlusNodes in {nodes}
    int ix, max_ix;
    // nodes is eventually going to contain pointers to every node in the tree
    BPlusNode *nodes[1000];
    // current is a pointer to the current node
    BPlusNode *current;

    stack = PyList_New(0);
    ix = 0;
    max_ix = 1;
    nodes[0] = tree->root;
    
    while (ix < max_ix) {

        // build the list of indices
        indices = PyList_New(0);
        current = nodes[ix];
        for (int jx = 0; jx < current->indices->size; jx++) {
            PyList_Append(indices, PyLong_FromLongLong(((l64 *)current->indices->arr)[jx]));
        }
        PyList_Append(stack, indices);
        Py_DECREF(indices);

        // update nodes
        if (current->children != NULL) {
            for (int jx = 0; jx < current->children->size; jx++) {
                nodes[max_ix] = ((BPlusNode **)current->children->arr)[jx];
                max_ix++;
            }
        }

        ix++;

    }

    return stack;

}

static PySequenceMethods BPlusTree_sq_methods = {
    (lenfunc)BPlusTree_sq_length,               /*sq_length*/
    0,                                          /*sq_concat*/
    0,                                          /*sq_repeat*/
    0,                                          /*sq_item*/
    0,                                          /*was_sq_slice*/
    0,                                          /*sq_ass_item*/
    0,                                          /*was_sq_ass_slice*/
    BPlusTree_sq_contains,                      /*sq_contains*/
    0,                                          /*sq_inplace_concat*/
    0,                                          /*sq_inplace_repeat*/
};

static PyMethodDef BPlusTree_tp_methods[] = { 
    {"get_b", BPlusTree_method_get_b, METH_NOARGS, "Return the maximum child nodes per node in the tree."},
    {"add", BPlusTree_method_add, METH_VARARGS, "Takes object {o}, computes the hash, and inserts {o} into the tree with index of the hash."},
    {"get_indices", BPlusTree_method_get_indices, METH_NOARGS, "Traverses the tree and returns a list of lists, where each 2nd level list represents a node, and each item is an index."},
    {NULL, NULL, 0, NULL}
};

static PyTypeObject BPlusTreeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "five_one_one_bplus.c.BPlusTree",           /*tp_name*/
    sizeof(BPlusTree),                          /*tp_basicsize*/
    0,                                          /*tp_itemsize*/
    (destructor)BPlusTree_tp_dealloc,           /*tp_dealloc*/
    0,                                          /*tp_print*/
    0,                                          /*tp_getattr*/
    0,                                          /*tp_setattr*/
    0,                                          /*tp_compare*/
    0,                                          /*tp_repr*/
    0,                                          /*tp_as_number*/
    &BPlusTree_sq_methods,                      /*tp_as_sequence*/
    0,                                          /*tp_as_mapping*/
    0,                                          /*tp_hash */
    0,                                          /*tp_call*/
    0,                                          /*tp_str*/
    0,                                          /*tp_getattro*/
    0,                                          /*tp_setattro*/
    0,                                          /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /*tp_flags*/
    0,                                          /*tp_doc*/
    0,                                          /*tp_traverse*/
    (inquiry)BPlusTree_tp_clear,                /*tp_clear*/
    BPlusTree_tp_richcompare,                   /*tp_richcompare*/
    0,                                          /*tp_weaklistoffset*/
    (getiterfunc)BPlusTree_tp_iter,             /*tp_iter*/
    0,                                          /*tp_iternext*/
    BPlusTree_tp_methods,                       /*tp_methods*/
    0,                                          /*tp_members*/
    0,                                          /*tp_getsets*/
    0,                                          /*tp_base*/
    0,                                          /*tp_dict*/
    0,                                          /*tp_descr_get*/
    0,                                          /*tp_descr_set*/
    0,                                          /*tp_dictoffset*/
    (initproc)BPlusTree_tp_init,                /*tp_init*/
    0,                                          /*tp_alloc*/
    BPlusTree_tp_new,                           /*tp_new*/
};

static PyMethodDef bplus_method_def[] = {
    {NULL, NULL, 0, NULL}
};
static struct PyModuleDef bplus_module_def = {
    PyModuleDef_HEAD_INIT,
    "c",
    "b-plus module implemented in C.",
    -1,
    bplus_method_def,
};

PyMODINIT_FUNC PyInit_c(void) {
    PyObject *bplus = PyModule_Create(&bplus_module_def);
    PyModule_AddType(bplus, &BPlusTreeType);
    return bplus;
}
