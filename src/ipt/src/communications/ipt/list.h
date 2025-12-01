///////////////////////////////////////////////////////////////////////////////
//
//                                 list.h
//
// This header file defines the class to implement a simple linked list 
// abstract data type.  This code is derived from original TCX code, so it 
// gets a little messy.  One of its quirks is that it tries to do slightly more
// efficient memory management of lists and list elements.  That is why you
// cannot do a "new IPList," but much use the IPList::Create and IPList::Delete
// static member functions to make and destroy IPList's
//
// Classes defined for export:
//    IPList
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ip_list_h
#define ip_list_h

struct IPListElem;

/* The list abstract data type */
class IPList {
  public:
    IPList();
    IPList(char*);
    IPList(char*, char*);
    ~IPList();
    
    void Prepend(char*);
    void Append(char*);
    void InsertAfter(char*, const char*);
    char* Pop();
    char* Remove(int (*)(const char*, const char*), const char*);
    void Remove(const char*);
    void RemoveAll(int (*)(const char*, const char*), const char*);
    void RemoveAll(const char*);
    void Clear();
    int Member(const char*);
    char* Return(int (*)(const char*, const char*), const char*);
    int Iterate(int (*)(char*, char*), char*);
    int IterateFromLast(int (*)(char*, char*), char*);
    IPList* Copy();
    int Equal(IPList*);
    char* First();
    char* Last();
    char* Next();
    void InsertUnique(char*);

    int Length() { return _length; }
    
    static IPList* Create() { return new IPList(); }
    static IPList* Create(char* item) { return new IPList(item); }
    static IPList* Create(char* item1 , char* item2)
        { return new IPList(item1, item2); }
    static void Delete(IPList* list) { if (list) delete list; }

    static void initThreads();

  private:
    void initialize();

  private:
    int _length;         // the number of elements in the list
    IPListElem* _first;  // the first element in the list
    IPListElem* _last;   // the last element in the list 
    IPListElem* _next;   // used to make the First, Next convenience functions
                         // work for iterating over the list
};

#endif 
