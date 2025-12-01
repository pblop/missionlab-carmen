#ifndef ip_formmacros_h
#define ip_formmacros_h

#include <ipt/formconfig.h>

#define REF(type, datastruct, dstart) *(type *)(datastruct+dstart)

#define TO_BUFFER_AND_ADVANCE(data, buffer, bstart, length) \
{bcopy((char *)data, (buffer)+bstart, length); bstart += length;}

#define FROM_BUFFER_AND_ADVANCE(data, buffer, bstart, length) \
{bcopy((buffer)+bstart, (char *)data, length); bstart += length;}

#define ODDPTR(x) ((x) & 1)

#define WORD_SIZE 4
#define WORD_SIZEP(x) (((int)(x) & (WORD_SIZE-1)) == 0)

#define shortToBytes(s,shortBytes) \
{\
   register const unsigned char *src= ((unsigned char *) &(s));\
   register unsigned char *dest= (unsigned char *)(shortBytes);\
   *dest++ = *src++;\
   *dest = *src;\
}

#define intToBytes(i,intBytes) \
{\
   register const unsigned char *src= ((unsigned char *) &(i));\
   register unsigned char *dest= (unsigned char *)(intBytes);\
/*   if ((unsigned char *)(intBytes) == (unsigned char *)&(i))*/\
/*   printf("Same ");*/\
/*   printf("before %08x ",(int)(i));*/\
/*   printf("%02x",(int)dest[0]);*/\
/*   printf("%02x",(int)dest[1]);*/\
/*   printf("%02x",(int)dest[2]);*/\
/*   printf("%02x",(int)dest[3]);*/\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src;\
/*   printf(" After %08x \n",(int)(i));*/\
/*   dest -= 3;*/\
/*   printf("%02x",(int)dest[0]);*/\
/*   printf("%02x",(int)dest[1]);*/\
/*   printf("%02x",(int)dest[2]);*/\
/*   printf("%02x",(int)dest[3]);*/\
/*   printf(" intToBytes\n");*/\
}

#define longToBytes(l,longBytes) \
{\
   register const unsigned char *src= ((unsigned char *) &(l));\
   register unsigned char *dest= (unsigned char *)(longBytes);\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src;\
}

#define floatToBytes(f,floatBytes) \
{\
   register const unsigned char *src= ((unsigned char *) &(f));\
   register unsigned char *dest= (unsigned char *)(floatBytes);\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src;\
}

#define doubleToBytes(d,doubleBytes) \
{\
   register const unsigned char *src= ((unsigned char *) &(d));\
   register unsigned char *dest= (unsigned char *)(doubleBytes);\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src;\
}

#define bytesToShort(shortBytes, sPtr) \
{\
   register const unsigned char *src= ((unsigned char *) (shortBytes));\
   register unsigned char *dest= (unsigned char *)(sPtr);\
   *dest++ = *src++;\
   *dest = *src;\
}

#define bytesToInt(intBytes, iPtr) \
{\
   register const unsigned char *src= ((unsigned char *) (intBytes));\
   register unsigned char *dest= (unsigned char *)(iPtr);\
/*   if ((unsigned char *)(intBytes) == (unsigned char *)(iPtr))*/\
/*   printf("Same ");*/\
/*   printf("before ");*/\
/*   printf("%02x",(int)src[0]);*/\
/*   printf("%02x",(int)src[1]);*/\
/*   printf("%02x",(int)src[2]);*/\
/*   printf("%02x",(int)src[3]);*/\
/*   printf(" %08x ", *iPtr);*/\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src;\
/*   printf(" after ");*/\
/*   src -= 3;*/\
/*   printf("%02x",(int)src[0]);*/\
/*   printf("%02x",(int)src[1]);*/\
/*   printf("%02x",(int)src[2]);*/\
/*   printf("%02x",(int)src[3]);*/\
/*   printf(" %08x ",*iPtr);*/\
/*   printf(" bytesToInt\n");*/\
}

#define bytesToLong(longBytes, lPtr) \
{\
   register const unsigned char *src= ((unsigned char *) (longBytes));\
   register unsigned char *dest= (unsigned char *)(lPtr);\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src;\
}

#define bytesToFloat(floatBytes, fPtr) \
{\
   register const unsigned char *src= ((unsigned char *) (floatBytes));\
   register unsigned char *dest= (unsigned char *)(fPtr);\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src;\
}

#define bytesToDouble(doubleBytes,dPtr) \
{\
   register const unsigned char *src= ((unsigned char *) (doubleBytes));\
   register unsigned char *dest= (unsigned char *)(dPtr);\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src;\
}

#if ((BYTE_ORDER == LITTLE_ENDIAN) || (BYTE_ORDER == BIG_ENDIAN))

/* The little endian is just reversed from the big endian, so we can
 * do the exchange when we do the copy and save calling hton*
 */

#define shortToRevBytes(s,shortBytes) \
{\
   register const unsigned char *src= ((unsigned char *) &s);\
   register unsigned char *dest= (unsigned char *)(shortBytes);\
   register unsigned char s0;\
   s0 = *src++;\
   *dest++ = *src;\
   *dest = s0;\
}

#define intToRevBytes(i,intBytes) \
{\
   register unsigned char *src= ((unsigned char *) &(i));\
   register unsigned char *dest = (intBytes);\
   register unsigned char s0,s1;\
/*   if ((unsigned char *)(intBytes) == (unsigned char *)&(i))*/\
/*   printf("Same ");*/\
/*   printf("before %08x ",(int)(i));*/\
/*   printf("%02x",(int)dest[0]);*/\
/*   printf("%02x",(int)dest[1]);*/\
/*   printf("%02x",(int)dest[2]);*/\
/*   printf("%02x",(int)dest[3]);*/\
   s0 = src[0];\
   s1 = src[1];\
   dest[0] = src[3];\
   dest[1] = src[2];\
   dest[2] = s1;\
   dest[3] = s0;\
/*   printf(" After %08x \n",(int)(i));*/\
/*   printf("%02x",(int)dest[0]);*/\
/*   printf("%02x",(int)dest[1]);*/\
/*   printf("%02x",(int)dest[2]);*/\
/*   printf("%02x",(int)dest[3]);*/\
/*   printf(" intToRevBytes\n");*/\
}

#define longToRevBytes(l,longBytes) intToRevBytes(l,longBytes)

#define floatToRevBytes(f,floatBytes) intToRevBytes(f,floatBytes)

#define doubleToRevBytes(d,doubleBytes) \
{\
   register int *Integer=((int *)&(d));\
   intToRevBytes(*Integer,(doubleBytes));\
   Integer++;\
   intToRevBytes(*Integer,((doubleBytes)+sizeof(int)));\
}


#define revBytesToShort(shortBytes, sPtr) \
{\
   register const unsigned char *src= (unsigned char *) (shortBytes);\
   register unsigned char *dest= (unsigned char *)(sPtr);\
   register unsigned char s0;\
   s0 = src[0];\
   dest[0] = src[1];\
   dest[1] = s0;\
}

#define revBytesToInt(intBytes, iPtr) \
{\
   register unsigned char *src= (unsigned char *)(intBytes);\
   register unsigned char *dest= ((unsigned char *) (iPtr));\
   register unsigned char s0,s1;\
/*   if ((unsigned char *)(intBytes) == (unsigned char *)(iPtr))*/\
/*   printf("Same ");*/\
/*   printf("before ");*/\
/*   printf("%02x",(int)src[0]);*/\
/*   printf("%02x",(int)src[1]);*/\
/*   printf("%02x",(int)src[2]);*/\
/*   printf("%02x",(int)src[3]);*/\
/*   printf(" %08x ", *iPtr);*/\
   s0 = src[0];\
   s1 = src[1];\
   dest[0] = src[3];\
   dest[1] = src[2];\
   dest[2] = s1;\
   dest[3] = s0;\
/*   printf(" after ");*/\
/*   printf("%02x",(int)src[0]);*/\
/*   printf("%02x",(int)src[1]);*/\
/*   printf("%02x",(int)src[2]);*/\
/*   printf("%02x",(int)src[3]);*/\
/*   printf(" %08x ",*iPtr);*/\
/*   printf(" revBytesToInt\n");*/\
}

#define revBytesToLong(longBytes, lPtr) revBytesToInt(longBytes,(int *)(lPtr)) 

#define revBytesToFloat(floatBytes, fPtr) \
revBytesToInt((floatBytes), (int *)(fPtr)) 
     
#define revBytesToDouble(doubleBytes,dPtr) \
{\
   register int *intPtr = (int *) (dPtr);\
   register unsigned char *restBytes = (unsigned char*) (doubleBytes) + sizeof(int);\
   revBytesToInt((doubleBytes),(intPtr));\
   intPtr++;\
   revBytesToInt((restBytes),(intPtr));\
}

#else /* not little endian or big endian, must be pdp endian.*/

#define shortToRevBytes(s,shortBytes) \
{\
   register const unsigned char *src= (unsigned char *) &(s);\
   register unsigned char *dest= (shortBytes);\
   register short temp = (s);\
   s = htons(s);\
   *dest++ = *src++;\
   *dest = *src; \
   s = temp;\
}

#define intToRevBytes(i,intBytes) \
{\
   register const unsigned char *src= (unsigned char *) &(i);\
   register unsigned char *dest = (intBytes);\
   register int temp = (i);\
   i = htonl(i);\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src; \
   i = temp;\
}

/*#if (sizeof(long) == 4)*/
#if TRUE

#define longToRevBytes(l,longBytes) \
{\
   register const unsigned char *src= (unsigned char *) &l;\
   register unsigned char *dest= (longBytes);\
   register long temp = (l);\
   l = htonl(l);\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src; \
   l = temp;\
}

/*#elif (sizeof(long) == 8)*/
#else

#define longToRevBytes(l,longBytes) \
{\
   int *Integer= ((int *)&l);\
   register const unsigned char *src= (unsigned char *) Integer;\
   register unsigned char *dest= (longBytes);\
   register long temp = l;\
   *Integer = htonl(*Integer);\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src;\
   l = temp;\
}

#endif

#define floatToRevBytes(f,floatBytes) \
{\
   int *Integer= ((int *)&f);\
   register const unsigned char *src= (unsigned char *) Integer;\
   register unsigned char *dest= (floatBytes);\
   register float temp = (f);\
   *Integer= htonl(*Integer);\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src; \
   f = temp;\
}

#define doubleToRevBytes(d,doubleBytes) \
{\
   int *Integer=((int *)&d);\
   register const unsigned char *src= (unsigned char *) Integer;\
   register unsigned char *dest = (doubleBytes);\
   register double temp = (d);\
   *Integer = htonl(*Integer);\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src;\
   *Integer = htonl(*Integer);\
   src = (const unsigned char *) Integer;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src; \
   d = temp;\
}


#define revBytesToShort(shortBytes, sPtr) \
{\
   register const unsigned char *src= (shortBytes);\
   register unsigned char *dest= (unsigned char *) sPtr;\
   *dest++ = *src++;\
   *dest = *src;\
   *sPtr = htons(*sPtr); }

#define revBytesToInt(intBytes, iPtr) \
{\
   register const unsigned char *src= (intBytes);\
   register unsigned char *dest= (unsigned char *) (iPtr);\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src;\
   *iPtr = htonl(*iPtr);}

/*#if (sizeof(long) == 4)*/
#if TRUE

#define revBytesToLong(longBytes, lPtr) \
{\
   register const unsigned char *src= (longBytes);\
   register unsigned char *dest= (unsigned char *) (lPtr);\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src;\
   *lPtr = htonl(*lPtr); }

/*#elif (sizeof(long) == 8)*/
#else

#define revBytesToLong(longBytes, lPtr) \
{\
   register const unsigned char *src= (longBytes);\
   register unsigned char *dest= (unsigned char *) (lPtr);\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src;\
   *lPtr = htonl(*lPtr); }
#endif

#define revBytesToFloat(floatBytes, fPtr) \
{\
   register const unsigned char *src= (floatBytes);\
   register unsigned char *dest= (unsigned char *) (fPtr);\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src;\
   *((int *)fPtr) = htonl(*((int *)fPtr));\
}

#define revBytesToDouble(doubleBytes,dPtr) \
{\
   register const unsigned char *src= (doubleBytes);\
   register unsigned char *dest= (unsigned char *) (dPtr);\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest++ = *src++;\
   *dest = *src;\
   *((int *)dPtr) = htonl(*((int *)dPtr));\
   *(((int *)dPtr)+1) = htonl(*(((int *)dPtr)+1));\
}

#endif /* LITTLE_ENDIAN || BIG_ENDIAN */

/* Define the encoding/decoding functions. */

/* Characters never need to be reordered. */
#define charToNetBytes(c,charBytes) {*((unsigned char *)(charBytes)) = (unsigned char)(c);}

#define netBytesToChar(charBytes, cPtr) \
{*((unsigned char *)(cPtr)) = *((unsigned char *)(charBytes));}

#define shortToNetBytes(s,shortBytes) shortToBytes(s,shortBytes) 

#define netBytesToShort(shortBytes, sPtr) \
{if (byteOrder == BYTE_ORDER) \
   {bytesToShort(shortBytes,sPtr)}\
else \
   {revBytesToShort(shortBytes,sPtr)}\
}

#define intToNetBytes(i,intBytes) intToBytes(i,intBytes) 

#define netBytesToInt(intBytes, iPtr) \
{if (byteOrder == BYTE_ORDER) \
   {bytesToInt(intBytes,iPtr)}\
else \
   {revBytesToInt(intBytes,iPtr)}\
}

#define longToNetBytes(l,longBytes) longToBytes(l,longBytes) 

#define netBytesToLong(longBytes, lPtr) \
{if (byteOrder == BYTE_ORDER) \
   {bytesToLong(longBytes,lPtr)}\
else \
   {revBytesToLong(longBytes,lPtr)}\
}

#define floatToNetBytes(f,floatBytes) floatToBytes(f,floatBytes) 

#define netBytesToFloat(floatBytes, fPtr) \
{if (byteOrder == BYTE_ORDER) \
   {bytesToFloat(floatBytes,fPtr)}\
else \
   {revBytesToFloat(floatBytes,fPtr)}\
}

#define doubleToNetBytes(d,doubleBytes) doubleToBytes(d,doubleBytes) 

#define netBytesToDouble(doubleBytes, dPtr) \
{if (byteOrder == BYTE_ORDER) \
   {bytesToDouble(doubleBytes,dPtr)}\
else \
   {revBytesToDouble(doubleBytes,dPtr)}\
}

#if (BYTE_ORDER == NETWORK_BYTE_ORDER)
/* No ops */
#define INT_TO_NET_INT(i) (i)
#define NET_INT_TO_INT(i) (i)
#elif ((BYTE_ORDER == LITTLE_ENDIAN) || (BYTE_ORDER == BIG_ENDIAN))
/* Eventually, this should be replaced.*/
#define INT_TO_NET_INT(i) htonl(i)
#define NET_INT_TO_INT(i) ntohl(i)
#else
#define INT_TO_NET_INT(i) htonl(i)
#define NET_INT_TO_INT(i) ntohl(i)
#endif

#endif
