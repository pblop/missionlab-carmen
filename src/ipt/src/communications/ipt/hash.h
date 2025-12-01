///////////////////////////////////////////////////////////////////////////////
//
//                                 hash.h
//
// This header file defines the class to implement a simple hash table abstract
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

#ifndef ip_hash_h
#define ip_hash_h

struct IPHashElem;

/* class for doing a generic hash table.  You need to give it a hash function
   and an equality function and then do a lot of casting, but it works. */
class IPHashTable {
  public:
    IPHashTable(int, int (*)(const char*), int (*)(const char*, const char*));
    virtual ~IPHashTable();

    char* Find(const char*);
    char* Insert(const char*, int, char*);
    char* Remove(const char*);

    char* Iterate(int (*)(char*, char*));
    char* Iterate(int (*)(char*, char*, void*), void*);
    void Stats();
    
  private:
    int _size;                       // number of buckets 
    int (*_hash_func)(const char*);  // hash function 
    int (*_eq_func)(const char*, const char*); // equality function
    IPHashElem** _table;             // the buckets
};

/* built in hash and equality functions for strings and integers */
extern int str_eq(const char*, const char*);
extern int str_hash(const char*);
extern int int_hash(const char*);
extern int int_eq(const char*, const char*);
extern int ptr_hash(const char*);

#endif
