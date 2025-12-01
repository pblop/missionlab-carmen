///////////////////////////////////////////////////////////////////////////////
//
//                                 list.cc
//
// This file implements the class to implement a simple linked list 
// abstract data type.  This code is derived from original TCX code, so it 
// gets a little messy.  One of its quirks is that it tries to do slightly more
// efficient memory management of list elements.  
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

#include <stdio.h>

#include <ipt/list.h>

#ifdef VXWORKS
#include <taskLib.h>
#endif

#ifdef WITH_PTHREADS
#include <ipt/libc.h>

pthread_mutex_t _list_mutex_data;
pthread_mutex_t* _list_mutex = NULL;

#endif

/* a list element */
struct IPListElem {
    IPListElem(IPListElem*);
    
    char *item;            // the data 
    IPListElem* next;      // the next element
    IPListElem* previous;  // the previous element
};

/* list of free cells */
static IPListElem* _cellFreeList = NULL;

/* how much to increase the list of free cells by when we run out */
#define LIST_CELL_INC_AMOUNT 10

static void listIncCellFreeList()
{
    int i;
    IPListElem* newCell;

    for(i=1;i<LIST_CELL_INC_AMOUNT;i++) {
        newCell = new IPListElem(_cellFreeList);

        _cellFreeList = newCell;
    }
}

inline IPListElem* listGetFreeCell()
{
#ifdef VXWORKS
    taskLock();
#endif
#ifdef WITH_PTHREADS
    if (_list_mutex) 
        pthread_mutex_lock(_list_mutex);
#endif
    if (!_cellFreeList)
        listIncCellFreeList();
    IPListElem* elem = _cellFreeList;
    _cellFreeList = _cellFreeList->next;
#ifdef VXWORKS
    taskUnlock();
#endif
#ifdef WITH_PTHREADS
    if (_list_mutex) 
        pthread_mutex_unlock(_list_mutex);
#endif

    return elem;
}

static void listFreeCell(IPListElem* listCell)
{
    listCell->item = NULL;
    listCell->previous = NULL;

#ifdef VXWORKS
    taskLock();
#endif
#ifdef WITH_PTHREADS
    if (_list_mutex) 
        pthread_mutex_lock(_list_mutex);
#endif
    listCell->next = _cellFreeList;
    _cellFreeList = listCell;
#ifdef VXWORKS
    taskUnlock();
#endif
#ifdef WITH_PTHREADS
    if (_list_mutex) 
        pthread_mutex_unlock(_list_mutex);
#endif
}

/* create a list element, and put it on the list of free cells "freecell" */
IPListElem::IPListElem(IPListElem* freecell)
{
    item = NULL;
    previous = NULL;
    next = freecell;
}

void IPList::initialize()
{
    _length = 0;
    _first = NULL;
    _last = NULL;
    _next = NULL;
}

/* Create a new list and but it on list of free list "freelist" */
IPList::IPList()
{
    initialize();
}


IPList::IPList(char* item)
{
    initialize();

    Prepend(item);
}

IPList::IPList(char* item1, char* item2)
{
    initialize();

    Prepend(item2);
    Prepend(item1);
}

IPList::~IPList()
{
    IPListElem* tmpA;
    IPListElem* tmpB;

    tmpA = _first;

    while (tmpA) {
        tmpB = tmpA;
        tmpA = tmpA->next;

        listFreeCell(tmpB);
    }
}

/**************************************************************************
 *
 * FUNCTION: Prepend(item)
 *
 * DESCRIPTION: Adds item as the first item in the list.
 *
 * INPUTS: 
 * char *item - a pointer to an item of data.
 *
 * OUTPUTS: none.
 *
 * EXCEPTIONS:  
 * If the list is NULL, return.
 * If the item is NULL, return.
 *
 * DESIGN: 
 * Create a list element to store item and insert the element as the first item
 * in the list.
 *
 * NOTES: If malloc returns NULL simply return <- this is a major problem.
 *
 **************************************************************************/

void IPList::Prepend(char* item)
{
    IPListElem* element;

    if (!item || !this)
        return;

    element = listGetFreeCell();
    element->item = item;
    element->next = _first;
    element->previous = NULL;

    if (!_first) {
        _first = element;
    }
    else {
        _first->previous = element;
    }

    if (!_last) {
        _last = element;
    }

    _length++;
    _first = element;
}


/**************************************************************************
 *
 * FUNCTION: void Append(item)
 *
 * DESCRIPTION: Adds item as the last item in the list.
 *
 * INPUTS: 
 * char *item - a pointer to an item of data.
 *
 * OUTPUTS: none.
 *
 * EXCEPTIONS:  
 * If the list is NULL, return.
 * If the item is NULL, return.
 *
 * DESIGN: 
 * Create a list element to store item and insert the element as the first item
 * in the list.
 *
 * NOTES: If malloc returns NULL simply return <- this is a major problem.
 *
 **************************************************************************/

void IPList::Append(char* item)
{
    IPListElem* element;

    if (!item || !this)
        return;

    element = listGetFreeCell();
    element->item = item;
    element->next = NULL;
    element->previous = _last;

    if (!_first) {
        _first = element;
    }

    if (!_last) {
        _last = element;
    }
    else {
        _last->next = element;
    }

    _length++;
    _last = element;
}



/**************************************************************************
 *
 * FUNCTION: void InsertAfter(item, after)
 *
 * DESCRIPTION: Splices item into the list after, after.
 *
 * INPUTS: 
 * char *item - a pointer to an item of data.
 * char *after - the item to be inserted after.
 *
 * OUTPUTS: none.
 *
 * EXCEPTIONS:  
 * If the list is NULL, return.
 * If the item is NULL, return.
 * If after is NULL call listInsertItemFirst.
 * If after is not found call listInsertItemLast.
 *
 * NOTES: If malloc returns NULL simply return <- this is a major problem.
 *
 **************************************************************************/

void IPList::InsertAfter(char* item, const char* after)
{
    IPListElem* element;
    IPListElem* tmp;

    if (!item || !this)
        return;

    if (!after) {
        Prepend(item);
        return;
    }

    tmp = _first;

    while (tmp && tmp->item != after) 
        tmp = tmp->next;

    if (!tmp) {
        Append(item);
        return;
    }

    element = listGetFreeCell();
    element->item = item;
    element->next = tmp->next;
    element->previous = tmp;
    if (tmp->next)
        tmp->next->previous = element;

    tmp->next = element;

    _length++;
}


/**************************************************************************
 *
 * FUNCTION: char *Pop()
 *
 * DESCRIPTION: Removes and returns first item from list.
 *
 * OUTPUTS:
 * The first item or NULL
 *
 * EXCEPTIONS:  If the list is NULL or the list is empty, return NULL.
 *
 * DESIGN: 
 * Remove the first list element, return the item and free the element.
 *
 * NOTES:
 * might want to start a free list of list elements.
 *
 **************************************************************************/

char *IPList::Pop()
{
    char *item;
    IPListElem* oldElement;

    item = NULL;
    
    if (this && _first) {
        item = _first->item;
        oldElement = _first;
        _first = _first->next;
        if (_first) {
            _first->previous = NULL;
        }
        if (_last == oldElement) {
            _last = NULL;
        }
        _length--;
        listFreeCell(oldElement);
    }

    return item;
}


/* Remove the list element that matches "param" according to the match function
   "func".  Return the data associated with this list element, or NULL if there
   was no such matching element */
char* IPList::Remove(int (*func)(const char*, const char*), const char *param)
{
    IPListElem* current;
    IPListElem* previous;

    if (!this || !_first)
        return NULL;

    current = previous = _first;

    if ((*func)(param, current->item)) {
        /* item is the first element of the list */
        if (_last == current) {
            _last = NULL;
        }
        _first = current->next;
        if (current->next) {
            current->next->previous = NULL;
        }
        _length--;
        char* res = current->item;
        listFreeCell(current);
        return res;
    }

    current = current->next;

    while (current) {
        if ((*func)(param, current->item)) {
            if (_last == current) {
                _last = previous;
            }
            current->previous = previous;
            previous->next = current->next;

            if (current->next) {
                current->next->previous = previous;
            }

            _length--;
            char* res = current->item;
            listFreeCell(current);
            return res;
        }
        previous = current;
        current = current->next;
    }

    return NULL;
}

/**************************************************************************
 *
 * FUNCTION: void RemoveAll(func, param)
 *
 * DESCRIPTION: 
 * Removes all items in the list found such that func(param, item)
 * returns 1 (TRUE).
 *
 * INPUTS: 
 * int (*func)() - pointer to a test function of the form func(param, item).
 * char *param - a pointer to a parameter for func.
 *
 * OUTPUTS: none.
 *
 * EXCEPTIONS:  If the list is NULL or the list is empty, return;
 *
 * DESIGN: 
 * Linearly search the list for a list element containing item,
 * such that func(param, item) returns 1 (TRUE). If found
 * the list element is removed and freed. All the items of the list
 * are tested. Reset element's previous pointer and _last if needed.
 *
 *
 **************************************************************************/

void IPList::RemoveAll(int (*func)(const char*, const char*),
                       const char* param)
{
    IPListElem* current;
    IPListElem* previous;

    if (!this || !_first)
        return;

    int res;

    while (_first && (res=(*func)(param, _first->item))) {
        if (res == -1)
            return;
        _length--;
        current = _first;
        _first = current->next;

        if (_first) {
            _first->previous = NULL;
        }
        if (_last == current) {
            _last = NULL;
        }
        listFreeCell(current);
    }

    if (!_first)
        return;

    previous = _first;
    current  = _first->next;

    while (current) {
        if ((res = (*func)(param, current->item))) {
            if (res == -1)
                return;
            if (_last == current) {
                _last = previous;
            }

            previous->next = current->next;

            if (current->next) {
                current->next->previous = previous;
            }

            _length--;
            listFreeCell(current);
            
            current = previous->next;
        }
        else {
            previous = current;
            current = current->next;
        }
    }
}


/**************************************************************************
 *
 * FUNCTION: int listItemEq(a, b)
 *
 * DESCRIPTION: Simple Equal Test for Remove.
 *
 * INPUTS:
 * char *a, *b;
 *
 * OUTPUTS: Returns 1 TRUE or 0 FALSE.
 *
 * DESIGN: return(a == b);
 *
 * NOTES:
 *
 **************************************************************************/

int listItemEq(const char* a, const char* b)
{
    return(a == b);
}


/**************************************************************************
 *
 * FUNCTION: Remove(item)
 *
 * DESCRIPTION: Removes an item from list.
 *
 * INPUTS:
 * char *item; 
 *
 * OUTPUTS: none.
 *
 * DESIGN: call Remove with listItemEq test.
 *
 * NOTES: list is modified.
 *
 **************************************************************************/

void IPList::Remove(const char* item)
{
    Remove(listItemEq, item);
}


/**************************************************************************
 *
 * FUNCTION: RemoveAll(item)
 *
 * DESCRIPTION: Removes an all such item from list.
 *
 * INPUTS:
 * char *item; 
 *
 * OUTPUTS: none.
 *
 * DESIGN: call RemoveAll with listItemEq test.
 *
 * NOTES: list is modified.
 *
 **************************************************************************/

void IPList::RemoveAll(const char* item)
{
    RemoveAll(listItemEq, item);
}

int listTrue(const char*, const char*)
{
    return 1;
}

void IPList::Clear()
{
    RemoveAll(listTrue, NULL);
}

/**************************************************************************
 *
 * FUNCTION: int Member(item)
 *
 * DESCRIPTION: Tests if item is an element of list.
 *
 * INPUTS: 
 * char *item - a pointer to an item of data.
 *
 * OUTPUTS: 
 * 0 - FALSE 
 * 1 - TRUE
 *
 * EXCEPTIONS:  If the list is NULL or the list is empty, return;
 *
 * DESIGN: 
 * Linearly search the list for a list element containing item. 
 * If found the value 1 is returned, else the value 0 is returned.
 *
 * NOTES:
 *
 **************************************************************************/

int IPList::Member(const char* item)
{
    IPListElem* tmp;

    if (!this)
        return 0; /* False */

    tmp = _first;

    while (tmp) {
        if (tmp->item == item)
            return 1; /* TRUE */
        tmp = tmp->next;
    }

    return 0; /* False */
}


/**************************************************************************
 *
 * FUNCTION: char *Return(func, param)
 *
 * DESCRIPTION:
 * Return is a more general form of Member.
 * Return will return the item (or one of the items) in list
 * for which func(param, item) is non-zero, i.e. is TRUE.
 * The function takes two arguments, the first is the param and the second is 
 * an item of the list and returns an integer value. int func(param, item).
 * If the functions does not satisfy any of the items in the list NULL
 * is returned.
 *
 * INPUTS: 
 * int (*func)();
 * char *param - a pointer to a parameter that is passed to func.
 *
 * OUTPUTS: 
 * A pointer to an item in the list that satisfies func(param, item) 
 * or NULL if no such item exists. 
 *
 * EXCEPTIONS:  If the list is NULL or the list is empty, NULL is returned.
 *
 * DESIGN: 
 * Linearly search the list for a list element containing item, such that
 * func(param, item) is non-zero. Then return the item.
 *
 * NOTES:
 *
 **************************************************************************/

char *IPList::Return(int (*func)(const char*, const char*), const char *param)
{
    IPListElem* tmp;
    IPListElem* nextTmp;

    if (!this)
        return NULL;
    else {
        tmp = _first;
        while (tmp) {
            nextTmp = tmp->next;
            if ((*func)(param, tmp->item))
                return tmp->item;
            else
                tmp = nextTmp;
        }
        return NULL;
    }
}


/**************************************************************************
 *
 * FUNCTION: int Iterate(func, param)
 *
 * DESCRIPTION:
 * Iterate will call the function func with param on all of its
 * elements stoping when the list is finished or when func returns 0 (ie
 * FALSE). The function func will take two arguments the first is 
 * param the second is an item of the set. func(param, item).
 *
 * Iterate starts from the first item in the list and iterates
 * forward through the items in the list.
 *
 * INPUTS: 
 * int (*func)();
 * char *param - a pointer to a parameter that is passed to func.
 *
 * OUTPUTS: 
 * Iterate returns 0 (FALSE) if the function func returns 0 (FALSE).
 * Otherwise Iterate returns 1 (TRUE).
 *
 * EXCEPTIONS: 
 *
 * Iterate will return 0 (FASLE) if the list is NULL.
 *
 * DESIGN: 
 * iterate through the list of elements calling func on each item.
 * return when the list is finished or func has returned 0 (FALSE).
 *
 **************************************************************************/

int IPList::Iterate(int (*func)(char*, char*), char *param)
{
    char *item;
    IPListElem* tmp;
    IPListElem* nextTmp;

    if (!this)
        return 0;

    tmp = _first;

    while (tmp) {
        item = tmp->item;
        nextTmp = tmp->next;
        if (item && !(*func)(param, item))
            return 0;
        tmp = nextTmp;
    }

    return 1;
}


/**************************************************************************
 *
 * FUNCTION: int IterateFromLast(func, param)
 *
 * DESCRIPTION:
 * IterateFromLast will call the function func with param on all of its
 * elements stoping when the list is finished or when func returns 0 (ie
 * FALSE). The function func will take two arguments the first is 
 * param the second is an item of the set. func(param, item).
 *
 * IterateFromLast starts with thelast item in the list and iterates
 * backwards through the list.
 *
 * INPUTS: 
 * int (*func)();
 * char *param - a pointer to a parameter that is passed to func.
 *
 * OUTPUTS: 
 * IterateFromLast returns 0 (FALSE) if the function func returns 0 (FALSE).
 * Otherwise IterateFromLast returns 1 (TRUE).
 *
 * EXCEPTIONS: 
 *
 * IterateFromLast will return 0 (FASLE) if the list is NULL.
 *
 * DESIGN: 
 * iterate through the list of elements calling func on each item.
 * return when the list is finished or func has returned 0 (FALSE).
 *
 **************************************************************************/

int IPList::IterateFromLast(int (*func)(char*, char*), char *param)
{
    char *item;
    IPListElem* tmp;
    IPListElem* previousTmp;

    if (!this)
        return 0;

    tmp = _last;

    while (tmp) {
        item = tmp->item;
        previousTmp = tmp->previous;
        if (item && !(*func)(param, item))
            return 0;
        tmp = previousTmp;
    }

    return 1;
}


/* insert an item into list.  Used in Copy iteration */
static int listCopyInsert(IPList* list, char* item)
{
    list->Prepend(item);

    return 1;
}

/**************************************************************************
 *
 * FUNCTION: IPList* Copy()
 *
 * DESCRIPTION: Copies the given list.
 *
 * OUTPUTS: A pointer to the newly created list of type IPList*.
 * If there is an error NULL is returned.
 *
 * EXCEPTIONS: none.
 *
 * DESIGN: Iterate through the original list inserting the items into the
 * new list.
 *
 * NOTES:
 *
 **************************************************************************/

IPList* IPList::Copy()
{
    IPList* newList;

    newList = Create();

    IterateFromLast((int (*)(char*, char*)) listCopyInsert, (char*) newList);

    return newList;
}

/* Return true if list2 eq's this list */
int IPList::Equal(IPList* list2)
{
    int good;
    IPListElem* a;
    IPListElem* b;

    if (this == list2)
        return 1;

    /* this is the same style test used in tms.c but it is not general ! */

    a = _first;
    b = list2->_first;

    good = 1;
    while (good && a && b) {
        good = (a->item == b->item);
        a = a->next;
        b = b->next;
    }

    return(good && (a == NULL) && (b == NULL));
}

/* Return the first item of the list.  Also set up _next so that Next() will
   work in a simple for (i=First(); i; i=Next()) loop through the list */
char *IPList::First()
{
    if (this && _first) {
        _next = _first;
        return(_first->item);
    }
    else
        return NULL;
}

/* Get the last item in the list */
char *IPList::Last()
{
    if (this && _last) {
        return(_last->item);
    }
    else
        return NULL;
}

/* Get the next item in the list.  First() must have been called first */
char *IPList::Next()
{
    if (this && _next) {
        _next = _next->next;

        if (_next)
            return(_next->item);
    }

    return NULL;
}

/* Insert "item" if it is not already a member of the list */
void IPList::InsertUnique(char* item)
{
    if (!Member(item)) {
        Prepend(item);
    }
}

extern "C" void IPTclear_list_cache()
{
    IPListElem* tmpA;

    while(_cellFreeList != NULL) {
        tmpA = _cellFreeList;
        _cellFreeList = _cellFreeList->next;
        delete tmpA;
    }
}

void IPList::initThreads()
{
#ifdef WITH_PTHREADS
    _list_mutex = &_list_mutex_data;
    pthread_mutex_init(_list_mutex, NULL);
#endif
}
    
