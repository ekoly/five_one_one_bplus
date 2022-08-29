// Definitions of public and private methods used by the B-Plus Tree.
// See bplustree.c for documentation on the methods declared in this file.


#ifndef BPLUSTREE_H
#define BPLUSTREE_H


#include <Python.h>
#include "structmember.h"
#include "bplustypes.h"
#include "general.h"
#include "array32.h"
#include "bplusnode.h"


// BEGIN BPlusTree private helper method headers
static void BPlusBranch_split(BPlusTree *self, BPlusNode *branch);
static void BPlusLeaf_split(BPlusTree *self, BPlusNode *leaf);
static PyObject *BPlusTree_insert(BPlusTree *tree, l64 key, PyObject *o);
static int BPlusTree_cmp(BPlusTree *tree1, BPlusTree *tree2);


// BEGIN tp method headers
static PyObject *BPlusTree_tp_new(PyTypeObject *subtype, PyObject *args, PyObject *kwargs);
static void BPlusTree_tp_clear(BPlusTree *self);
static void BPlusTree_tp_dealloc(BPlusTree *self);
static int BPlusTree_tp_init(BPlusTree *self, PyObject *args, PyObject *kwargs);
static PyObject *BPlusTree_tp_richcompare(PyObject *o1, PyObject *o2, int op);
static PyObject *BPlusTree_tp_iter(PyObject *self);


// BEGIN sequence method headers
static Py_ssize_t BPlusTree_sq_length(PyObject *self);
static int BPlusTree_sq_contains(PyObject *self, PyObject *value);


// BEGIN public method headers
static PyObject *BPlusTree_method_get_b(PyObject *self, PyObject *args);
static PyObject *BPlusTree_method_add(PyObject *self, PyObject *args);
static PyObject *BPlusTree_method_get_indices(PyObject *self, PyObject *args);


#endif
