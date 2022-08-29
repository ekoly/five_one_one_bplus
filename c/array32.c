// Definition of array type and helper methods related to it.
#include <Python.h>
#include "bplustypes.h"
#include "general.h"
#include "array32.h"


// basically ripped from the Python bisect module
// {a} is an array of {l64} assumed to be in sorted order.
// Returns the leftmost index at which {x} can be inserted while keeping {a} in
// sorted order.
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

// basically ripped from the Python bisect module
// {a} is an array of {l64} assumed to be in sorted order.
// Returns the rightmost index at which {x} can be inserted while keeping {a} in
// sorted order.
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

// convenience method for inserting {x} into an array of {l64} at {index}.
int insert_l64(Array32 *a, int index, l64 x) {
    l64 *arr = (l64 *)a->arr;
    for (int i = a->size - 1; i >= index; i--) {
        arr[i+1] = arr[i];
    }
    arr[index] = x;
    a->size++;

    return 1;
}

// convenience method for inserting {x} into an array of {PyObject *} at {index}.
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

// convenience method for inserting {x} into an array of {BPlusNode *} at {index}.
int insert_BPlusNode(Array32 *a, int index, BPlusNode *x) {
    BPlusNode **arr = (BPlusNode **)a->arr;
    for (int i = a->size - 1; i >= index; i--) {
        arr[i+1] = arr[i];
    }
    arr[index] = x;
    a->size++;

    return 1;
}
