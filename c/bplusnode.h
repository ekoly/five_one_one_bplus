// Definition of the BPlusNode type and methods related to it.
// See bplusnode.c for documentation on the methods declared in this file.

#ifndef BPLUSNODE_H
#define BPLUSNODE_H


#include <Python.h>
#include "bplustypes.h"
#include "general.h"
#include "array32.h"


// BEGIN BPlusNode helper functions
BPlusNode *BPlusNode_init(int b);
BPlusNode *BPlusLeaf_init(int b);
BPlusNode *BPlusBranch_init(int b);
void BPlusNode_dealloc(BPlusNode *node);
BPlusNode *BPlusNode_search(BPlusNode *root, l64 key);
int BPlusLeaf_search(BPlusNode *leaf, l64 key, PyObject *o);
int BPlusLeaf_insert(BPlusNode *leaf, l64 key, PyObject *o);


#endif
