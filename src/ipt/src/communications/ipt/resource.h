#ifndef ip_resource_h
#define ip_resource_h

class IPResource {
  public:
    IPResource();
    virtual ~IPResource();

    virtual void ref() const;
    virtual void unref() const;
    virtual void unref_deferred() const;
    virtual void cleanup();

    /* nops for nil pointers */
    static void ref(const IPResource*);
    static void unref(const IPResource*);
    static void unref_deferred(const IPResource*);

    /* postpone unref deletes */
    static int defer(int);
    static void clear();
    static void flush(int unlock=1);

    static void initThreads();

  private:
    unsigned _refcount;
  private:
    /* prohibit default assignment */
    IPResource& operator =(const IPResource&);
};
    
#endif
