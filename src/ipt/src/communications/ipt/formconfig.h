/*
 *
 *                                 formconfig.h
 *
 * The format code depends on how data structures are layed out in memory.
 * This include file declares set of constants that control how the formatting 
 * will be done.  The two important variables are BYTE_ORDER (BIG_ENDIAN or
 * LITTLE_ENDIAN) and ALIGN (usually ALIGN_WORD, i.e. on 68k or ALIGN_LONGEST
 * on sun4's).  The file defines these two variables based on the machine type
 *
 *  "1995, Carnegie Mellon University. All Rights Reserved." This
 *  software is made available for academic and research purposes only. No
 *  commercial license is hereby granted.  Copying and other reproduction is
 *  authorized only for research, education, and other non-commercial
 *  purposes.  No warranties, either expressed or implied, are made
 *  regarding the operation, use, or results of the software.
 *
 */

#ifndef ip_formconfig_h
#define ip_formconfig_h

typedef enum 
{
  /* 
   * Align things on word boundaries unless next item uses an odd number 
   * of bytes.
   */
  PACKED_ALIGNMENT = 0x00,
  
  /* 
   * Align things on word boundaries unless next item uses an odd number 
   * of bytes.
   */
  WORD_ALIGNMENT = 0x01,
  
  /* 
   * Align things on the boundary of the longest type in the struct.
   */
  LONGEST_ALIGNMENT = 0x02,
  
  /* 
   * Align things on the boundary of the longest type in the struct, up to the
   * size of an int.
   */
  INT_ALIGNMENT = 0x04
} ALIGNMENT_TYPE;

#define ALIGN_UNDEFINED -1
#define ALIGN_PACKED  0x00
#define ALIGN_WORD    0x01
#define ALIGN_LONGEST 0x02
#define ALIGN_INT     0x04

#ifdef LINUX
#undef LITTLE_ENDIAN
#undef BIG_ENDIAN
#undef PDP_ENDIAN
#undef BYTE_ORDER
#endif

#define	LITTLE_ENDIAN	0	/* least-significant byte first (vax) */
#define	BIG_ENDIAN	1	/* most-significant byte first (IBM, net) */
#define	PDP_ENDIAN	2	/* LSB first in word, MSW first in long (pdp) */
#define BYTE_ORDER_UNDEFINED -1

#if defined(M68K) || defined(sun3) || defined(SUN3) || defined (M68k) 
#define ALIGN ALIGN_WORD
#define BYTE_ORDER BIG_ENDIAN 
#elif defined(THINK_C) || defined(applec)
#define ALIGN ALIGN_WORD
#define BYTE_ORDER BIG_ENDIAN 
#elif defined(R4000) || defined(r4000)
#define ALIGN ALIGN_LONGEST
#define BYTE_ORDER BIG_ENDIAN 
#elif defined(sun4) || defined(SUN4) || defined(sparc) || defined(__sparc)
#define ALIGN ALIGN_LONGEST
#define BYTE_ORDER BIG_ENDIAN 
#elif defined(pmax) || defined(alpha)
#define ALIGN ALIGN_LONGEST
#define BYTE_ORDER BIG_ENDIAN 
#elif defined(i386) || defined(LINUX)
#define ALIGN ALIGN_INT
#define BYTE_ORDER LITTLE_ENDIAN 
#elif defined(SGI)
#define ALIGN ALIGN_LONGEST
#define BYTE_ORDER BIG_ENDIAN 
#elif defined(PPC)
#define ALIGN ALIGN_INT
#define BYTE_ORDER BIG_ENDIAN 
#else 
#undef ALIGN
#undef BYTE_ORDER
#endif

#define NETWORK_BYTE_ORDER BIG_ENDIAN
/* Use this for testing.*/
/*#define NETWORK_BYTE_ORDER LITTLE_ENDIAN*/
#define NETWORK_ALIGNMENT ALIGN_WORD

#endif
