///////////////////////////////////////////////////////////////////////////////
//
//                                 format.h
//
// This file declares the classes that can be used to format message data.
//
// Classes defined for export:
//    IPFormat, IPPointerFormat, IPFixedArrayFormat, IPVarArrayFormat,
//    IPStructFormat, IPPrimFormat, IPLengthFormat, IPFormatAction
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ip_formatters_h
#define ip_formatters_h

#include <ipt/formconfig.h>
#include <ipt/resource.h>

class IPFormat;
class IPStructFormat;
class IPHashTable;
class IPFormatParser;

struct IPSizesType {
  int buffer, data;
};

class IPFormatAction {
  public:
    virtual void start_struct(int) const {};
    virtual void end_struct() const {};
    virtual void start_fixed_array(int, const IPFormat*) const {};
    virtual void end_fixed_array() const {};
    virtual void start_var_array(int, const IPFormat*) const {};
    virtual void end_var_array() const {};
    virtual void start_pointer(const IPFormat*) const {};

    virtual void length_act(int l, char*) const = 0;
    virtual void char_act(char c) const = 0;
    virtual void int_act(int i) const = 0;
    virtual void short_act(short s) const = 0;
    virtual void long_act(long l) const = 0;
    virtual void double_act(double d) const = 0;
    virtual void float_act(float f) const = 0;
    virtual void bool_act(int b) const = 0;
    virtual void string_act(const char* s) const = 0;
};

/* The formatting class.  It's members are derived directly from the TCX 
   C routines, so things get a little messy */
class IPFormat : public IPResource {
  public:
    enum {
        LengthFMT, StructFMT, PointerFMT, FixedArrayFMT, VarArrayFMT,
        StringFMT, CharFMT, IntFMT, ShortFMT, LongFMT, FloatFMT, DoubleFMT,
        BooleanFMT,
    };

    IPFormat(IPFormatParser*, int);

    virtual int equals(const IPFormat*) const ;
    virtual int alignCheck() const { return 0; }
    virtual IPSizesType bufferSize(const char* dataStruct,
                                   int dStart, 
                                   const IPStructFormat* parentFormat,
                                   int struct_start) const = 0;
    virtual IPSizesType transferToBuffer(char* dataStruct, int dStart,
                                         char *buffer, int bStart, 
                                         const IPStructFormat* parentFormat,
                                         int struct_start) const = 0;
    virtual IPSizesType
        transferToDataStructure(char* dataStruct, int dStart,
                                char *buffer, int bStart, int bSize,
                                const IPStructFormat* parentFormat,
                                int byteOrder, int alignment) const = 0;
    virtual int freeDataElements(char*, int, char*, int, const IPStructFormat*,
                         int struct_start) const = 0;
    virtual int actData(const IPFormatAction*, char* dataStruct, int dStart,
                        const IPStructFormat* parentFormat,
                        int struct_start) const = 0;

    int dataStructureSize() const;
    int bufferSize(void *DataStruct) const;
    void encodeData(void *DataStruct, char *Buffer, int BStart) const;
    void *decodeData(char *Buffer, int BStart, int BSize,
                     char *DataStruct,
                     int byteOrder, int alignment) const;
    void freeDataStructure(void *dataStruct, char*, int) const;
    void PrintData(void*) const;
    int alignFormat(int currentDataSize) const;
    virtual int sameFixedSizeDataBuffer() const = 0;

    IPFormatParser* parser() const { return _parser; }
    int type() const { return _type; }

/* should be protected, bugs in C++ compilers force public */    
    virtual int elementSize() const = 0;
    virtual int data_structure_size() const = 0;
    virtual int fixedLengthFormat() const = 0;
    virtual int flatNoPaddingFormat() const = 0;
#if ((ALIGN & ALIGN_LONGEST) || (ALIGN & ALIGN_INT))
    virtual int mostRestrictiveElement() const = 0;
#endif
    virtual IPFormat* optimizeFormatter();
    virtual IPSizesType
        transfer_array_to_buffer(int arraySize,
                                 char* dataStruct, int currentData,
                                 char *buffer, int currentByte,
                                 const IPStructFormat* parentFormat,
                                 int struct_start) const;
    virtual IPSizesType transfer_array_to_data(int arraySize,
                                               char* dataStruct, int dStart,
                                               char* buffer, int bStart,
                                               int bSize, int byteOrder,
                                               int alignment);

    int cache_fixed_size(int fs) const;
    int fixed_size_cache() const { return _fixed_size_data_buffer; }
    
  private:
    int _type;
    IPFormatParser* _parser;
    int _fixed_size_data_buffer; // 1 if no pointers, -1 if unknown
    int _data_structure_size;   // size of data, -1 for unknown
};

class IPPrimFormat : public IPFormat {
  public:
    IPPrimFormat(IPFormatParser* parser, int simple_type,
                 int type) : IPFormat(parser, type)
                     { _simple_type = simple_type; }
    int simple_type() const { return _simple_type; }

    virtual int sameFixedSizeDataBuffer() const;
    virtual IPSizesType bufferSize(const char* dataStruct, int dStart, 
                                   const IPStructFormat* parentFormat,
                                   int struct_start) const;
    IPSizesType transferToBuffer(char* dataStruct, int dStart,
                                 char *buffer, int bStart, 
                                 const IPStructFormat* parentFormat,
                                 int struct_start) const;
    virtual IPSizesType
        transferToDataStructure(char* dataStruct, int dStart,
                                char *buffer, int bStart, int bSize,
                                const IPStructFormat* parentFormat,
                                int byteOrder, int alignment) const;
    virtual IPSizesType
        transfer_array_to_buffer(int arraySize,
                                 char* dataStruct, int currentData,
                                 char *buffer, int currentByte,
                                 const IPStructFormat* parentFormat,
                                 int struct_start) const;
    virtual IPSizesType transfer_array_to_data(int arraySize,
                                               char* dataStruct, int dStart,
                                               char* buffer, int bStart,
                                               int bSize, int byteOrder,
                                               int alignment);
    virtual int freeDataElements(char*, int, char*, int, const IPStructFormat*,
                                 int struct_start) const;

    virtual int alignCheck() const;
    virtual int data_structure_size() const;
    virtual int elementSize() const;
    virtual int fixedLengthFormat() const;
    virtual int flatNoPaddingFormat() const;
#if ((ALIGN & ALIGN_LONGEST) || (ALIGN & ALIGN_INT))
    virtual int mostRestrictiveElement() const;
#endif

    virtual int encode(const char* datastruct, int dstart,
                       char* buffer, int bstart) const = 0;
    virtual int decode(char* datastruct, int dstart,
                       char* buffer, int bstart, int bsize,
                       int byteOrder, int alignment) const = 0;
    virtual int e_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const = 0;
    virtual int a_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const = 0;
    virtual int r_length(const char* datastruct, int dstart,
                         char* buffer, int bstart) const = 0;
    virtual int free_data(char*, int, char*, int, int) const { return 0; }

  private:
    int _simple_type;
};

class IPLengthFormat : public IPFormat {
  public:
    IPLengthFormat(IPFormatParser* parser, int length)
        : IPFormat(parser, LengthFMT)
        { _length = length; }

    int length() const { return _length; }

    virtual int equals(const IPFormat*) const ;
    virtual int sameFixedSizeDataBuffer() const;
    virtual IPSizesType bufferSize(const char* dataStruct, int dStart, 
                                   const IPStructFormat* parentFormat,
                                   int struct_start) const;
    IPSizesType transferToBuffer(char* dataStruct, int dStart,
                                 char *buffer, int bStart, 
                                 const IPStructFormat* parentFormat,
                                 int struct_start) const;
    virtual IPSizesType
        transferToDataStructure(char* dataStruct, int dStart,
                                char *buffer, int bStart, int bSize,
                                const IPStructFormat* parentFormat,
                                int byteOrder, int alignment) const;
    virtual int freeDataElements(char*, int, char*, int, const IPStructFormat*,
                                 int struct_start) const;
    virtual int actData(const IPFormatAction*, char* dataStruct, int dStart,
                        const IPStructFormat* parentFormat,
                        int struct_start) const;

    virtual int data_structure_size() const;
    virtual int elementSize() const;
    virtual int fixedLengthFormat() const;
    virtual int flatNoPaddingFormat() const;
#if ((ALIGN & ALIGN_LONGEST) || (ALIGN & ALIGN_INT))
    virtual int mostRestrictiveElement() const;
#endif
    virtual int alignCheck() const;

  private:
    int _length;
};

class IPPointerFormat : public IPFormat {
  public:
    IPPointerFormat(IPFormatParser* parser, IPFormat* f)
        : IPFormat(parser, PointerFMT)
        { _format = f; }
    virtual ~IPPointerFormat();

    IPFormat* format() const { return _format; }
    
    virtual int equals(const IPFormat*) const ;
    virtual IPFormat* optimizeFormatter();
    virtual int sameFixedSizeDataBuffer() const;
    virtual IPSizesType bufferSize(const char* dataStruct, int dStart, 
                                   const IPStructFormat* parentFormat,
                                   int struct_start) const;
    IPSizesType transferToBuffer(char* dataStruct, int dStart,
                                 char *buffer, int bStart, 
                                 const IPStructFormat* parentFormat,
                                 int struct_start) const;
    virtual IPSizesType
        transferToDataStructure(char* dataStruct, int dStart,
                                char *buffer, int bStart, int bSize,
                                const IPStructFormat* parentFormat,
                                int byteOrder, int alignment) const;
    virtual int freeDataElements(char*, int, char*, int, const IPStructFormat*,
                                 int struct_start) const;
    virtual int actData(const IPFormatAction*, char* dataStruct, int dStart,
                        const IPStructFormat* parentFormat,
                        int struct_start) const;

    virtual int data_structure_size() const;
    virtual int elementSize() const;
    virtual int fixedLengthFormat() const;
    virtual int flatNoPaddingFormat() const;
#if ((ALIGN & ALIGN_LONGEST) || (ALIGN & ALIGN_INT))
    virtual int mostRestrictiveElement() const;
#endif

  private:
    const IPFormat* choose_ptr_format(const IPFormat* parentFormat) const
        { return ( _format ? _format : parentFormat ); }

  private:
    IPFormat* _format;
};

class IPStructFormat : public IPFormat {
  public:
    IPStructFormat(IPFormatParser* p, int num_elems);
    virtual ~IPStructFormat();

    int num_elements() const { return _num_elements; }
    IPFormat** elements() const { return _elements; }

    int equals(const IPFormat*) const;
    int alignField(int currentField, int currentDataSiz) const;
    virtual IPFormat* optimizeFormatter();
    virtual IPSizesType bufferSize(const char* dataStruct, int dStart, 
                                   const IPStructFormat* parentFormat,
                                   int struct_start) const;
    IPSizesType transferToBuffer(char* dataStruct, int dStart,
                                 char *buffer, int bStart, 
                                 const IPStructFormat* parentFormat,
                                 int struct_start) const;
    virtual IPSizesType
        transferToDataStructure(char* dataStruct, int dStart,
                                char *buffer, int bStart, int bSize,
                                const IPStructFormat* parentFormat,
                                int byteOrder, int alignment) const;
    virtual int freeDataElements(char*, int, char*, int, const IPStructFormat*,
                                 int struct_start) const;
    virtual int actData(const IPFormatAction*, char* dataStruct, int dStart,
                        const IPStructFormat* parentFormat,
                        int struct_start) const;

    virtual int sameFixedSizeDataBuffer() const;
    virtual int data_structure_size() const;
    virtual int elementSize() const;
    virtual int fixedLengthFormat() const;
    virtual int flatNoPaddingFormat() const;
#if ((ALIGN & ALIGN_LONGEST) || (ALIGN & ALIGN_INT))
    virtual int mostRestrictiveElement() const;
#endif

  private:
    int _num_elements;
    IPFormat** _elements;
};

class IPFixedArrayFormat : public IPFormat {
  public:
    IPFixedArrayFormat(IPFormatParser* p, IPFormat*, int num_dimensions);
    virtual ~IPFixedArrayFormat();

    IPFormat* format() const { return _format; }
    int num_dimensions() const { return _num_dimensions; }
    int* dimensions() const { return _dimensions; }

    virtual int equals(const IPFormat*) const ;
    virtual int sameFixedSizeDataBuffer() const;
    virtual IPSizesType bufferSize(const char* dataStruct, int dStart, 
                                   const IPStructFormat* parentFormat,
                                   int struct_start) const;
    virtual IPFormat* optimizeFormatter();
    IPSizesType transferToBuffer(char* dataStruct, int dStart,
                                 char *buffer, int bStart, 
                                 const IPStructFormat* parentFormat,
                                 int struct_start) const;
    virtual IPSizesType
        transferToDataStructure(char* dataStruct, int dStart,
                                char *buffer, int bStart, int bSize,
                                const IPStructFormat* parentFormat,
                                int byteOrder, int alignment) const;
    virtual int freeDataElements(char*, int, char*, int, const IPStructFormat*,
                                 int struct_start) const;
    virtual int actData(const IPFormatAction*, char* dataStruct, int dStart,
                        const IPStructFormat* parentFormat,
                        int struct_start) const;

    virtual int data_structure_size() const;
    virtual int elementSize() const;
    virtual int fixedLengthFormat() const;
    virtual int flatNoPaddingFormat() const;
#if ((ALIGN & ALIGN_LONGEST) || (ALIGN & ALIGN_INT))
    virtual int mostRestrictiveElement() const;
#endif

  private:
    int fixedArraySize() const;

  private:
    int _num_dimensions;
    int *_dimensions;
    IPFormat* _format;
};

class IPVarArrayFormat : public IPFormat {
  public:
    IPVarArrayFormat(IPFormatParser* p, IPFormat* f, int num_indices);
    ~IPVarArrayFormat();

    int num_indices() const { return _num_indices; }
    int* indices() const { return _indices; }
    IPFormat* format() const { return _format; }

    virtual int equals(const IPFormat*) const ;
    virtual int sameFixedSizeDataBuffer() const;
    virtual IPSizesType bufferSize(const char* dataStruct, int dStart, 
                                   const IPStructFormat* parentFormat,
                                   int struct_start) const;
    IPSizesType transferToBuffer(char* dataStruct, int dStart,
                                 char *buffer, int bStart, 
                                 const IPStructFormat* parentFormat,
                                 int struct_start) const;
    virtual IPSizesType
        transferToDataStructure(char* dataStruct, int dStart,
                                char *buffer, int bStart, int bSize,
                                const IPStructFormat* parentFormat,
                                int byteOrder, int alignment) const;
    virtual IPFormat* optimizeFormatter();
    virtual int freeDataElements(char*, int, char*, int, const IPStructFormat*,
                                 int struct_start) const;
    virtual int actData(const IPFormatAction*, char* dataStruct, int dStart,
                        const IPStructFormat* parentFormat,
                        int struct_start) const;

    virtual int data_structure_size() const;
    virtual int elementSize() const;
    virtual int fixedLengthFormat() const;
    virtual int flatNoPaddingFormat() const;
#if ((ALIGN & ALIGN_LONGEST) || (ALIGN & ALIGN_INT))
    virtual int mostRestrictiveElement() const;
#endif

  private:
    int array_size(const IPStructFormat*, const char*, int) const;

  private:
    IPFormat* _format;
    int _num_indices;
    int* _indices;
};

inline int DataInPlace(char* data_struct, char* buffer, int size_buffer)
{
    return (data_struct >= buffer && data_struct < buffer+size_buffer);
}

#endif 
