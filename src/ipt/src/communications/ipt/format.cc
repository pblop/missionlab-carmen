///////////////////////////////////////////////////////////////////////////////
//
//                                 format.cc
//
// This file implements the classes that can be used to format message data.
// The algorithms were inherited from TCA v8.0 (authors Reid Simmons 
// (reids@cs.cmu.edu), and Rich Goodwinn (rich@cs.cmu.edu), but I have 
// converted it to a consistent C++ style to make it more readable and 
// efficient
//
// Classes defined for export:
//    IPFormat, IPPointerFormat, IPFixedArrayFormat, IPVarArrayFormat,
//    IPStructFormat, IPPrimFormat, IPLengthFormat
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
#include <stdlib.h>
#include <string.h>

#include "./libc.h"

#include <ipt/format.h>
#include <ipt/primformatters.h>
#include <ipt/form_macros.h>

/* Create an unknown formatter */
IPFormat::IPFormat(IPFormatParser* parser, int type)
{
    _fixed_size_data_buffer = -1;
    _data_structure_size = -1;
    _parser = parser;
    _type = type;
}

IPPointerFormat::~IPPointerFormat()
{
    IPResource::unref(_format);
}

IPFixedArrayFormat::IPFixedArrayFormat(IPFormatParser* p, IPFormat* f,
                                       int num_dim)
    : IPFormat(p, FixedArrayFMT)
{
    _format = f;
    if (_format)
        IPResource::ref(_format);
    _dimensions = new int[num_dim];
    _num_dimensions = num_dim;
}

IPFixedArrayFormat::~IPFixedArrayFormat()
{
    IPResource::unref(_format);
    delete _dimensions;
}

IPVarArrayFormat::IPVarArrayFormat(IPFormatParser* p, IPFormat* f,
                                   int num_indices) : IPFormat(p, VarArrayFMT)
{
    _format = f;
    _num_indices = num_indices;
    _indices = new int[_num_indices];
}

IPVarArrayFormat::~IPVarArrayFormat()
{
    delete _indices;
    IPResource::unref(_format);
}

IPStructFormat::IPStructFormat(IPFormatParser* p, int num_elems)
    : IPFormat(p, StructFMT)
{
    _num_elements = num_elems;
    _elements = new IPFormat*[num_elems];
    bzero((char*) _elements, num_elems*sizeof(IPFormat*));
}

IPStructFormat::~IPStructFormat()
{
    for(int i=0; i<_num_elements; i++)
        IPResource::unref(_elements[i]);

    delete _elements;
}

int IPFixedArrayFormat::fixedArraySize() const
{
    int i, arraySize;
  
    for(i=0, arraySize=1; i<num_dimensions(); i++)
        arraySize *= dimensions()[i];
    return arraySize;
}

int IPVarArrayFormat::array_size(const IPStructFormat* parentFormat,
                                 const char* dataStruct,
                                 int struct_start) const
{ 
    int i,j, arraySize=1, size=0;
    IPFormat** parentStructArray;
    int currentData, sizePlace;
  
    parentStructArray = parentFormat->elements();

    for(i=0, arraySize=1; i < num_indices(); i++) {
        currentData = struct_start;
        sizePlace = indices()[i]-1;
        for (j=0;j<sizePlace;j++) {
            currentData += parentStructArray[j]->dataStructureSize();
            currentData = parentFormat->alignField(j, currentData);
        }
        bcopy(dataStruct+currentData, (char*) &size, sizeof(int));
        arraySize *= size;
    }

    return arraySize;
}

/*****************************************************************************
 *
 * FUNCTION: int elementSize(format) 
 *
 * DESCRIPTION:
 * Returns the size (ALength) of the format's element.
 * If the format is a structured type, returns 0 unless all the elements
 * have the same length.
 *
 * INPUTS: IPFormat* format;
 *
 * OUTPUTS: int
 *
 *****************************************************************************/

int IPLengthFormat::elementSize() const
{
    return length();
}

int IPPrimFormat::elementSize() const 
{
    return a_length(NULL, 0, NULL, 0);
}

int IPPointerFormat::elementSize() const 
{
    return sizeof(char*);
}

int IPVarArrayFormat::elementSize() const 
{
    return sizeof(char*);
}

int IPFixedArrayFormat::elementSize() const
{
    return format()->elementSize();
}

int IPStructFormat::elementSize() const
{
    int firstSize, i;
    firstSize = elements()[0]->elementSize();
    if (firstSize != 0) {
        for (i=1; i<num_elements(); i++) {
            if (firstSize != elements()[i]->elementSize())
                return 0;
        }
    }
    return firstSize;
}

/*****************************************************************************
 *
 * FUNCTION: int fixedLengthFormat(format) 
 *
 * DESCRIPTION:
 * Returns 1 (1) if the Format contains no pointers.
 * (i.e., it is composed only of simple types).
 *
 * INPUTS: IPFormat* format;
 *
 * OUTPUTS: int
 *
 *****************************************************************************/

int IPLengthFormat::fixedLengthFormat() const
{
    return 1;
}

int IPPrimFormat::fixedLengthFormat() const
{
    return simple_type();
}

int IPPointerFormat::fixedLengthFormat() const
{
    return 0;
}

int IPVarArrayFormat::fixedLengthFormat() const
{
    return 0;
}

int IPFixedArrayFormat::fixedLengthFormat() const
{
    return format()->fixedLengthFormat();
}

int IPStructFormat::fixedLengthFormat() const
{
    int i, subelementSize=0;

    /* For compatibility between Sun3's and Sun4's, in addition to all
     *  being simple types, the subelements of a structure must be the
     *  same size in order for the structure to be considered "fixed length" 
     */
    for (i=0; i<num_elements(); i++) {
        if (!elements()[i]->fixedLengthFormat()) {
            return 0;
        } else if (i==0) { /* First element */
            subelementSize = elements()[0]->elementSize();
            if (subelementSize == 0) return 0;
        } else if (subelementSize != elements()[i]->elementSize())
            return 0;
    }
    return 1;
}

/*****************************************************************************
 *
 * FUNCTION: int flatNoPaddingFormat(format) 
 *
 * DESCRIPTION:
 * Returns 1 (1) if the Format contains no pointers.
 * (i.e., it is composed only of simple types).
 *
 * INPUTS: IPFormat* format;
 *
 * OUTPUTS: int
 *
 *****************************************************************************/

int IPLengthFormat::flatNoPaddingFormat() const
{
    return 1;
}

int IPPrimFormat::flatNoPaddingFormat() const
{
    return simple_type();
}

int IPPointerFormat::flatNoPaddingFormat() const
{
    return 0;
}

int IPVarArrayFormat::flatNoPaddingFormat() const
{
    return 0;
}

int IPFixedArrayFormat::flatNoPaddingFormat() const
{
    return format()->flatNoPaddingFormat();
}

int IPStructFormat::flatNoPaddingFormat() const
{
    int i, subelementSize=0;

    /* For compatibility between Sun3's and Sun4's, in addition to all
     *  being simple types, the subelements of a structure must be the
     *  same size in order for the structure to be considered "fixed length" 
     */
    for (i=0; i<num_elements(); i++) {
        if (!elements()[i]->flatNoPaddingFormat()) {
            return 0;
        }
        else if (i==0) { /* First element */
            subelementSize = elements()[0]->elementSize();
            if (subelementSize == 0) return 0;
        }
        else if (subelementSize != elements()[i]->elementSize())
            return 0;
    }
    return 1;
}

/*****************************************************************************
 *
 * FUNCTION: int sameFixedSizeDataBuffer(format) 
 *
 * DESCRIPTION:
 * Returns 1 (1) if the Format contains no pointers.
 * (i.e., it is composed only of simple types).
 *
 * INPUTS: IPFormat* format;
 *
 * OUTPUTS: int
 *
 *****************************************************************************/

int IPFormat::cache_fixed_size(int fs) const
{
    IPFormat* cache = (IPFormat*) this;
    cache->_fixed_size_data_buffer = fs;

    return fs;
}

int IPLengthFormat::sameFixedSizeDataBuffer() const
{ 
    if (fixed_size_cache() != -1)
        return fixed_size_cache();

    cache_fixed_size(1);
    return 1;
}

int IPPrimFormat::sameFixedSizeDataBuffer() const
{ 
    if (fixed_size_cache() != -1)
        return fixed_size_cache();

    return cache_fixed_size(simple_type());
}

int IPPointerFormat::sameFixedSizeDataBuffer() const
{ 
    if (fixed_size_cache() != -1)
        return fixed_size_cache();
    return cache_fixed_size(0);
}

int IPVarArrayFormat::sameFixedSizeDataBuffer() const
{ 
    if (fixed_size_cache() != -1)
        return fixed_size_cache();
    return cache_fixed_size(0);
}

int IPFixedArrayFormat::sameFixedSizeDataBuffer() const
{ 
    if (fixed_size_cache() != -1)
        return fixed_size_cache();
    return cache_fixed_size(format()->sameFixedSizeDataBuffer());
}

int IPStructFormat::sameFixedSizeDataBuffer() const
{ 
    if (fixed_size_cache() != -1)
        return fixed_size_cache();

    int i, offset=0;

    for (i=0; i<num_elements(); i++) {
        if (!elements()[i]->sameFixedSizeDataBuffer()) 
            return cache_fixed_size(0);

        /* Need to find out if there is any padding needed.*/
        offset = offset + elements()[i]->dataStructureSize();
        if (alignField(i, offset) != offset) 
            return cache_fixed_size(0);
    }
    return cache_fixed_size(1);
}


/*****************************************************************************
 *
 * FUNCTION: int mostRestrictiveElement(format) 
 *
 * DESCRIPTION:
 * For sun4/pmax version: Returns the longest element (ALength) of the format.
 * For Mach/486 version: Returns the longest element (ALength) of the format,
 *                       up to length of integer (4-bytes).
 *
 * INPUTS: const IPFormat* format;
 *
 * OUTPUTS: int
 *
 *****************************************************************************/

#if ((ALIGN & ALIGN_LONGEST) || (ALIGN & ALIGN_INT))
int IPLengthFormat::mostRestrictiveElement() const
{
    return length();
}

int IPPrimFormat::mostRestrictiveElement() const
{
#if (ALIGN & ALIGN_LONGEST)
    return r_length(NULL, 0, NULL, 0);
#elif (ALIGN & ALIGN_INT)
    int maxSize;
    maxSize = r_length(NULL, 0, NULL, 0);
    return (maxSize < (int) sizeof(int) ? maxSize : sizeof(int));
#else
    /* should never get here. */
#endif
}

int IPPointerFormat::mostRestrictiveElement() const
{
    return sizeof(char*);
}

int IPVarArrayFormat::mostRestrictiveElement() const
{
    return sizeof(char*);
}

int IPFixedArrayFormat::mostRestrictiveElement() const
{
    return format()->mostRestrictiveElement();
}

int IPStructFormat::mostRestrictiveElement() const
{
    int maxSize=0, nextSize, i;
    maxSize = elements()[0]->mostRestrictiveElement();
    for (i=1; i<num_elements(); i++) {
        nextSize = elements()[i]->mostRestrictiveElement();
        if (nextSize > maxSize) maxSize = nextSize;
    }
#if (ALIGN & ALIGN_LONGEST)
    return maxSize;
#elif (ALIGN & ALIGN_INT)
    return (maxSize < (int) sizeof(int) ? maxSize : sizeof(int));
#endif
}
#endif

/*****************************************************************************
 *
 * FUNCTION: int alignField(format, currentField, currentDataSize)
 *
 * DESCRIPTION:
 * Returns either "currentDataSize" or 1 + "currentDataSize", to reflect
 * how C would align fields in a structure.
 *
 * Sun3 version:
 * This function works on the (empirical) model that the C compiler used on the
 * Sun3s aligns fields in structures on word boundaries (unless the next 
 * field in the structure is an odd number of bytes).
 *
 * sun4/pmax version:
 * Fields must be aligned on the "appropriate" boundaries - e.g., ints on 4
 * byte boundaries, doubles on 8 byte boundaries.  Structures are padded at
 * the end to align with the most restrictive (longest) field in the structure.
 *
 * Mach/486 version:
 * Based on observation with the Mach cc compiler on 486 (and 386) machines.
 * Similar to sun4 version: Fields must be aligned on the "appropriate" 
 * boundaries, except the relevant boundaries are 1-byte (char), 
 * 2-bytes (short),
 * and 4-bytes (everything else).  Structures are padded at the end 
 * to align with the most restrictive (longest) field in the structure.
 *
 * INPUTS:
 * IPFormat* format; 
 * int currentField, currentDataSize;
 *
 * OUTPUTS: 
 *
 *****************************************************************************/

int IPLengthFormat::alignCheck() const 
{
    return ODDPTR(length());
}

int IPPrimFormat::alignCheck() const
{
    return ODDPTR(a_length(NULL, 0, NULL, 0));
}

int IPStructFormat::alignField(int currentField, int currentDataSize) const
{ 
#if (ALIGN & ALIGN_WORD)
    int nextField;
    IPFormat* nextFormat;
    IPFormat** formatArray; 
    
    if (!ODDPTR(currentDataSize)) 
        return currentDataSize;
    else {
        formatArray = elements();
        nextField = 1 + currentField;
        if (nextField == num_elements())
            /* end of structure */
            return 1+currentDataSize;
        else {
            nextFormat = formatArray[nextField];
            if (nextFormat->alignCheck())
                return currentDataSize;
            else return 1+currentDataSize;
        }
    }
#elif ((ALIGN & ALIGN_LONGEST) | (ALIGN & ALIGN_INT))
    int nextField, appropriateSize, rem;
    IPFormat** formatArray;   
    
    formatArray = elements();
    nextField = 1+currentField;
    if (nextField == num_elements()) {
        /* end of structure;pad to appropriate boundary of longest subelement*/
        appropriateSize = mostRestrictiveElement();
    }
    else {
        /* on Sparc (and Mach/486 machines), element must start on boundary
           compatible with size of largest element within the sub-structure */
        appropriateSize = formatArray[nextField]->mostRestrictiveElement();
    }
    /* Round up to the next even multiple of "appropriateSize" */
    rem = currentDataSize % appropriateSize;
    if (rem != 0) 
        currentDataSize += appropriateSize - rem;
    
    return currentDataSize;
#else
    /* Should be an error. */
#endif
}

/* returns the aligned index at which this format must start */
int IPFormat::alignFormat(int currentDataSize) const
{ 
#if (ALIGN & ALIGN_WORD)
    if (!ODDPTR(currentDataSize)) 
        return currentDataSize;
    else {
        if (alignCheck())
            return currentDataSize;
        else return 1+currentDataSize;
    }
#elif ((ALIGN & ALIGN_LONGEST) | (ALIGN & ALIGN_INT))
    int appropriateSize, rem;

    /* on Sparc (and Mach/486 machines), element must start on boundary
       compatible with size of largest element within the sub-structure */
    appropriateSize = mostRestrictiveElement();

    /* Round up to the next even multiple of "appropriateSize" */
    rem = currentDataSize % appropriateSize;
    if (rem != 0) 
        currentDataSize += appropriateSize - rem;
    
    return currentDataSize;
#else
    /* Should be an error. */
#endif
}

/*****************************************************************************
 *
 * FUNCTION: IPFormat* optimizeFormatter(format) 
 *
 * DESCRIPTION:
 * If the formatter (or any sub-formatter of it) is a fixed length format, 
 * replace the definition with the appropriate length format.
 * For example, "{{int, float}, string}" gets optimized to "{4, 4, string}"
 * and "{int, float, [int:3]}" gets optimized to "20".
 *
 * Returns the new formatter, or the original one if no change occurs.
 *
 * INPUTS: IPFormat* format;
 *
 * OUTPUTS: IPFormat*
 *
 *****************************************************************************/

IPFormat* IPFormat::optimizeFormatter()
{
    return new IPLengthFormat(_parser, dataStructureSize());
}

IPFormat* IPStructFormat::optimizeFormatter()
{
    int i;
    IPFormat** formatArray;

    formatArray = elements();
    for(i=0;i < num_elements();i++) 
        formatArray[i] = formatArray[i]->optimizeFormatter();

    return this;
}
    
IPFormat* IPPointerFormat::optimizeFormatter()
{
    _format = _format->optimizeFormatter();

    return this;
}

IPFormat* IPVarArrayFormat::optimizeFormatter()
{
    _format = _format->optimizeFormatter();

    return this;
}

IPFormat* IPFixedArrayFormat::optimizeFormatter()
{
    _format = _format->optimizeFormatter();

    return this;
}

/*****************************************************************************
 *
 * FUNCTION: IPSizesType bufferSize1(format, dataStruct, dStart, parentFormat)
 *
 * DESCRIPTION:
 * "ParentFormat" is needed by SelfPtr ("*!") and VarArray ("<..>") formatters.
 *  Both these formatters can only be embedded in a Struct format ("{...}").
 *
 * INPUTS:
 * const IPFormat* format, parentFormat;
 * char* dataStruct; 
 * int dStart; 
 *
 * OUTPUTS: IPSizesType
 *
 *****************************************************************************/

IPSizesType IPLengthFormat::bufferSize(const char*, int, const IPStructFormat*,
                                       int) const
{
    IPSizesType sizes;

    sizes.buffer = length();
    sizes.data = length();

    return sizes;
}

IPSizesType IPPrimFormat::bufferSize(const char* dataStruct, int dStart,
                                     const IPStructFormat*, int) const
{
    IPSizesType sizes;

    sizes.buffer = e_length((char *)dataStruct, dStart, NULL, 0);
    sizes.data = a_length(NULL, 0, NULL, 0);
    return sizes;
}

IPSizesType IPPointerFormat::bufferSize(const char* dataStruct, int dStart,
                                        const IPStructFormat* parentFormat,
                                        int struct_start) const
{
    IPSizesType sizes;
    int bufferSize;
    char* structPtr;
    const IPFormat* nextFormat;

    structPtr = REF(char*, dataStruct, dStart);
    bufferSize = sizeof(char);
    if (structPtr) { 
        nextFormat = choose_ptr_format(parentFormat);
        sizes = nextFormat->bufferSize(structPtr, 0, NULL, struct_start);
        bufferSize += sizes.buffer;
    }
    sizes.data = sizeof(char*);
    sizes.buffer = bufferSize;

    return sizes;
}

IPSizesType IPStructFormat::bufferSize(const char* dataStruct, int dStart,
                                       const IPStructFormat*, int) const
{
    IPSizesType sizes;
    IPFormat** formatArray;
    int i, bufferSize, currentData;

    bufferSize = 0;
    currentData = dStart;

    formatArray = elements();
    for(i=0;i < num_elements();i++) {
        sizes = formatArray[i]->bufferSize(dataStruct, currentData,
                                           this, dStart);
        bufferSize += sizes.buffer;
        currentData = alignField(i, currentData+sizes.data);
    }

    sizes.buffer = bufferSize;
    sizes.data = currentData - dStart;
    return sizes;
}

IPSizesType IPFixedArrayFormat::bufferSize(const char* dataStruct, int dStart,
                                           const IPStructFormat* parentFormat,
                                           int struct_start) const
{
    IPSizesType sizes;
    int i, bufferSize, currentData, arraySize, elements;
    IPFormat* nextFormat;

    bufferSize = 0;
    currentData = dStart;

    arraySize = fixedArraySize();
    nextFormat = format();
    if (nextFormat->sameFixedSizeDataBuffer()) {
        elements = arraySize * nextFormat->dataStructureSize();
        bufferSize += elements;
        currentData += elements;
    } else {
        for(i=0;i < arraySize;i++) {
            sizes = nextFormat->bufferSize(dataStruct, currentData,
                                           parentFormat, struct_start);
            bufferSize += sizes.buffer;
            currentData += sizes.data;
        }
    }

    sizes.buffer = bufferSize;
    sizes.data = currentData - dStart;
    return sizes;
}

IPSizesType IPVarArrayFormat::bufferSize(const char* dataStruct, int dStart,
                                         const IPStructFormat* parentFormat,
                                         int struct_start) const
{
    IPSizesType sizes;
    int i, bufferSize, currentData, arraySize, structStart;
    IPFormat* nextFormat;
    char* structPtr;

    bufferSize = 0;
    currentData = dStart;

    nextFormat = format();
    arraySize = array_size(parentFormat, dataStruct, struct_start);
    bufferSize += sizeof(int); /* for the size of the array */
    if (nextFormat->sameFixedSizeDataBuffer())
        bufferSize += arraySize * nextFormat->dataStructureSize();
    else {
        structPtr = REF(char*, dataStruct, currentData);
        structStart = 0;
        if (structPtr != NULL) {
            for(i=0;i < arraySize;i++) {
                sizes = nextFormat->bufferSize((char*) structPtr, structStart,
                                               parentFormat, struct_start);
                bufferSize += sizes.buffer;
                structStart += sizes.data;
            }
        }
    }
    /* skip over the pointer to the array */
    currentData += sizeof(char*);

    sizes.buffer = bufferSize;
    sizes.data = currentData - dStart;
    return sizes;
}

/*****************************************************************************
 *
 * FUNCTION: int dataStructureSize(format) 
 *
 * DESCRIPTION:
 *
 * INPUTS: IPFormat* format;
 *
 * OUTPUTS: int
 *
 *****************************************************************************/

int IPFormat::dataStructureSize() const
{ 
    int size;

    if (_data_structure_size != -1)
        return _data_structure_size;

    size = data_structure_size();
    IPFormat* cache = (IPFormat*) this;
    cache->_data_structure_size = size;
    return size;
}

int IPLengthFormat::data_structure_size() const 
{
    return length();
}

int IPPrimFormat::data_structure_size() const
{
    return a_length (NULL, 0, NULL, 0);
}

int IPVarArrayFormat::data_structure_size() const
{
    return sizeof(char*);
}

int IPPointerFormat::data_structure_size() const
{
    return sizeof(char*);
}

int IPStructFormat::data_structure_size() const
{
    int size = 0;
    int i;
    IPFormat** formatArray;

    formatArray = elements();
    for(i=0;i < num_elements();i++) 
        size = alignField(i, size+formatArray[i]->dataStructureSize());

    return size;
}

int IPFixedArrayFormat::data_structure_size() const
{
    return fixedArraySize() * format()->dataStructureSize();
}


/*****************************************************************************
 *
 * FUNCTION: IPSizesType transferToBuffer(format, dataStruct, dStart, buffer, 
 *                                       bStart, parentFormat)
 *
 * DESCRIPTION:
 *
 * INPUTS:
 * const IPFormat* format, parentFormat;
 * void *dataStruct; 
 * int dStart, bStart; 
 * char *buffer;
 *
 * OUTPUTS: IPSizesType
 *
 *****************************************************************************/

IPSizesType IPLengthFormat::transferToBuffer(char* dataStruct, int dStart,
                                             char *buffer, int bStart,
                                             const IPStructFormat*, int) const
{ 
    IPSizesType sizes;

    TO_BUFFER_AND_ADVANCE(dataStruct+dStart, buffer, bStart, 
                          length());

    sizes.data = length();
    sizes.buffer = length();
    return sizes;
}
    
IPSizesType IPPrimFormat::transferToBuffer(char* dataStruct, int dStart,
                                           char *buffer, int bStart,
                                           const IPStructFormat*, int) const
{ 
    IPSizesType sizes;

    sizes.buffer = encode(dataStruct, dStart, buffer, bStart);
    sizes.data = a_length(NULL, 0, NULL, 0);

    return sizes;
}

IPSizesType IPPointerFormat::transferToBuffer(char* dataStruct, int dStart,
                                              char *buffer, int bStart,
                                              const IPStructFormat* parentFormat,
                                              int) const
{ 
    IPSizesType sizes;
    const IPFormat* nextFormat;
    char* structPtr;
    int currentByte = bStart;

    structPtr = REF(char*, dataStruct, dStart);
    /* Z means data, 0 means NULL*/
    buffer[currentByte] = (structPtr) ? 'Z' : '\0';
    currentByte += sizeof(char);
    
    if (structPtr) { 
        nextFormat = choose_ptr_format(parentFormat);
        sizes =
            nextFormat->transferToBuffer(structPtr, 0, buffer, currentByte,
                                         NULL, 0);
        currentByte += sizes.buffer;
    }

    sizes.buffer = currentByte - bStart;
    sizes.data = sizeof(char*);
    return sizes;
}

IPSizesType IPStructFormat::transferToBuffer(char* dataStruct, int dStart,
                                             char *buffer, int bStart,
                                             const IPStructFormat*, int) const
{ 
    IPSizesType sizes;
    IPFormat** formatArray;
    int i;
    int currentByte = bStart;
    int currentData = dStart;

    formatArray = elements();
    for(i=0;i < num_elements();i++) {
        sizes =
            formatArray[i]->transferToBuffer(dataStruct, currentData, buffer,
                                             currentByte, this, dStart);
        currentByte += sizes.buffer;
        currentData = alignField(i, currentData+sizes.data);
    }

    sizes.data = currentData - dStart;
    sizes.buffer = currentByte - bStart;
    return sizes;
}

IPSizesType IPFixedArrayFormat::transferToBuffer(char* dataStruct, int dStart,
                                                 char *buffer, int bStart,
                                                 const IPStructFormat* parentFormat,
                                                 int struct_start) const
{ 
    IPSizesType sizes;
    IPFormat* nextFormat;
    int arraySize, elements;
    int currentByte = bStart;
    int currentData = dStart;

    arraySize = fixedArraySize();
    nextFormat = format();
    if (nextFormat->sameFixedSizeDataBuffer()) {
        elements = arraySize * nextFormat->dataStructureSize();
        bcopy(dataStruct+currentData, buffer+currentByte, elements);
        currentByte += elements;
        currentData += elements;
    } else {
        sizes =
            nextFormat->transfer_array_to_buffer(arraySize,
                                                 dataStruct, currentData, 
                                                 buffer, currentByte,
                                                 parentFormat, struct_start);
        currentData += sizes.data;
        currentByte = sizes.buffer;
    }

    sizes.data = currentData - dStart;
    sizes.buffer = currentByte - bStart;
    return sizes;
}

IPSizesType
IPFormat::transfer_array_to_buffer(int arraySize,
                                   char* dataStruct, int currentData,
                                   char *buffer, int currentByte,
                                   const IPStructFormat* parentFormat,
                                   int struct_start) const
{
    int i;
    IPSizesType sizes;
    
    for(i=0;i < arraySize;i++) {
        sizes = transferToBuffer(dataStruct, currentData, buffer, currentByte,
                                 parentFormat, struct_start);
        currentByte += sizes.buffer;
        currentData += sizes.data;
    }

    sizes.buffer = currentByte;
    sizes.data = currentData;
    
    return sizes;
}

IPSizesType
IPPrimFormat::transfer_array_to_buffer(int arraySize,
                                       char* dataStruct, int currentData,
                                       char *buffer, int currentByte,
                                       const IPStructFormat*, int) const
{
    int alength, i;
    IPSizesType sizes;

    if (simple_type()) {
        alength = a_length(NULL, 0, NULL, 0);
        switch (alength) {
          case sizeof(char): 
            /* Characters, do not need to do anything but copy. */
            {
                register char *bytes = (buffer+currentByte);
                register char *datas = (dataStruct+currentData);
                for(i=0;i < arraySize;i++) {
                    charToNetBytes(*(char *)(datas),(bytes));
                    bytes += sizeof(char);
                    datas += sizeof(char);
                }
                currentByte += sizeof(char) * arraySize;
                currentData += sizeof(char) * arraySize;
                break;
            }
          case sizeof(short): /* Shorts, . */ 
            if ((buffer+currentByte)==(dataStruct+currentData)) {
                /* The copy is in place, just need to rearrange. */
                register unsigned short *datas = 
                    (unsigned short *)(dataStruct+currentData);
                for(i=0; i<arraySize; i++) {
                    *datas =
                        (short)(*datas << 8) | (short)(*datas >> 8);
                }
            } else {
                register char *bytes = (buffer+currentByte);
                register char *datas = (dataStruct+currentData);
                for(i=0;i < arraySize;i++) {
                    shortToNetBytes(*(short *)(datas),(bytes));
                    bytes += sizeof(short);
                    datas += sizeof(short);
                }
            }
            currentByte += sizeof(short) * arraySize;
            currentData += sizeof(short) * arraySize;
            break;
          case sizeof(int): 
            if ((buffer+currentByte) == (dataStruct+currentData)) {
                /* The copy is in place, just need to rearrange. */
                register unsigned int *datas = 
                    (unsigned int *) (dataStruct+currentData);
                for(i=0; i<arraySize; i++) {
                    *datas = (((*datas & 0x000000FF) << 24) |
                              ((*datas >> 24 ) & 0x000000FF) |
                              ((*datas & 0x0000FF00) << 8) |
                              ((*datas >> 8 ) & 0x0000FF00));
                }
            } else {
                register char *bytes = (buffer+currentByte);
                register char *datas = (dataStruct+currentData);
                for(i=0;i < arraySize;i++) {
                    intToNetBytes(*(int *)(datas),(bytes));
                    bytes += sizeof(int);
                    datas += sizeof(int);
                }
            }
            currentByte += sizeof(int) * arraySize;
            currentData += sizeof(int) * arraySize;
            break;
          case sizeof(double): 
            if ((buffer+currentByte) == (dataStruct+currentData)) {
                /* The copy is in place, just need to rearrange. */
                register unsigned int *datas = 
                    (unsigned int *) (dataStruct+currentData);
                for(i=0; i<2*arraySize; i++) {
                    *datas = (((*datas & 0x000000FF) << 24) |
                              ((*datas >> 24 ) & 0x000000FF) |
                              ((*datas & 0x0000FF00) << 8) |
                              ((*datas >> 8 ) & 0x0000FF00));
                }
            } else {
                char *bytes = (buffer+currentByte);
                char *datas = (dataStruct+currentData);
                for(i=0;i < arraySize;i++) {
                    doubleToNetBytes(*(double *)(datas),(bytes));
                    bytes += sizeof(double);
                    datas += sizeof(double);
                }
            }
            currentByte += sizeof(double) * arraySize;
            currentData += sizeof(double) * arraySize;
            break;
          default:
            fprintf(stderr,
                    "Unhandled primitive element of size %d \n",
                    alength);
        }
    } else {
        for(i=0;i < arraySize;i++) {
            sizes.buffer = encode(dataStruct, currentData,
                                  buffer, currentByte);
            currentData += a_length(dataStruct, currentData,
                                    buffer, currentByte);
            currentByte += sizes.buffer;
        }
    }

    sizes.buffer = currentByte;
    sizes.data = currentData;
    return sizes;
}

IPSizesType
IPVarArrayFormat::transferToBuffer(char* dataStruct, int dStart,
                                   char *buffer, int bStart,
                                   const IPStructFormat* parentFormat,
                                   int struct_start) const
{ 
    IPSizesType sizes;
    int currentByte = bStart;
    int currentData = dStart;
    IPFormat* nextFormat;
    int arraySize, elements;
    char* structPtr;

    arraySize = array_size(parentFormat, dataStruct, struct_start);
    nextFormat = format();
    
    intToNetBytes(arraySize, buffer+currentByte);
    currentByte += sizeof(int);
    structPtr = REF(char*, dataStruct, currentData);
    if (nextFormat->sameFixedSizeDataBuffer()) {
        elements = arraySize * nextFormat->dataStructureSize();
        bcopy(structPtr, buffer+currentByte, elements);
        currentByte += elements;
    } else {
        sizes =
            nextFormat->transfer_array_to_buffer(arraySize,
                                                 structPtr, 0,
                                                 buffer, currentByte,
                                                 parentFormat, struct_start);
        currentByte = sizes.buffer;
    }

    currentData += sizeof(char*);

    sizes.buffer = currentByte - bStart;
    sizes.data = currentData - dStart;
    return sizes;  
}

IPSizesType
IPLengthFormat::transferToDataStructure(char* dataStruct, int dStart,
                                        char *buffer, int bStart, int,
                                        const IPStructFormat*, int, int) const
{
    IPSizesType sizes;

    FROM_BUFFER_AND_ADVANCE(dataStruct+dStart, buffer, bStart, 
                            length());
    sizes.buffer = length();
    sizes.data = length();

    return sizes;
}

IPSizesType
IPPrimFormat::transferToDataStructure(char* dataStruct, int dStart,
                                      char *buffer, int bStart, int bSize,
                                      const IPStructFormat*,
                                      int byteOrder, int alignment) const
{
    IPSizesType sizes;

    sizes.buffer = decode(dataStruct, dStart, buffer, bStart, bSize,
                          byteOrder, alignment);
    sizes.data = a_length(NULL, 0, NULL, 0);

    return sizes;
}

IPSizesType
IPPointerFormat::transferToDataStructure(char* dataStruct, int dStart,
                                         char *buffer, int bStart, int bSize,
                                         const IPStructFormat* parentFormat,
                                         int byteOrder, int alignment) const
{
    IPSizesType sizes;
    char ptrVal;
    int currentByte = bStart;
    char* newStruct;
    const IPFormat* nextFormat;

    FROM_BUFFER_AND_ADVANCE(&ptrVal, buffer, currentByte, sizeof(char));
    if (ptrVal == '\0') 
        newStruct = NULL;
    else {
        nextFormat = choose_ptr_format(parentFormat);    
        newStruct = new char[nextFormat->dataStructureSize()];
        sizes =
            nextFormat->transferToDataStructure(newStruct, 0,
                                                buffer, currentByte, bSize,
                                                NULL, byteOrder, alignment);
        currentByte += sizes.buffer;
    }
    TO_BUFFER_AND_ADVANCE(&newStruct, dataStruct, dStart,
                          sizeof(char*));

    sizes.buffer = currentByte - bStart;
    sizes.data = sizeof(char*);
    return sizes;
}

IPSizesType
IPStructFormat::transferToDataStructure(char* dataStruct, int dStart,
                                        char *buffer, int bStart, int bSize,
                                        const IPStructFormat*,
                                        int byteOrder, int alignment) const
{
    IPSizesType sizes;
    IPFormat** formatArray;
    int currentByte = bStart;
    int currentData = dStart;
    int i;

    formatArray = elements();
    for(i=0;i < num_elements();i++) {
        sizes =
            formatArray[i]->transferToDataStructure(dataStruct, 
                                                    currentData, buffer,
                                                    currentByte, bSize,
                                                    this, byteOrder,alignment);
        currentByte += sizes.buffer;
        currentData = alignField(i, currentData+sizes.data);
    }

    sizes.buffer = currentByte - bStart;
    sizes.data = currentData - dStart;

    return sizes;
}

IPSizesType
IPFixedArrayFormat::transferToDataStructure(char* dataStruct, int dStart,
                                            char *buffer, int bStart,
                                            int bSize, const IPStructFormat*,
                                            int byteOrder, int alignment) const
{
    IPSizesType sizes;
    int currentByte = bStart;
    int currentData = dStart;
    int elements, arraySize;
    IPFormat* nextFormat;

    arraySize = fixedArraySize();
    nextFormat = format();
    if ((byteOrder == BYTE_ORDER) &&
        (nextFormat->sameFixedSizeDataBuffer())) {
        elements = arraySize * nextFormat->dataStructureSize();
        bcopy(buffer+currentByte, dataStruct+currentData, elements);
        currentByte += elements;
        currentData += elements;
    } else {
        sizes = nextFormat->transfer_array_to_data(arraySize,
                                                   dataStruct, currentData,
                                                   buffer, currentByte, bSize,
                                                   byteOrder, alignment);
        currentData = sizes.data;
        currentByte = sizes.buffer;
    }

    sizes.buffer = currentByte - bStart;
    sizes.data = currentData - dStart;

    return sizes;
}

IPSizesType IPFormat::transfer_array_to_data(int arraySize,
                                             char* dataStruct, int dStart,
                                             char* buffer, int bStart,
                                             int bSize, int byteOrder,
                                             int alignment)
{
    int i;
    IPSizesType sizes;
    int currentByte = bStart;
    int currentData = dStart;

    for(i=0;i < arraySize;i++) {
        sizes = transferToDataStructure(dataStruct, currentData,
                                        buffer, currentByte, bSize,
                                        NULL, byteOrder, alignment);
        currentByte += sizes.buffer;
        currentData += sizes.data;
    }

    sizes.buffer = currentByte;
    sizes.data = currentData;
    return sizes;
}

IPSizesType IPPrimFormat::transfer_array_to_data(int arraySize,
                                                 char* dataStruct,
                                                 int currentData,
                                                 char* buffer, int currentByte,
                                                 int bSize, int byteOrder,
                                                 int alignment)
{
    IPSizesType sizes;
    int alength;
    int inplace = DataInPlace(dataStruct, buffer, bSize);
    int i;

    if (simple_type()) {
        alength = a_length(NULL, 0, NULL, 0);
        switch (alength) {
          case sizeof(char): 
            /* Characters, do not need to do anything but copy. */
            {
                register char *bytes = (buffer+currentByte);
                register char *datas = (dataStruct+currentData);
                for(i=0;i < arraySize;i++) {
                    netBytesToChar((bytes),(char *)(datas));
                    bytes += sizeof(char);
                    datas += sizeof(char);
                }
                currentByte += sizeof(char) * arraySize;
                currentData += sizeof(char) * arraySize;
                break;
            }
          case sizeof(short): /* Shorts, . */ 
            if (inplace) {
                /* The copy is in place, just need to rearrange. */
                register unsigned short *datas = 
                    (unsigned short *)(dataStruct+currentData);
                for(i=0; i<arraySize; i++) {
                    *datas = (short)(*datas << 8) | (short)(*datas >> 8);
                }
            } else {
                register char *bytes = (buffer+currentByte);
                register char *datas = (dataStruct+currentData);
                for(i=0;i < arraySize;i++) {
                    netBytesToShort((bytes),(short *)(datas));
                    bytes += sizeof(short);
                    datas += sizeof(short);
                }
            }
            currentByte += sizeof(short) * arraySize;
            currentData += sizeof(short) * arraySize;
            break;
          case sizeof(int): 
            if (inplace) {
                /* The copy is in place, just need to rearrange. */
                register unsigned int *datas = 
                    (unsigned int *) (dataStruct+currentData);
                for(i=0; i<arraySize; i++) {
                    *datas = (((*datas & 0x000000FF) << 24) |
                              ((*datas >> 24 ) & 0x000000FF) |
                              ((*datas & 0x0000FF00) << 8) |
                              ((*datas >> 8 ) & 0x0000FF00));
                }
            } else {
                register char *bytes = (buffer+currentByte);
                register char *datas = (dataStruct+currentData);
                for(i=0;i < arraySize;i++) {
                    netBytesToInt((bytes),(int *)(datas));
                    bytes += sizeof(int);
                    datas += sizeof(int);
                }
            }
            currentByte += sizeof(int) * arraySize;
            currentData += sizeof(int) * arraySize;
            break;
          case sizeof(double): 
            if (inplace) {
                /* The copy is in place, just need to rearrange. */
                register unsigned int *datas = 
                    (unsigned int *) (dataStruct+currentData);
                for(i=0; i<2*arraySize; i++) {
                    *datas = (((*datas & 0x000000FF) << 24) |
                              ((*datas >> 24 ) & 0x000000FF) |
                              ((*datas & 0x0000FF00) << 8) |
                              ((*datas >> 8 ) & 0x0000FF00));
                }
            } else {
                char *bytes = (buffer+currentByte);
                char *datas = (dataStruct+currentData);
                for(i=0;i < arraySize;i++) {
                    netBytesToDouble((bytes),(double *)(datas));
                    bytes += sizeof(double);
                    datas += sizeof(double);
                }
            }
            currentByte += sizeof(double) * arraySize;
            currentData += sizeof(double) * arraySize;
            break;
          default:
            fprintf(stderr, "Unhanded primitive element of size %d \n",
                    alength);
        }
    } else {
        for(i=0;i < arraySize;i++) {
            sizes.buffer = decode(dataStruct, currentData,
                                  buffer, currentByte,
                                  bSize,
                                  byteOrder, alignment);
            currentData += a_length(dataStruct, currentData,
                                           buffer, currentByte);
            currentByte += sizes.buffer;
        }
    }

    sizes.buffer = currentByte;
    sizes.data = currentData;
    return sizes;
}

IPSizesType
IPVarArrayFormat::transferToDataStructure(char* dataStruct, int dStart,
                                          char *buffer, int bStart,
                                          int bSize,
                                          const IPStructFormat*,
                                          int byteOrder, int alignment) const
{
    IPSizesType sizes;
    int currentByte = bStart;
    int currentData = dStart;
    int elements, arraySize, fixed_format;
    IPFormat* nextFormat;
    char* newStruct;

    netBytesToInt(buffer+currentByte, &arraySize);
    currentByte += sizeof(int);
    nextFormat = format();

    fixed_format =
        (bSize &&
         nextFormat->sameFixedSizeDataBuffer() &&
         (currentByte == nextFormat->alignFormat(currentByte)));
    if (fixed_format)
        newStruct = ((arraySize == 0) ?
                     (char*) NULL : (buffer+currentByte));
    else
        newStruct = 
            ((arraySize == 0) ? (char*) NULL :
             new char[(arraySize*
                       nextFormat->dataStructureSize())]);
    TO_BUFFER_AND_ADVANCE(&newStruct, dataStruct, currentData, 
                          sizeof(char*));

    if (fixed_format && byteOrder == BYTE_ORDER) {
        elements = arraySize * nextFormat->dataStructureSize();
        currentByte += elements;
    } else {
        sizes = nextFormat->transfer_array_to_data(arraySize,
                                                   newStruct, 0,
                                                   buffer, currentByte, bSize,
                                                   byteOrder, alignment);
        currentByte = sizes.buffer;
    }

    sizes.buffer = currentByte - bStart;
    sizes.data = currentData - dStart;
    return sizes;
}

/*************************************************************
  
  THESE FUNCTIONS FORM THE INTERFACE TO THE REST OF THE SYSTEM
  
  *************************************************************/

int IPFormat::bufferSize(void *DataStruct) const
{ 
    IPSizesType sizes;
    
    sizes = bufferSize((char*) DataStruct, 0, NULL, 0);
    return sizes.buffer;
}

/*************************************************************/

void IPFormat::encodeData(void *DataStruct,
                          char *Buffer, int BStart) const
{
    transferToBuffer((char*) DataStruct, 0, Buffer, BStart,
                     NULL, 0); 
}

/*************************************************************/


void *IPFormat::decodeData(char *Buffer, int BStart, int BSize,
                           char *DataStruct,
                           int byteOrder, int alignment) const
{
    if (DataStruct == NULL) {
        if (BSize && sameFixedSizeDataBuffer()) {
            DataStruct = Buffer;
            if (byteOrder == BYTE_ORDER)
                return DataStruct;
        } else
            DataStruct = new char[dataStructureSize()];
    }
    
    transferToDataStructure(DataStruct, 0, Buffer, BStart, BSize,
                            NULL, byteOrder, alignment);
    return DataStruct;
}

/*****************************************************************************
 *
 * FUNCTION: freeDataElements
 *
 * DESCRIPTION: Frees the data elements "malloc"ed by the equivalent call
 *              to "TransferToDataStructure"
 *
 * INPUTS: format of data structure
 *         pointer data structure itself
 *         start of relevant part of data structure
 *         format of parent data structure (or NULL)
 *
 * OUTPUTS: number of bytes processed in the top-level dataStruct.
 *
 *****************************************************************************/

int IPLengthFormat::freeDataElements(char*, int, char*, int, const
                                     IPStructFormat*, int) const
{
    if (sameFixedSizeDataBuffer())
        return dataStructureSize();

    return length();
}

int IPPrimFormat::freeDataElements(char* dataStruct,
                                   int dStart, char* buffer, int bsize,
                                   const IPStructFormat*, int) const
{
    if (sameFixedSizeDataBuffer())
        return dataStructureSize();

    free_data(dataStruct, dStart, buffer, 0, bsize);
    return a_length(NULL, 0, NULL, 0);
}

int IPPointerFormat::freeDataElements(char* dataStruct,
                                      int dStart, char* buffer, int bsize,
                                      const IPStructFormat* parentFormat,
                                      int struct_start) const
{
    if (sameFixedSizeDataBuffer())
        return dataStructureSize();

    char** structPtr;
    const IPFormat* nextFormat;

    structPtr = &(REF(char*, dataStruct, dStart));
    if (*structPtr) {
        nextFormat = choose_ptr_format(parentFormat);    
        nextFormat->freeDataElements(*structPtr, 0, buffer, bsize,
                                     NULL, struct_start);
        delete ((char *)*structPtr);
        *structPtr = NULL;
    }

    return sizeof(char*);
}

int IPStructFormat::freeDataElements(char* dataStruct,
                                     int dStart, char* buffer, int bsize,
                                     const IPStructFormat*, int) const
{
    if (sameFixedSizeDataBuffer())
        return dataStructureSize();

    int currentData = dStart;
    IPFormat** formatArray;
    int i, size;

    formatArray = elements();
    for(i=0;i < num_elements();i++) {
        size = formatArray[i]->freeDataElements(dataStruct, currentData,
                                                buffer, bsize, this, dStart);
        currentData = alignField(i, currentData+size);
    }

    return currentData - dStart;
}
    
int IPFixedArrayFormat::freeDataElements(char* dataStruct,
                                         int dStart, char* buffer, int bsize,
                                         const IPStructFormat*,
                                         int struct_start) const
{
    if (sameFixedSizeDataBuffer())
        return dataStructureSize();

    int i, arraySize, size;
    const IPFormat* nextFormat;
    int currentData = dStart;

    arraySize = fixedArraySize();
    nextFormat = format();
    if (nextFormat->sameFixedSizeDataBuffer()) {
        currentData += arraySize * nextFormat->dataStructureSize();
    } else {
        for(i=0;i < arraySize;i++) {
            size = nextFormat->freeDataElements(dataStruct, currentData,
                                                buffer, bsize,
                                                NULL, struct_start);
            currentData += size;
        }
    }

    return currentData - dStart;
}

int IPVarArrayFormat::freeDataElements(char* dataStruct,
                                         int dStart, char* buffer, int bsize,
                                         const IPStructFormat* parentFormat,
                                         int struct_start) const
{
    if (sameFixedSizeDataBuffer())
        return dataStructureSize();

    int i, arraySize, size, structStart;
    const IPFormat* nextFormat;
    char* *structPtr;
    int currentData = dStart;

    structPtr = &(REF(char*, dataStruct, currentData));
    if (*structPtr) {
        arraySize = array_size(parentFormat, dataStruct, struct_start);
        nextFormat = format();
        if (!nextFormat->sameFixedSizeDataBuffer()) {
            structStart = 0;
            for(i=0;i < arraySize;i++) {
                size = nextFormat->freeDataElements(*structPtr,
                                                    structStart,
                                                    buffer, bsize,
                                                    parentFormat,
                                                    struct_start);
                structStart += size;
            }
        }
        if (!DataInPlace(*structPtr, buffer, bsize))
            delete((char *)*structPtr);
        *structPtr = NULL;
    }
    currentData += sizeof(void *);

    return currentData - dStart;
}

void IPFormat::freeDataStructure(void *dataStruct,
                                 char* buffer, int bsize) const
{
    freeDataElements((char*) dataStruct, 0, buffer, bsize, NULL, 0);

    if (!DataInPlace((char*) dataStruct, buffer, bsize))
    {
        // ENDO - MEM LEAK
        //delete dataStruct;
        delete [] (char *)dataStruct;
        dataStruct = NULL; // Added by ENDO
    }
}

class IPPrintFormatAction : public IPFormatAction {
  public:
    IPPrintFormatAction(FILE* f) { _file = f; };
    
    virtual void start_struct(int) const { fprintf(_file, "{ "); }
    virtual void end_struct() const { fprintf(_file, "} "); }
    virtual void start_fixed_array(int, const IPFormat*) const
        { fprintf(_file, "[ "); }
    virtual void end_fixed_array() const { fprintf(_file, "] "); }
    virtual void start_var_array(int size, const IPFormat*)
        const { fprintf(_file, "< %d : ", size); }
    virtual void end_var_array() const { fprintf(_file, "> "); }
    virtual void start_pointer(const IPFormat* f) const
        { if (f) fprintf(_file, "*"); else fprintf(_file, "NULL "); }
    virtual void length_act(int l, char*) const
        { fprintf(_file, "[ %d bytes ] ", l); }
    virtual void char_act(char c) const { fprintf(_file, "%c ", c); }
    virtual void int_act(int i) const { fprintf(_file, "%d ", i); }
    virtual void short_act(short s) const { fprintf(_file, "%d ", s); }
    virtual void long_act(long i) const { fprintf(_file, "%ld ", i); }
    virtual void double_act(double d) const { fprintf(_file, "%f ", d); }
    virtual void float_act(float f) const { fprintf(_file, "%f ", f); }
    virtual void string_act(const char* s) const { fprintf(_file, "%s ", s); }
    virtual void bool_act(int b) const
        { fprintf(_file, "%c ", (b ? 't' : 'f')); }

  private:
    FILE* _file;
};
    

void IPFormat::PrintData(void* formatted) const
{
    IPPrintFormatAction act(stdout);
    actData(&act, (char*) formatted, 0, NULL, 0);
}

int IPLengthFormat::actData(const IPFormatAction* act,
                            char* dataStruct, int dStart,
                            const IPStructFormat*, int) const
{
    act->length_act(length(), dataStruct+dStart);
    return length();
}

int IPPointerFormat::actData(const IPFormatAction* act,
                             char* dataStruct, int dStart,
                             const IPStructFormat* parentFormat,
                             int struct_start) const
{
    char* structPtr;
    const IPFormat* nextFormat;


    structPtr = REF(char*, dataStruct, dStart);
    if (structPtr) {
        nextFormat = choose_ptr_format(parentFormat);    
        act->start_pointer(nextFormat);
        nextFormat->actData(act, structPtr, 0, NULL, struct_start);
    } else
        act->start_pointer(NULL);
    return sizeof(char*);
}

int IPStructFormat::actData(const IPFormatAction* act,
                            char* dataStruct, int dStart,
                            const IPStructFormat*, int) const
{
    int currentData = dStart;
    IPFormat** formatArray;
    int size, i;

    formatArray = elements();
    act->start_struct(num_elements());
    
    for(i=0; i < num_elements();i++) {
        size = formatArray[i]->actData(act, dataStruct, currentData, this,
                                       dStart);
        currentData = alignField(i, currentData+size);
    }

    act->end_struct();

    return currentData - dStart;
}

int IPFixedArrayFormat::actData(const IPFormatAction* act, char* dataStruct,
                                int dStart, const IPStructFormat*,
                                int struct_start) const
{
    int size, i, arraySize;
    IPFormat* nextFormat;
    int currentData = dStart;

    arraySize = fixedArraySize();
    nextFormat = format();
    act->start_fixed_array(arraySize, nextFormat);
    for(i=0;i < arraySize;i++) {
        nextFormat->actData(act, dataStruct, currentData, NULL, struct_start);
        size = nextFormat->dataStructureSize();
        currentData += size;
    }

    act->end_fixed_array();

    return currentData - dStart;
}

int IPVarArrayFormat::actData(const IPFormatAction* act,
                              char* dataStruct, int dStart,
                              const IPStructFormat* parentFormat,
                              int struct_start) const
{
    char* structPtr;
    int size, i, arraySize, structStart;
    IPFormat* nextFormat;

    structPtr = REF(char*, dataStruct, dStart);
    if (structPtr) {
        arraySize = array_size(parentFormat, dataStruct, struct_start);
        nextFormat = format();
        act->start_var_array(arraySize, nextFormat);
        structStart = 0;
        for(i=0;i < arraySize;i++) {
            size = nextFormat->actData(act, structPtr, structStart, NULL,
                                       struct_start);
            structStart += size;
        }
        act->end_var_array();
    } else {
        act->start_var_array(0, NULL);
        act->end_var_array();
    }

    return sizeof(char*);
}

int IPFormat::equals(const IPFormat* f) const 
{
    return (f == this) || (f->type() == type());
}

int IPLengthFormat::equals(const IPFormat* f) const
{
    return ((f == this) ||
            ((f->type() == LengthFMT) &&
             ((IPLengthFormat*) f)->length() == length()));
}

int IPPointerFormat::equals(const IPFormat* f) const
{
    if (f == this)
        return 1;

    if (f->type() != PointerFMT)
        return 0;

    return format()->equals(((IPPointerFormat*) f)->format());
}

int IPFixedArrayFormat::equals(const IPFormat* f) const
{
    if (f == this)
        return 1;
    if (f->type() != type())
        return 0;
    const IPFixedArrayFormat* fa = (IPFixedArrayFormat*) f;
    if (num_dimensions() != fa->num_dimensions())
        return 0;

    int i;
    for (i=0;i<num_dimensions();i++)
        if (dimensions()[i] != fa->dimensions()[i])
            return 0;

    return format()->equals(fa->format());
}

int IPVarArrayFormat::equals(const IPFormat* f) const
{
    if (f == this)
        return 1;
    if (f->type() != type())
        return 0;

    IPVarArrayFormat* va = (IPVarArrayFormat*) f;
    if (va->num_indices() != num_indices())
        return 0;

    return format()->equals(va->format());
}

int IPStructFormat::equals(const IPFormat* f) const
{
    if (f == this)
        return 1;
    if (f->type() != type())
        return 0;

    IPStructFormat* s = (IPStructFormat*) f;
    if (s->num_elements() != num_elements())
        return 0;

    int i;
    for (i=0;i<num_elements();i++) 
        if (!elements()[i]->equals(s->elements()[i]))
            return 0;

    return 1;
}
