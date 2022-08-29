#include <Python.h>
#include "structmember.h"
#include "bplustypes.h"
#include "general.h"
#include "array32.h"
#include "bplusnode.h"
#include "bplustree.h"


// define our subslot for BPlusTree sequence methods
static PySequenceMethods BPlusTree_sq_methods = {
    (lenfunc)BPlusTree_sq_length,               /*sq_length*/
    0,                                          /*sq_concat*/
    0,                                          /*sq_repeat*/
    0,                                          /*sq_item*/
    0,                                          /*was_sq_slice*/
    0,                                          /*sq_ass_item (???)*/
    0,                                          /*was_sq_ass_slice (???)*/
    BPlusTree_sq_contains,                      /*sq_contains*/
    0,                                          /*sq_inplace_concat*/
    0,                                          /*sq_inplace_repeat*/
};


// define our subslot for BPlusTree public methods
static PyMethodDef BPlusTree_tp_methods[] = { 
    {"get_b", BPlusTree_method_get_b, METH_NOARGS, "Return the maximum child nodes per node in the tree."},
    {"add", BPlusTree_method_add, METH_VARARGS, "Takes object {o}, computes the hash, and inserts {o} into the tree with index of the hash."},
    {"get_indices", BPlusTree_method_get_indices, METH_NOARGS, "Traverses the tree and returns a list of lists, where each 2nd level list represents a node, and each item is an index."},
    {NULL, NULL, 0, NULL}
};


// define our BPlusTreeType type object
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


// BEGIN tp method definitions
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

        Py_DECREF(debug);
        Py_DECREF(msg1);
        Py_DECREF(msg2);
        
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


static PyObject *BPlusTree_tp_richcompare(PyObject *o1, PyObject *o2, int op) {
    
    int is_bplustree;

    if ((is_bplustree = PyObject_IsInstance(o2, (PyObject *)&BPlusTreeType)) == -1) {
        // got error calling isinstance
        return NULL;
    } else if (is_bplustree == 0) {
        // {o2} is not a BPlusTree or a subtype of BPlusTree
        if (op == Py_EQ) {
            Py_RETURN_FALSE;
        } else if (op == Py_NE) {
            Py_RETURN_TRUE;
        }
        Py_RETURN_NOTIMPLEMENTED;
    }

    // if we get here, {o1} and {o2} are both BPlusTrees or subtypes

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
    PyObject
        *container_list = PyList_New(tree->size),
        *iterator,
        *value,
        *subvalue;
    int
        ix = 0,
        res;

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

            value = ((PyObject **)current->values->arr)[jx];
            res = PyObject_IsInstance(value, (PyObject *)&PyList_Type);

            if (res == -1) {
                // error determining if {value} is a list
                Py_DECREF(container_list);
                return NULL;
            } else if (res == 0) {
                // {value} is not a list, handle it normally
                // update reference to item because PyList_SetItem steals a reference
                Py_INCREF(value);
                if (PyList_SetItem(container_list, ix, value) == -1) {
                    Py_DECREF(container_list);
                    return NULL;
                };
                ix++;
            } else {
                // {value} is a list representing objects with hash collisions
                // add each element of it to our container
                for (int kx = 0; kx < PyList_Size(value); kx++) {
                    subvalue = PyList_GetItem(value, kx);
                    // update reference to item because PyList_SetItem steals a reference
                    Py_INCREF(subvalue);
                    if (PyList_SetItem(container_list, ix, subvalue) == -1) {
                        Py_DECREF(container_list);
                        return NULL;
                    };
                    ix++;
                }
            }
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


// BEGIN sequence methods
// this is called on call to len()
Py_ssize_t BPlusTree_sq_length(PyObject *self) {
    return ((BPlusTree *)self)->size;
}


// this is called on use of the `in` keyword
int BPlusTree_sq_contains(PyObject *self, PyObject *value) {

    BPlusTree *tree = (BPlusTree *)self;
    l64 key;
    BPlusNode *leaf = NULL;
    int ix;

    key = PyObject_Hash(value);

    leaf = BPlusNode_search(tree->root, key);

    ix = BPlusLeaf_search(leaf, key, value);

    // see comments above BPlusLeaf_search for info about what -1 means
    if (ix >= 0) {
        // {value} is in the tree
        return 1;
    } else if (ix == -2) {
        // there was an error in the search operation
        return -1;
    }
 
    return 0;

}


// BEGIN public method definitions
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

// BEGIN helper function definitions

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
    int res;

    leaf = BPlusNode_search(tree->root, key);

    res = BPlusLeaf_insert(leaf, key, o);

    if (res == -1) {
        // an error occurred in the insert operation
        // an error should already be set
        return NULL;
    } else if (res == 0) {
        // {o} was already in the list, do nothing
        Py_RETURN_NONE;
    }

    if (leaf->values->size > tree->b) {
        BPlusLeaf_split(tree, leaf);
    }

    tree->size++;
    Py_RETURN_NONE;

}

// comparison helper function
int BPlusTree_cmp(BPlusTree *tree1, BPlusTree *tree2) {

    BPlusNode 
        *curr1 = tree1->root,
        *curr2 = tree2->root;
    l64
        item1,
        item2;
    PyObject
        *value1,
        *value2;
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

        value1 = ((PyObject **)curr1->values->arr)[ix1];
        value2 = ((PyObject **)curr2->values->arr)[ix2];

        if (PyObject_RichCompareBool(value1, value2, Py_EQ) != 1) {
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

// define our module methods
// currently no module methods
static PyMethodDef bplus_method_def[] = {
    {NULL, NULL, 0, NULL}
};
// define our module
static struct PyModuleDef bplus_module_def = {
    PyModuleDef_HEAD_INIT,
    "c",
    "b-plus module implemented in C.",
    -1,
    bplus_method_def,
};

// register our module and add the BPlusTree type to it
PyMODINIT_FUNC PyInit_c(void) {
    PyObject *bplus = PyModule_Create(&bplus_module_def);
    PyModule_AddType(bplus, &BPlusTreeType);
    return bplus;
}
