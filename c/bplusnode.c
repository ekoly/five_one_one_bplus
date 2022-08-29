// BPlusNode helper method definitions.
#include <Python.h>
#include "bplustypes.h"
#include "general.h"
#include "array32.h"
#include "bplusnode.h"


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
// construct a node that will have pointers to PyObjects and no children
BPlusNode *BPlusLeaf_init(int b) {
    // construct the node
    BPlusNode *node = BPlusNode_init(b);
    node->values = (Array32 *)malloc(sizeof(Array32));
    node->values->size = 0;
    node->values->arr = (PyObject **)malloc(sizeof(PyObject *) * (b+1));

    return node;
}

// Branch Constructor
// construct a node that will have leaves or other nodes beneath it
BPlusNode *BPlusBranch_init(int b) {
    // construct the node
    BPlusNode *node = BPlusNode_init(b);
    node->children = (Array32 *)malloc(sizeof(Array32));
    node->children->size = 0;
    node->children->arr = (BPlusNode **)malloc(sizeof(BPlusNode *) * (b+1));

    return node;
}

// deallocate memory for {node} and its members
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
//  1. If {key}/{o} are not in {leaf}, -1
//  2. If {key}/{o} are in {leaf}, an integer value {ix} representing the
//      location of {o}
//  3. If there is an error, -2
int BPlusLeaf_search(BPlusNode *leaf, l64 key, PyObject *o) {

    int ix = bisect_left(leaf->indices, key);

    if (ix >= leaf->indices->size) {
        return -1;
    }

    if (((l64 *)leaf->indices->arr)[ix] != key) {
        return -1;
    }

    PyObject *val_ix = ((PyObject **)leaf->values->arr)[ix];

    if (PyObject_RichCompareBool(o, val_ix, Py_EQ) == 1) {
        // {o} is already contained in the BPlusTree
        return 1;
    }

    // We have a collision!
    // Current strategy for dealing with this is replace the object at {ix}
    // with a list, and add all items with the same hash to the list
    // This works because lists are an unhashable type and therefore cannot
    // be mistaken for an object intentionally in the tree
    int res = PyObject_IsInstance(val_ix, (PyObject *)&PyList_Type);
    PyObject *collision_container;

    if (res == -1) {
        // error determining if the value at {ix} is a list
        return -2;
    } else if (res == 0) {
        // not a list, therefore not a known collision and {o} is not in the
        // list
        return -1;
    }

    collision_container = val_ix;

    // iterate through the list and check if {o} is contained
    for (int jx = 0; jx < PyList_Size(collision_container); jx++) {
        val_ix = PyList_GetItem(collision_container, jx);
        if (PyObject_RichCompareBool(o, val_ix, Py_EQ) == 1) {
            return 1;
        }
    }

    // {o} is not in the list
    return 0;

}

// Helper method for inserting {key}/{o} into the leaf.
// Assumes {leaf} is the proper leaf within the tree to insert {o}.
// Returns one of the following:
//  1. If {o} was not already in the tree and the insert operation was
//      successful, returns 1
//  2. If {o} was already in the tree, does nothing and returns 0
//  3. On an error, returns -1
int BPlusLeaf_insert(BPlusNode *leaf, l64 key, PyObject *o) {

    int ix = bisect_left(leaf->indices, key);

    if (ix >= leaf->indices->size) {
        #if DEBUG >= 1
        PyObject *msg0 = PyUnicode_FromString("BPlusLeaf_insert: Appending object in leaf");
        if (PyObject_CallFunctionObjArgs(_print, msg0, o, NULL) == NULL) {
            Py_DECREF(msg0);
            Py_INCREF(PyExc_RuntimeError);
            PyErr_SetString(PyExc_RuntimeError, "Got error calling print statement.");
            return -1;
        }
        Py_DECREF(msg0);
        #endif
        insert_l64(leaf->indices, ix, key);
        insert_PyObject(leaf->values, ix, o);

        return 1;
    }

    if (((l64 *)leaf->indices->arr)[ix] != key) {
        #if DEBUG >= 1
        PyObject
            *msg1 = PyUnicode_FromString("BPlusLeaf_insert: inserting object in leaf"),
            *msg2 = PyUnicode_FromString("at index"),
            *pyobj_ix = PyLong_FromLong(ix);
        if (PyObject_CallFunctionObjArgs(_print, msg1, o, msg2, pyobj_ix, NULL) == NULL) {
            Py_DECREF(msg1);
            Py_DECREF(msg2);
            Py_DECREF(pyobj_ix);
            Py_INCREF(PyExc_RuntimeError);
            PyErr_SetString(PyExc_RuntimeError, "Got error calling print statement.");
            return -1;
        }
        Py_DECREF(msg1);
        Py_DECREF(msg2);
        Py_DECREF(pyobj_ix);
        #endif
        insert_l64(leaf->indices, ix, key);
        insert_PyObject(leaf->values, ix, o);

        return 1;
    }

    PyObject *val_ix = ((PyObject **)leaf->values->arr)[ix];

    if (PyObject_RichCompareBool(o, val_ix, Py_EQ) == 1) {
        // {o} is already contained in the BPlusTree
        #if DEBUG >= 1
        PyObject
            *msg4 = PyUnicode_FromString("BPlusLeaf_insert: object already exists in leaf");
        if (PyObject_CallFunctionObjArgs(_print, msg4, o, NULL) == NULL) {
            Py_DECREF(msg4);
            Py_INCREF(PyExc_RuntimeError);
            PyErr_SetString(PyExc_RuntimeError, "Got error calling print statement.");
            return -1;
        }
        Py_DECREF(msg4);
        #endif
        return 0;
    }

    // We have a collision!
    // Current strategy for dealing with this is replace the object at {ix}
    // with a list, and add all items with the same hash to the list
    // This works because lists are an unhashable type and therefore cannot
    // be mistaken for an object intentionally in the tree
    int res = PyObject_IsInstance(val_ix, (PyObject *)&PyList_Type);
    PyObject *collision_container;

    if (res == -1) {
        // error
        Py_INCREF(PyExc_RuntimeError);
        PyErr_SetString(PyExc_RuntimeError, "Got error checking if existing value is a list.");
        return -1;
    } else if (res == 0) {
        #if DEBUG >= 1
        PyObject
            *msg5 = PyUnicode_FromString("BPlusLeaf_insert: got collision: creating new collision container");
        if (PyObject_CallFunctionObjArgs(_print, msg5, NULL) == NULL) {
            Py_DECREF(msg5);
            Py_INCREF(PyExc_RuntimeError);
            PyErr_SetString(PyExc_RuntimeError, "Got error calling print statement.");
            return -1;
        }
        Py_DECREF(msg5);
        #endif
        // not already a list, create one
        collision_container = PyList_New(1);
        ((PyObject **)leaf->values->arr)[ix] = collision_container;
        // SetItem steals a reference, which is acceptable in this case because
        // we're removing it from the leaf container
        PyList_SetItem(collision_container, 0, val_ix);
    } else {
        #if DEBUG >= 1
        PyObject
            *msg6 = PyUnicode_FromString("BPlusLeaf_insert: got collision: already exists a collision container:");
        if (PyObject_CallFunctionObjArgs(_print, msg6, val_ix, NULL) == NULL) {
            Py_DECREF(msg6);
            Py_INCREF(PyExc_RuntimeError);
            PyErr_SetString(PyExc_RuntimeError, "Got error calling print statement.");
            return -1;
        }
        Py_DECREF(msg6);
        #endif
        // there is already a list, check if {o} is contained in it
        collision_container = val_ix;
        for (int jx = 0; jx < PyList_Size(collision_container); jx++) {
            PyObject *subval = PyList_GetItem(collision_container, jx);
            if (PyObject_RichCompareBool(o, subval, Py_EQ) == 1) {
                return 0;
            }
        }
    }

    // if we get to this point, it means we have a {collision_container} that
    // does not currently contain {o}

    // the append operation increases refcount of {o}
    if (PyList_Append(collision_container, o) == -1) {
        // append operation failed
        return -1;
    }
 
    // keep the list in sorted order
    if (PyList_Sort(collision_container) == -1) {
        // sort operation failed
        Py_INCREF(PyExc_RuntimeError);
        PyErr_SetString(PyExc_RuntimeError, "Got error sorting {collision_container} after attempting to handle a hash collision.");
        return -1;
    }

    #if DEBUG >= 1
    PyObject
        *msg7 = PyUnicode_FromString("BPlusLeaf_insert: collision container directly before return:");
    if (PyObject_CallFunctionObjArgs(_print, msg7, collision_container, NULL) == NULL) {
        Py_DECREF(msg7);
        Py_INCREF(PyExc_RuntimeError);
        PyErr_SetString(PyExc_RuntimeError, "Got error calling print statement.");
        return -1;
    }
    Py_DECREF(msg7);
    #endif

    return 1;
 
}
