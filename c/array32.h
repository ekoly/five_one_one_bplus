// Definition of array type and helper methods related to it.
// See array32.c for documentation on the methods declared in this file.

#ifndef ARRAY32_H
#define ARRAY32_H


#include <Python.h>
#include "bplustypes.h"
#include "general.h"


// BEGIN Array32 helper function headers
int bisect_left(Array32 *a, l64 x);
int bisect_right(Array32 *a, l64 x);
int insert_l64(Array32 *a, int index, l64 x);
int insert_PyObject(Array32 *a, int index, PyObject *x);
int insert_BPlusNode(Array32 *a, int index, BPlusNode *x);


#endif
