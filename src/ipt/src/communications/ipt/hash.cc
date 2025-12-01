///////////////////////////////////////////////////////////////////////////////
//
//                                 hash.cc
//
// This file implements the class to implement a simple hash table abstract
// data type.  This code is derived from original TCX code, so it gets a 
// little messy
//
// Classes defined for export:
//    IPHashTable
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef VXWORKS
#include "./libc.h"
#endif

#include <ipt/hash.h>

#define ABS(x) (((x) < 0) ? (-(x)) : (x))

/* structure for hash buckets */
struct IPHashElem {
    char* key;
    char* data;
    IPHashElem* next;
};

/* Create a hash table with "size" buckets with hash function "hashFunc" and
   equality function "eqFunc" */
IPHashTable::IPHashTable(int size, int (*hashFunc)(const char*),
                         int (*eqFunc)(const char*, const char*))
{
    int i;
    
    _table = new IPHashElem*[size];
    for(i=0;i < size;i++)
        _table[i] = NULL;

    _size = size;
    _hash_func= hashFunc;
    _eq_func = eqFunc;
}

/* Delete the hash table and the keys.  Deletion of data is up to the user */
IPHashTable::~IPHashTable()
{
    int i;
    IPHashElem* cur;
    IPHashElem* doomed;

    for (i=0; i<_size;i++) {
        cur = _table[i];
        while (cur) {
            doomed = cur;
            cur = cur->next;
            delete doomed->key;
            delete doomed;
        }
    }

    // ENDO - MEM LEAK
    //delete _table;
    delete [] _table;
    _table = NULL; // Added by ENDO
}

/* Find an element in the hash table bucket given an equality function
   "eq", the bucket "list", and the key "key" */
static IPHashElem* findElement(int (*eq)(const char*, const char*),
                               IPHashElem* list, const char* key)
{
    IPHashElem* tmp;

    tmp = list;
    while (tmp) {
        if ((*eq)(tmp->key, key))
            return tmp;
        tmp = tmp->next;
    }
  
    return NULL;
}

/* Find an element of the hash table matching "key" and return the data 
   associated with that key.  If no matching element is found, return NULL */
char* IPHashTable::Find(const char* key)
{
    IPHashElem* tmp;
    int hash, location;

    hash = (*_hash_func)(key);
    location = hash % _size;
    location = ABS(location);

    tmp = _table[location];
    if (tmp) {
        tmp = findElement(_eq_func, tmp, key);
        if (tmp)
            return(tmp->data);
        else
            return NULL;
    } else
        return NULL;
}

/* Insert an item with key "key" (key having "keySize" bytes) and data
   "item."  Note that the key is copied while the item is just referenced */
char* IPHashTable::Insert(const char *key, int keySize, char *item)
{
    char *oldData;
    int hash, location;
    IPHashElem* tmp;
    IPHashElem* element;

    hash = (*_hash_func)(key);
    location = hash % _size;
    location = ABS(location);

    tmp = _table[location];
    if (tmp) {
        tmp = findElement(_eq_func, tmp, key);
        if (tmp) {
            /* replace item with new information */
            oldData = tmp->data;
            tmp->data = item;

            return oldData;
        }
    }
    
    element = new IPHashElem;
    element->key = new char[keySize];
    bcopy(key, element->key, keySize);
    element->data = item;
    element->next = _table[location];
    _table[location] = element;

    return NULL;
}

/* Remove the element matching "key" from the hash table.  If there was an 
   element, return the data of the element that was removed.  The internal
   key is deleted */
char* IPHashTable::Remove(const char* key)
{
    int (*eq)(const char*, const char*);
    char *oldData;
    int hash, location;
    IPHashElem* previous;
    IPHashElem* current;

    hash = (*_hash_func)(key);
    location = hash % _size;
    location = ABS(location);

    eq = _eq_func;

    previous = _table[location];
    if (!previous)
        return NULL;

    if ((*eq)(previous->key, key)) {
        _table[location] = previous->next;
        oldData = previous->data;
        delete previous->key;
        delete previous;
        return oldData;
    }
    current = previous->next;
    while (current) {
        if ((*eq)(current->key, key)) {
            oldData = current->data;
            previous->next = current->next;
            delete current->key;
            delete current;
            return oldData;
        }
        previous = current;
        current = current->next;
    }
    
    return NULL;
}

/* Iterate a function "iterFunc" over every element in the hash table */
char* IPHashTable::Iterate(int (*iterFunc)(char*, char*))
{
    int i;
    IPHashElem* tmp;

    for (i=0;i < _size;i++) {
        tmp = _table[i];
        while (tmp){
            if (!(*iterFunc)(tmp->key, tmp->data))
                return tmp->key;
            tmp = tmp->next;
        }
    }

    return NULL;
}

/* Iterate a function "iterFunc" over every element in the hash table */
char* IPHashTable::Iterate(int (*iterFunc)(char*, char*, void*), void* data)
{
    int i;
    IPHashElem* tmp;

    for (i=0;i < _size;i++) {
        tmp = _table[i];
        while (tmp){
            if (!(*iterFunc)(tmp->key, tmp->data, data))
                return tmp->key;
            tmp = tmp->next;
        }
    }

    return NULL;
}

/* print statistics about the hash table */
void IPHashTable::Stats()
{
    IPHashElem* elem;
    int i, num=0, max=0, full=0, length;

    for (i=0;i < _size; i++) {
        elem = _table[i];
        if (elem) {
            full++;
            length = 0;
            while (elem) {
                num++;
                length++;
                elem = elem->next;
            }
            if (length > max) 
                max = length;
        }
    }
    fprintf(stderr, 
            "hashTableStats: Has %d elements in %d slots; maximum list is %d\n",
            num, full, max);
}

/* string equality function */
int str_eq(const char* s1, const char* s2)
{
    return (strcmp(s1, s2) == 0);
}

/* string hash function */
int str_hash(const char* str)
{
    register char *p = (char*) str;
    register char c;
    register int hash = 0;
    
    while (*p) {
        c = *p++;
        if (c >= 0140) c -= 40;
        hash = ((hash<<3) + (hash>>28) + c);
    }
    return (hash & 07777777777);
}

/* integer hash function (expects a pointer to an integer cast to char*) */
int int_hash(const char* ptr)
{
    return *((int*) ptr);
}

/* integer equality function (expects a pointers to an integer cast to
   char*'s) */
int int_eq(const char* s1, const char* s2)
{
    return *((int*) s1) == *((int*) s2);
}

/* pointer hash function (expects a pointer to some other kind of pointer
   cast as a char*) */
int ptr_hash(const char* ptr)
{
    return (*((int*) ptr)) >> 2;  // assumes a 4 byte word size
}
