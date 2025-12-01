#include <stdio.h>

#include <ipt/resource.h>
#include <ipt/list.h>

#ifdef VXWORKS
#include <taskLib.h>
#endif

#ifdef WITH_PTHREADS
#include <ipt/libc.h>

pthread_mutex_t _res_mutex_data;
pthread_mutex_t* _res_mutex = NULL;

#endif

class IPResourceImpl {
friend class IPResource;

    static int _deferred;
    static IPList* _deletes;
};

int IPResourceImpl::_deferred = 0;
IPList* IPResourceImpl::_deletes = NULL;

IPResource::IPResource() { _refcount = 0; }
IPResource::~IPResource() { }

void IPResource::ref() const
{
    IPResource* r = (IPResource*)this;
    r->_refcount += 1;
}

void IPResource::unref() const
{
    IPResource* r = (IPResource*)this;
    if (r->_refcount != 0) {
        r->_refcount -= 1;
    }
    if (r->_refcount == 0) {
        r->cleanup();
        delete r;
    }
}

void IPResource::unref_deferred() const
{
    IPResource* r = (IPResource*)this;
    if (r->_refcount != 0) {
        r->_refcount -= 1;
    }
    if (r->_refcount == 0) {
        r->cleanup();
        if (IPResourceImpl::_deferred) {
#ifdef VXWORKS
            taskLock();
#endif
#ifdef WITH_PTHREADS
            if (_res_mutex) 
                pthread_mutex_lock(_res_mutex);
#endif
            if (IPResourceImpl::_deletes == NULL) {
                IPResourceImpl::_deletes = new IPList;
            }
            IPResourceImpl::_deletes->Append((char*) r);
#ifdef VXWORKS
            taskUnlock();
#endif
#ifdef WITH_PTHREADS
            if (_res_mutex) 
                pthread_mutex_unlock(_res_mutex);
#endif
        } else {
            delete r;
        }
    }
}

void IPResource::cleanup() { }

void IPResource::ref(const IPResource* r)
{
    if (r != NULL) {
        r->ref();
    }
}

void IPResource::unref(const IPResource* r)
{
    if (r != NULL) {
        r->unref();
    }
}

void IPResource::unref_deferred(const IPResource* r)
{
    if (r != NULL) {
        r->unref_deferred();
    }
}

int IPResource::defer(int b)
{
    int previous = IPResourceImpl::_deferred;
    if (b != previous) {
        flush();
        IPResourceImpl::_deferred = b;
    }
    return previous;
}

void IPResource::flush(int unlock)
{
#ifdef VXWORKS
    taskLock();
#endif
#ifdef WITH_PTHREADS
    if (_res_mutex) 
        pthread_mutex_lock(_res_mutex);
#endif
    IPList* list = IPResourceImpl::_deletes;
    if (list != NULL) {
        int previous = IPResourceImpl::_deferred;
        IPResourceImpl::_deferred = 0;
        IPResource* r;
        while ((r = (IPResource*) list->Pop())) 
            delete r;

        IPResourceImpl::_deferred = previous;
    }
    if (unlock) {
#ifdef VXWORKS
        taskUnlock();
#endif
#ifdef WITH_PTHREADS
        if (_res_mutex) 
            pthread_mutex_unlock(_res_mutex);
#endif
    }
}

void IPResource::clear()
{
    flush(0);
    delete IPResourceImpl::_deletes;
    IPResourceImpl::_deletes = NULL;
#ifdef VXWORKS
    taskUnlock();
#endif
#ifdef WITH_PTHREADS
    if (_res_mutex) 
        pthread_mutex_unlock(_res_mutex);
#endif
}

void IPResource::initThreads()
{
#ifdef WITH_PTHREADS
    _res_mutex = &_res_mutex_data;
    pthread_mutex_init(_res_mutex, NULL);
#endif
}
    
