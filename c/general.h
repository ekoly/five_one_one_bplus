// Header file for general-use helper functions used by the B-Plus Tree code.
// See general.c for documentation on the methods declared in this file.

// set DEBUG to an integer greater than 0 to enable debug print statements.
// set DEBUG to 0 to disable debug print statements.
#define DEBUG 0

#ifndef GENERAL_H
#define GENERAL_H


#include <Python.h>


int setBuiltins();
void printRefCount(char *label, PyObject *x);


#endif
