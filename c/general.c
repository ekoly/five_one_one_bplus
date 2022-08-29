// Definitions for general-use helper functions used by the B-Plus Tree code.

#include <Python.h>
#include "general.h"

// globals for builtin methods
// these should be set the first time a BPlusTree is instantiated
PyObject
    *_builtins = NULL,
    *_iter = NULL,
    *_next = NULL,
    *_hash = NULL,
    *_print = NULL;

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

