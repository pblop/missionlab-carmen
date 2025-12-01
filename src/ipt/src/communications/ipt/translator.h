#ifndef ip_translator_h
#define ip_translator_h

class IPHashTable;

class IPTranslator {
  public:
    IPTranslator(const char* domain_name);
    ~IPTranslator();

    int translation(int);
    void make_translation(int, int);
    void clear_translations();
    const char* domain_name() { return _domain_name; }

  private:
    IPHashTable* _table;
    char* _domain_name;
};

#endif
