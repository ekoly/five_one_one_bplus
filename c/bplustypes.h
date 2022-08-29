// Definitions of the types used in this module.


#ifndef BPLUSTYPES_H
#define BPLUSTYPES_H


// convenience type for storing hashes
typedef long long int l64;


// convenience type for representing arrays
// we'll have {Array32} objects storing {l64}, {PyObject *}, and {BPlusNode *}.
typedef struct Array32 {
    void *arr;
    int size;
} Array32;


// Our basic node object.
// May be either a "leaf node" (no child nodes) or a "branch node" (has child
// nodes).
//  1. All nodes have {indices} which is an {Array32} storing {l64} hashes.
//  2. All nodes besides the root node have a {parent} which is a pointer to
//      the parent node.
//  3. "branches" have {children} which is an {Array32} storing {BPlusNode *}.
//  4. "leaves" have {values} which is an {Array32} storing {PyObject *}.
//  5. "leaves" also have {prev} and {next}, which are pointers to the
//      neighboring leaves.
typedef struct BPlusNode {
    Array32 *indices;
    Array32 *values;
    Array32 *children;
    struct BPlusNode *parent;
    struct BPlusNode *prev;
    struct BPlusNode *next;
} BPlusNode;


// define our python type
typedef struct BPlusTree {
    PyObject_HEAD
    BPlusNode *root;
    int b;
    int size;
    BPlusNode *iter_current_node;
    int iter_current_index;
} BPlusTree;


#endif
