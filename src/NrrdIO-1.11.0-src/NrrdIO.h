/*
  NrrdIO: stand-alone code for basic nrrd functionality
  Copyright (C) 2012, 2011, 2010, 2009  University of Chicago
  Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must
     not claim that you wrote the original software. If you use this
     software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.

  2. Altered source versions must be plainly marked as such, and must
     not be misrepresented as being the original software.

  3. This notice may not be removed or altered from any source distribution.
*/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <float.h>

/*
******** TEEM_VERSION
**
** TEEM_VERSION is a single (decimal) number that will always increase
** monotically, and the _MAJOR, _MINOR, _PATCH are also numbers that
** can be used to implement pre-processor logic about specifc
** versions.  The TEEM_VERSION_STRING is used in the (existing) char
** *airTeemVersion (added in version 1.9.0).  Yes, keeping these in
** sync is currently a manual operation.
**
** NOTE: Significant API changes (aside from API additions) should NOT
** occur with changes in patch level, only with major or minor version
** changes.
**
** NOTE: ../../CMakeLists.txt's Teem_VERSION variables must be in sync
*/
#define TEEM_VERSION_MAJOR       1   /* must be 1 digit */
#define TEEM_VERSION_MINOR      11   /* 1 or 2 digits */
#define TEEM_VERSION_PATCH      00   /* 1 or 2 digits */
#define TEEM_VERSION         11100   /* must be 5 digits, to facilitate
                                        easy numerical comparison */
#define TEEM_VERSION_STRING "1.11.0" /* cannot be so easily compared */



#ifdef __cplusplus
extern "C" {
#endif


#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(TEEM_STATIC)
#  if defined(TEEM_BUILD) || defined(air_EXPORTS) || defined(teem_EXPORTS)
#    define NRRDIO_EXPORT extern __declspec(dllexport)
#  else
#    define NRRDIO_EXPORT extern __declspec(dllimport)
#  endif
#else /* TEEM_STATIC || UNIX */
#  define NRRDIO_EXPORT extern
#endif

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
typedef signed __int64 airLLong;
typedef unsigned __int64 airULLong;
#  define AIR_LLONG_FMT "%I64d"
#  define AIR_ULLONG_FMT "%I64u"
#  define AIR_LLONG(x) x##i64
#  define AIR_ULLONG(x) x##ui64
#else
typedef signed long long airLLong;
typedef unsigned long long airULLong;
#  define AIR_LLONG_FMT "%lld"
#  define AIR_ULLONG_FMT "%llu"
#  define AIR_LLONG(x) x##ll
#  define AIR_ULLONG(x) x##ull
#endif


/*
** These serve as conservative estimates on how large various strings
** might end up being.  It would be theoretically better to completely
** avoid the use of fixed-size buffers, but in many contexts the
** implementation complexity of handling them reliably is distracts
** from more urgent implementation goals.  In the mean time, these can
** be used safely as long as the lengths are used consistently.
**
** The possibly unfortunate convention that has become established in
** Teem is code using these tends to NOT add the "+1", to explicitly
** indicate the space for 0-termination, and instead assumes it is
** part of the numbers below, even though this is at the cost of
** confusion about how the maximal strlen() will be less than each of
** these numbers. This will be addressed in Teem 2.0.
*/
#define AIR_STRLEN_SMALL (128+1) /* has to be big enough to hold:
                                  - printed value of size_t and ptrdiff_t,
                                  - line of text that should contain file
                                    format "magic"
                                  */
#define AIR_STRLEN_MED   (256+1)
#define AIR_STRLEN_LARGE (512+1)
#define AIR_STRLEN_HUGE (1024+1) /* has to be big enough to hold
                                    a biff error message (one line of it) */

/*
******** airPtrPtrUnion
**
** union of addresses of pointers to various types, to deal with strict
** aliasing warnings, especially with the first argument to airArrayNew().
** Unfortunately this can't meet the needs of all such cases because some
** libraries need to manage addresses of arrays of other kinds of
** library-specific objects (about which air is ignorant).
*/
typedef union {
  unsigned char **uc;
  signed char **sc;
  char **c;
  char ***cp;
  unsigned short **us;
  short **s;
  unsigned int **ui;
  int **i;
  float **f;
  double **d;
  void **v;
} airPtrPtrUnion;

/*
******** airEnum struct
**
** The airEnum provides the basic mechanism of mapping from a
** string to an int enum value, and back.
*/
typedef struct {
  const char *name;
               /* what are these things? */
  unsigned int M;
               /* str[0]: string for the unknown/invalid value;
                * str[1] .. str[M]: canonical strings for the enum values;
                * "val" NULL: unknown/invalid = 0;
                *             valid values are 1 .. M
                * "val" non-NULL: unknown/invalid = val[0];
                *                 valid are val[1].. val[M]
                */
  const char **str;
               /* see above */
  const int *val;
               /* see above */
  const char **desc;
               /* desc[i] is a short description of the enum values represented
                  by str[i] (thereby starting with the unknown value), to be
                  used to by things like hest */
  const char **strEqv;
               /* If non-NULL, all the variations in strings recognized in
                  mapping from string to value (the values in valEqv).
                  This **MUST** be terminated by a zero-length string ("") so
                  as to signify the end of the list.  This should *not*
                  contain the string for unknown/invalid.
                  If "strEqv" is NULL, then mapping from string to value is
                  done only by traversing "str", and "valEqv" is ignored. */
  const int *valEqv;
               /* If strEqv non-NULL, valEqv holds the values corresponding
                  to the strings in strEqv, with one integer for each
                  non-zero-length string in strEqv: strEqv[i] is a valid
                  string representation for value valEqv[i]. This should *not*
                  contain the value for unknown/invalid.
                  This "valEqv" is ignored if "strEqv" is NULL. */
  int sense;   /* require case matching on strings */
} airEnum;
NRRDIO_EXPORT int airEnumUnknown(const airEnum *enm);
NRRDIO_EXPORT int airEnumValCheck(const airEnum *enm, int val);
NRRDIO_EXPORT const char *airEnumStr(const airEnum *enm, int val);
NRRDIO_EXPORT const char *airEnumDesc(const airEnum *enm, int val);
NRRDIO_EXPORT int airEnumVal(const airEnum *enm, const char *str);
NRRDIO_EXPORT char *airEnumFmtDesc(const airEnum *enm, int val, int canon,
                                const char *fmt);
NRRDIO_EXPORT void airEnumPrint(FILE *file, const airEnum *enm);

/*
******** airEndian enum
**
** for identifying how a file was written to disk, for those encodings
** where the raw data on disk is dependent on the endianness of the
** architecture.
*/
enum {
  airEndianUnknown,         /* 0: nobody knows */
  airEndianLittle = 1234,   /* 1234: Intel and friends */
  airEndianBig = 4321,      /* 4321: the rest */
  airEndianLast
};
/* endianAir.c */
NRRDIO_EXPORT const airEnum *const airEndian;
NRRDIO_EXPORT int airMyEndian(void);

/* array.c: poor-man's dynamically resizable arrays */
typedef struct {
  void *data,         /* where the data is */
    **dataP;          /* (possibly NULL) address of user's data variable,
                         kept in sync with internal "data" variable */
  unsigned int len,   /* length of array: # units for which there is
                         considered to be data (which is <= total # units
                         allocated).  The # bytes which contain data is
                         len*unit.  Always updated (unlike "*lenP") */
    *lenP,            /* (possibly NULL) address of user's length variable,
                         kept in sync with internal "len" variable */
    incr,             /* the granularity of the changes in amount of space
                         allocated: when the length reaches a multiple of
                         "incr", then the array is resized */
    size;             /* array is allocated to have "size" increments, or,
                         size*incr elements, or,
                         size*incr*unit bytes */
  size_t unit;        /* the size in bytes of one element in the array */
  int noReallocWhenSmaller;  /* as it says */

  /* the following are all callbacks useful for maintaining either an array
     of pointers (allocCB and freeCB) or array of structs (initCB and
     doneCB).  allocCB or initCB is called when the array length increases,
     and freeCB or doneCB when it decreases.  Any of them can be NULL if no
     such activity is desired.  allocCB sets values in the array (as in
     storing the return from malloc(); freeCB is called on values in the
     array (as in calling free()), and the values are cast to void*.  allocCB
     and freeCB don't care about the value of "unit" (though perhaps they
     should).  initCB and doneCB are called on the _addresses_ of elements in
     the array.  allocCB and initCB are called for the elements in ascending
     order in the array, and freeCB and doneCB are called in descending
     order.  allocCB and initCB are mutually exclusive- they can't both be
     non-NULL. Same goes for freeCB and doneCB */
  void *(*allocCB)(void);  /* values of new elements set to return of this */
  void *(*freeCB)(void *); /* called on the values of invalidated elements */
  void (*initCB)(void *);  /* called on addresses of new elements */
  void (*doneCB)(void *);  /* called on addresses of invalidated elements */

} airArray;
NRRDIO_EXPORT airArray *airArrayNew(void **dataP, unsigned int *lenP, size_t unit,
                                 unsigned int incr);
NRRDIO_EXPORT void airArrayStructCB(airArray *a, void (*initCB)(void *),
                                 void (*doneCB)(void *));
NRRDIO_EXPORT void airArrayPointerCB(airArray *a, void *(*allocCB)(void),
                                  void *(*freeCB)(void *));
NRRDIO_EXPORT void airArrayLenSet(airArray *a, unsigned int newlen);
NRRDIO_EXPORT unsigned int airArrayLenIncr(airArray *a, int delta);
NRRDIO_EXPORT airArray *airArrayNix(airArray *a);
NRRDIO_EXPORT airArray *airArrayNuke(airArray *a);


/*
******** airFP enum
**
** the different kinds of floating point number afforded by IEEE 754,
** and the values returned by airFPClass_f().
**
** The values probably won't agree with those in #include's like
** ieee.h, ieeefp.h, fp_class.h.  This is because IEEE 754 hasn't
** defined standard values for these, so everyone does it differently.
**
** This enum uses underscores (against Teem convention) to help
** legibility while also conforming to the spirit of the somewhat
** standard naming conventions
*/
enum {
  airFP_Unknown,               /*  0: nobody knows */
  airFP_SNAN,                  /*  1: signalling NaN */
  airFP_QNAN,                  /*  2: quiet NaN */
  airFP_POS_INF,               /*  3: positive infinity */
  airFP_NEG_INF,               /*  4: negative infinity */
  airFP_POS_NORM,              /*  5: positive normalized non-zero */
  airFP_NEG_NORM,              /*  6: negative normalized non-zero */
  airFP_POS_DENORM,            /*  7: positive denormalized non-zero */
  airFP_NEG_DENORM,            /*  8: negative denormalized non-zero */
  airFP_POS_ZERO,              /*  9: +0.0, positive zero */
  airFP_NEG_ZERO,              /* 10: -0.0, negative zero */
  airFP_Last                   /* after the last valid one */
};
/* 754.c: IEEE-754 related stuff values */
typedef union {
  unsigned int i;
  float f;
} airFloat;
typedef union {
  airULLong i;
  double d;
} airDouble;
NRRDIO_EXPORT const int airMyQNaNHiBit;
NRRDIO_EXPORT float airFPPartsToVal_f(unsigned int sign,
                                   unsigned int expo,
                                   unsigned int mant);
NRRDIO_EXPORT void airFPValToParts_f(unsigned int *signP,
                                  unsigned int *expoP,
                                  unsigned int *mantP, float v);
NRRDIO_EXPORT double airFPPartsToVal_d(unsigned int sign,
                                    unsigned int expo,
                                    unsigned int mant0,
                                    unsigned int mant1);
NRRDIO_EXPORT void airFPValToParts_d(unsigned int *signP,
                                  unsigned int *expoP,
                                  unsigned int *mant0P,
                                  unsigned int *mant1P,
                                  double v);
NRRDIO_EXPORT float airFPGen_f(int cls);
NRRDIO_EXPORT double airFPGen_d(int cls);
NRRDIO_EXPORT int airFPClass_f(float val);
NRRDIO_EXPORT int airFPClass_d(double val);
NRRDIO_EXPORT void airFPFprintf_f(FILE *file, float val);
NRRDIO_EXPORT void airFPFprintf_d(FILE *file, double val);
NRRDIO_EXPORT const airFloat airFloatQNaN;
NRRDIO_EXPORT const airFloat airFloatSNaN;
NRRDIO_EXPORT const airFloat airFloatPosInf;
NRRDIO_EXPORT const airFloat airFloatNegInf;
NRRDIO_EXPORT float airNaN(void);
NRRDIO_EXPORT int airIsNaN(double d);
NRRDIO_EXPORT int airIsInf_f(float f);
NRRDIO_EXPORT int airIsInf_d(double d);
NRRDIO_EXPORT int airExists(double d);


/*
******** airType
**
** Different types which air cares about.
** Currently only used in the command-line parsing, but perhaps will
** be used elsewhere in air later
*/
enum {
  airTypeUnknown,   /*  0 */
  airTypeBool,      /*  1 */
  airTypeInt,       /*  2 */
  airTypeUInt,      /*  3 */
  airTypeLongInt,   /*  4 */
  airTypeULongInt,  /*  5 */
  airTypeSize_t,    /*  6 */
  airTypeFloat,     /*  7 */
  airTypeDouble,    /*  8 */
  airTypeChar,      /*  9 */
  airTypeString,    /* 10 */
  airTypeEnum,      /* 11 */
  airTypeOther,     /* 12 */
  airTypeLast
};
#define AIR_TYPE_MAX   12
/* parseAir.c */
NRRDIO_EXPORT double airAtod(const char *str);
NRRDIO_EXPORT int airSingleSscanf(const char *str, const char *fmt, void *ptr);
NRRDIO_EXPORT const airEnum *const airBool;
NRRDIO_EXPORT unsigned int airParseStrB(int *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* (nothing used) */);
NRRDIO_EXPORT unsigned int airParseStrI(int *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* (nothing used) */);
NRRDIO_EXPORT unsigned int airParseStrUI(unsigned int *out, const char *s,
                                      const char *ct, unsigned int n,
                                      ... /* (nothing used) */);
NRRDIO_EXPORT unsigned int airParseStrZ(size_t *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* (nothing used) */);
NRRDIO_EXPORT unsigned int airParseStrF(float *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* (nothing used) */);
NRRDIO_EXPORT unsigned int airParseStrD(double *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* (nothing used) */);
NRRDIO_EXPORT unsigned int airParseStrC(char *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* (nothing used) */);
NRRDIO_EXPORT unsigned int airParseStrS(char **out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* REQ'D even if n>1: int greedy */);
NRRDIO_EXPORT unsigned int airParseStrE(int *out, const char *s,
                                     const char *ct, unsigned int n,
                                     ... /* REQUIRED: airEnum *e */);
NRRDIO_EXPORT unsigned int (*airParseStr[AIR_TYPE_MAX+1])(void *, const char *,
                                                       const char *,
                                                       unsigned int, ...);

/* string.c */
NRRDIO_EXPORT char *airStrdup(const char *s);
NRRDIO_EXPORT size_t airStrlen(const char *s);
NRRDIO_EXPORT int airStrtokQuoting;
NRRDIO_EXPORT char *airStrtok(char *s, const char *ct, char **last);
NRRDIO_EXPORT unsigned int airStrntok(const char *s, const char *ct);
NRRDIO_EXPORT char *airStrtrans(char *s, char from, char to);
NRRDIO_EXPORT char *airStrcpy(char *dst, size_t dstSize, const char *src);
NRRDIO_EXPORT int airEndsWith(const char *s, const char *suff);
NRRDIO_EXPORT char *airUnescape(char *s);
NRRDIO_EXPORT char *airOneLinify(char *s);
NRRDIO_EXPORT char *airToLower(char *str);
NRRDIO_EXPORT char *airToUpper(char *str);
NRRDIO_EXPORT unsigned int airOneLine(FILE *file, char *line, unsigned int size);

/* sane.c */
/*
******** airInsane enum
**
** reasons for why airSanity() failed (specifically, the possible
** return values for airSanity()
*/
enum {
  airInsane_not,           /*  0: actually, all sanity checks passed */
  airInsane_endian,        /*  1: airMyEndian is wrong */
  airInsane_pInfExists,    /*  2: AIR_EXISTS(positive infinity) was true */
  airInsane_nInfExists,    /*  3: AIR_EXISTS(negative infinity) was true */
  airInsane_NaNExists,     /*  4: AIR_EXISTS(NaN) was true */
  airInsane_FltDblFPClass, /*  5: double -> float assignment messed up the
                               airFPClass_f() of the value */
  airInsane_QNaNHiBit,     /*  6: airMyQNaNHiBit is wrong */
  airInsane_AIR_NAN,       /*  7: airFPClass_f(AIR_QNAN) wrong
                                  (no longer checking on problematic SNAN) */
  airInsane_dio,           /*  8: airMyDio set to something invalid */
  airInsane_UCSize,        /*  9: unsigned char isn't 8 bits */
  airInsane_FISize,        /* 10: sizeof(float), sizeof(int) not 4 */
  airInsane_DLSize         /* 11: sizeof(double), sizeof(airLLong) not 8 */
};
#define AIR_INSANE_MAX        11
NRRDIO_EXPORT const char *airInsaneErr(int insane);
NRRDIO_EXPORT int airSanity(void);

/* miscAir.c */
NRRDIO_EXPORT const char *airTeemVersion;
NRRDIO_EXPORT const char *airTeemReleaseDate;
NRRDIO_EXPORT void *airNull(void);
NRRDIO_EXPORT void *airSetNull(void **ptrP);
NRRDIO_EXPORT void *airFree(void *ptr);
NRRDIO_EXPORT FILE *airFopen(const char *name, FILE *std, const char *mode);
NRRDIO_EXPORT FILE *airFclose(FILE *file);
NRRDIO_EXPORT int airSinglePrintf(FILE *file, char *str, const char *fmt, ...);
NRRDIO_EXPORT char *airSprintSize_t(char str[AIR_STRLEN_SMALL], size_t val);

/* dio.c */
/*
******** airNoDio enum
**
** reasons for why direct I/O won't be used with a particular
** file/pointer combination
*/
enum {
  airNoDio_okay,    /*  0: actually, you CAN do direct I/O */
  airNoDio_arch,    /*  1: Teem thinks this architecture can't do it */
  airNoDio_format,  /*  2: Teem thinks given data file format can't use it */
  airNoDio_std,     /*  3: DIO isn't possible for std{in|out|err} */
  airNoDio_fd,      /*  4: couldn't get underlying file descriptor */
  airNoDio_dioinfo, /*  5: calling fcntl() to get direct I/O info failed */
  airNoDio_small,   /*  6: requested size is too small */
  airNoDio_size,    /*  7: requested size not a multiple of d_miniosz */
  airNoDio_ptr,     /*  8: pointer not multiple of d_mem */
  airNoDio_fpos,    /*  9: current file position not multiple of d_miniosz */
  airNoDio_setfl,   /* 10: fcntl(fd, SETFL, FDIRECT) failed */
  airNoDio_test,    /* 11: couldn't memalign() even a small bit of memory */
  airNoDio_disable  /* 12: someone disabled it with airDisableDio */
};
#define AIR_NODIO_MAX  12
NRRDIO_EXPORT const char *airNoDioErr(int noDio);
NRRDIO_EXPORT const int airMyDio;
NRRDIO_EXPORT int airDisableDio;
NRRDIO_EXPORT void airDioInfo(int *align, int *min, int *max, int fd);
NRRDIO_EXPORT int airDioTest(int fd, const void *ptr, size_t size);
NRRDIO_EXPORT void *airDioMalloc(size_t size, int fd);
NRRDIO_EXPORT size_t airDioRead(int fd, void *ptr, size_t size);
NRRDIO_EXPORT size_t airDioWrite(int fd, const void *ptr, size_t size);

/* mop.c: clean-up utilities */
enum {
  airMopNever,
  airMopOnError,
  airMopOnOkay,
  airMopAlways
};
typedef void *(*airMopper)(void *);
typedef struct {
  void *ptr;         /* the thing to be processed */
  airMopper mop;     /* the function to which does the processing */
  int when;          /* from the airMopWhen enum */
} airMop;
NRRDIO_EXPORT airArray *airMopNew(void);
NRRDIO_EXPORT int airMopAdd(airArray *arr, void *ptr, airMopper mop, int when);
NRRDIO_EXPORT void airMopSub(airArray *arr, void *ptr, airMopper mop);
NRRDIO_EXPORT void airMopMem(airArray *arr, void *_ptrP, int when);
NRRDIO_EXPORT void airMopUnMem(airArray *arr, void *_ptrP);
NRRDIO_EXPORT void airMopPrint(airArray *arr, const void *_str, int when);
NRRDIO_EXPORT void airMopDone(airArray *arr, int error);
NRRDIO_EXPORT void airMopError(airArray *arr);
NRRDIO_EXPORT void airMopOkay(airArray *arr);
NRRDIO_EXPORT void airMopDebug(airArray *arr);


/*******     the interminable sea of defines and macros     *******/

#define AIR_TRUE 1
#define AIR_FALSE 0
#define AIR_WHITESPACE " \t\n\r\v\f"       /* K+R pg. 157 */

/*
******** AIR_UNUSED
**
** one way of reconciling "warning: unused parameter" with
** C's "error: parameter name omitted"
*/
#define AIR_UNUSED(x) (void)(x)

/*
******** AIR_CAST, AIR_UINT, AIR_INT
**
** just casts, but with the added ability to grep for them more easily,
** since casts should probably always be revisited and reconsidered.
*/
#define AIR_CAST(t, v) ((t)(v))
#define AIR_UINT(x) AIR_CAST(unsigned int, x)
#define AIR_INT(x) AIR_CAST(int, x)

/*
******** AIR_VOIDP, AIR_CVOIDP
**
** explicit casting to "void *" (and "const void *") from non-void* pointers
** is strictly speaking needed for the %p format specifier in printf-like
** functions; this is a slightly more convenient form
*/
#define AIR_VOIDP(x) AIR_CAST(void *, x)
#define AIR_CVOIDP(x) AIR_CAST(const void *, x)

/*
******** AIR_MALLOC, AIR_CALLOC
**
** slightly simpler wrapper around cast and malloc/calloc
**
** HEY note that "T" is not guarded by parentheses in its first usage,
** as arguments in Teem macros normally are
*/
#define AIR_MALLOC(N, T) (T*)(malloc((N)*sizeof(T)))
#define AIR_CALLOC(N, T) (T*)(calloc((N), sizeof(T)))

/*
******** AIR_ENDIAN, AIR_QNANHIBIT, AIR_DIO
**
** These reflect particulars of hardware which we're running on. The
** difference from the things starting with TEEM_ is that the TEEM_
** values are for passing architecture-specific to compilation of source
** files, and thes AIR_ variables are for advertising that information
** to anyone linking against air (or Teem) and including air.h.
*/
#define AIR_ENDIAN (airMyEndian())
#define AIR_QNANHIBIT (airMyQNaNHiBit)
#define AIR_DIO (airMyDio)

/*
******** AIR_NAN, AIR_QNAN, AIR_SNAN, AIR_POS_INF, AIR_NEG_INF
**
** its nice to have these values available without the cost of a
** function call.
**
** NOTE: AIR_POS_INF and AIR_NEG_INF correspond to the _unique_
** bit-patterns which signify positive and negative infinity.  With
** the NaNs, however, they are only one of many possible
** representations.
*/
#define AIR_NAN  (airFloatQNaN.f)
#define AIR_QNAN (airFloatQNaN.f)
#define AIR_SNAN (airFloatSNaN.f)
#define AIR_POS_INF (airFloatPosInf.f)
#define AIR_NEG_INF (airFloatNegInf.f)

/*
******** AIR_EXISTS
**
** is non-zero (true) only for values which are not NaN or +/-infinity
**
** You'd think that (x == x) might work, but no no no, some optimizing
** compilers (e.g. SGI's cc) say "well of course they're equal, for all
** possible values".  Bastards!
**
** One of the benefits of IEEE 754 floating point numbers is that
** gradual underflow means that x = y <==> x - y = 0 for any (positive
** or negative) normalized or denormalized float.  Otherwise this
** macro could not be valid; some floating point conventions say that
** a zero-valued exponent means zero, regardless of the mantissa.
**
** However, there MAY be problems on machines which use extended
** (80-bit) floating point registers, such as Intel chips- where the
** same initial value 1) directly read from the register, versus 2)
** saved to memory and loaded back, may end up being different.  I
** have yet to produce this behavior, or convince myself it can't happen.
**
** The reason to #define AIR_EXISTS as airExists is that on some
** optimizing compilers, the !((x) - (x)) doesn't work.  This has been
** the case on Windows and 64-bit irix6 (64 bit) with -Ofast.  If
** airSanity fails because a special value "exists", then use the
** first version of AIR_EXISTS.
**
** There is a performance consequence of using airExists(x), in that it
** is a function call, although (HEY) we should facilitate inline'ing it
** for compilers that know how to.
**
** gcc 4.5.3 -std=c89, at least on cygwin, has problems with
** the type of "!((x) - (x))" when used with bit-wise xor ^, saying
** "invalid operands to binary ^ (have ‘int’ and ‘int’)" but these
** problems oddly went away with the explicit cast to int.
*/
#if 1
#define AIR_EXISTS(x) (airExists(x))
#else
#define AIR_EXISTS(x) (AIR_CAST(int, !((x) - (x))))
#endif


/*
******** AIR_MAX(a,b), AIR_MIN(a,b), AIR_ABS(a)
**
** the usual
*/
#define AIR_MAX(a,b) ((a) > (b) ? (a) : (b))
#define AIR_MIN(a,b) ((a) < (b) ? (a) : (b))
#define AIR_ABS(a) ((a) > 0.0f ? (a) : -(a))

/*
******** AIR_COMPARE(a,b)
**
** the sort of compare that qsort() wants for ascending sort
*/
#define AIR_COMPARE(a,b) ((a) < (b)     \
                          ? -1          \
                          : ((a) > (b) \
                             ? 1        \
                             : 0))

/*
******** AIR_IN_OP(a,b,c), AIR_IN_CL(a,b,c)
**
** is true if the middle argument is in the open/closed interval
** defined by the first and third arguments
**
** AIR_IN_OP is new name for old AIR_BETWEEN
** AIR_IN_CL is new name for old AIR_INSIDE
*/
#define AIR_IN_OP(a,b,c) ((a) < (b) && (b) < (c))     /* closed interval */
#define AIR_IN_CL(a,b,c) ((a) <= (b) && (b) <= (c))   /* open interval */

/*
******** AIR_CLAMP(a,b,c)
**
** returns the middle argument, after being clamped to the closed
** interval defined by the first and third arguments
*/
#define AIR_CLAMP(a,b,c) ((b) < (a)        \
                           ? (a)           \
                           : ((b) > (c)    \
                              ? (c)        \
                              : (b)))

/*
******** AIR_MOD(i, N)
**
** returns that integer in [0, N-1] which is i plus a multiple of N. It
** may be unfortunate that the expression (i)%(N) appears three times;
** this should be inlined.  Or perhaps the compiler's optimizations
** (common sub-expression elimination) will save us.
**
** Note: integer divisions are not very fast on some modern chips;
** don't go silly using this one.
*/
#define AIR_MOD(i, N) ((i)%(N) >= 0 ? (i)%(N) : N + (i)%(N))

/*
******** AIR_LERP(w, a, b)
**
** returns a when w=0, and b when w=1, and linearly varies in between
*/
#define AIR_LERP(w, a, b) ((w)*((b) - (a)) + (a))

/*
******** AIR_AFFINE(i,x,I,o,O)
**
** given intervals [i,I], [o,O] and a value x which may or may not be
** inside [i,I], return the value y such that y stands in the same
** relationship to [o,O] that x does with [i,I].  Or:
**
**    y - o         x - i
**   -------   =   -------
**    O - o         I - i
**
** It is the callers responsibility to make sure I-i and O-o are
** both non-zero.  Strictly speaking, real problems arise only when
** when I-i is zero: division by zero generates either NaN or infinity
**
** NOTE that "x" is evaluated only once (which makes this more useful),
** as is "I" and "O" (usually not so important); "i" and "o" are each
** evaluated twice
*/
#define AIR_AFFINE(i,x,I,o,O) ( \
((double)(O)-(o))*((double)(x)-(i)) / ((double)(I)-(i)) + (o))

/*
******** AIR_DELTA(i,x,I,o,O)
**
** given intervals [i,I] and [o,O], calculates the number y such that
** a change of x within [i,I] is proportional to a change of y within
** [o,O].  Or:
**
**      y             x
**   -------   =   -------
**    O - o         I - i
**
** It is the callers responsibility to make sure I-i and O-o are
** both non-zero
**
** NOTE that all arguments are evaluated only once
*/
#define AIR_DELTA(i,x,I,o,O) ( \
((double)(O)-(o))*((double)(x)) / ((double)(I)-(i)) )

/*
******** AIR_ROUNDUP, AIR_ROUNDDOWN
**
** rounds integers up or down; just wrappers around floor and ceil
*/
#define AIR_ROUNDUP(x)   ((int)(floor((x)+0.5)))
#define AIR_ROUNDDOWN(x) ((int)(ceil((x)-0.5)))
#define AIR_ROUNDUP_UI(x)   ((unsigned int)(floor((x)+0.5)))
#define AIR_ROUNDDOWN_UI(x) ((unsigned int)(ceil((x)-0.5)))

#ifdef __cplusplus
}
#endif





#ifdef __cplusplus
extern "C" {
#endif

/*
** biffMsg struct
**
** externally usable thing for holding error messages
*/
typedef struct {
  char *key;                   /* string for identifying the general source
                                  of the error message; set once, at time
                                  of biffMsg creation */
  char **err;                  /* array of error strings; the err array itself
                                  is NOT null-terminated */
  unsigned int errNum;         /* length of "err" == # strings stored */
  airArray *errArr;            /* air array for err and num */
} biffMsg;

/* biffmsg.c */
NRRDIO_EXPORT biffMsg *biffMsgNew(const char *key);
NRRDIO_EXPORT biffMsg *biffMsgNix(biffMsg *msg);
NRRDIO_EXPORT void biffMsgAdd(biffMsg *msg, const char *err);
NRRDIO_EXPORT void biffMsgClear(biffMsg *msg);
NRRDIO_EXPORT unsigned int biffMsgLineLenMax(const biffMsg *msg);
NRRDIO_EXPORT void biffMsgMove(biffMsg *dest, biffMsg *src,
                             const char *err);
NRRDIO_EXPORT unsigned int biffMsgErrNum(const biffMsg *msg);
NRRDIO_EXPORT unsigned int biffMsgStrlen(const biffMsg *msg);
NRRDIO_EXPORT void biffMsgStrSet(char *ret, const biffMsg *msg);
NRRDIO_EXPORT biffMsg *biffMsgNoop;

/* biffbiff.c */
NRRDIO_EXPORT void biffAdd(const char *key, const char *err);
NRRDIO_EXPORT void biffAddf(const char *key, const char *errfmt, ...)
#ifdef __GNUC__
  __attribute__ ((format(printf,2,3)))
#endif
;
NRRDIO_EXPORT void biffMaybeAdd(const char *key, const char *err, int useBiff);
NRRDIO_EXPORT void biffMaybeAddf(int useBiff, const char *key,
                               const char *errfmt, ... )
#ifdef __GNUC__
__attribute__ ((format(printf,3,4)))
#endif
;
NRRDIO_EXPORT char *biffGet(const char *key);
NRRDIO_EXPORT unsigned int biffGetStrlen(const char *key);
NRRDIO_EXPORT void biffSetStr(char *str, const char *key);
NRRDIO_EXPORT void biffDone(const char *key);
NRRDIO_EXPORT char *biffGetDone(const char *key);

#ifdef __cplusplus
}
#endif



#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

/* feel free to set these to higher values and recompile */
#define NRRD_DIM_MAX 64            /* Max array dimension (nrrd->dim) */
#define NRRD_SPACE_DIM_MAX 60       /* Max dimension of "space" around array
                                      (nrrd->spaceDim) */

#define NRRD_EXT_NRRD   ".nrrd"
#define NRRD_EXT_NHDR   ".nhdr"
#define NRRD_EXT_PGM    ".pgm"
#define NRRD_EXT_PPM    ".ppm"
#define NRRD_EXT_PNG    ".png"
#define NRRD_EXT_VTK    ".vtk"
#define NRRD_EXT_TEXT   ".txt"
#define NRRD_EXT_EPS    ".eps"

/* HEY: should this be renamed -> MAXNUM ? Would be more consistent
   with other Teem pound-define names */
#define NRRD_KERNEL_PARMS_NUM 8    /* max # arguments to a kernel-
                                      this is weird: it isn't the max
                                      of any of the NrrdKernels
                                      defined by the nrrd library
                                      (that is more like 3), but is
                                      the max number of parms of any
                                      NrrdKernel used by anyone using
                                      Teem, such as in gage.
                                      Enforcing one global max
                                      simplifies implementation. */

/*
** For the 64-bit integer types (not standard except in C99), we used
** to try to use the names for the _MIN and _MAX values which are used
** in C99 (as well as gcc) such as LLONG_MAX, or those used on SGI
** such as LONGLONG_MAX.  However, since the tests (in nrrdSanity)
** were re-written to detect overflow based on manipulation of
** specific values, we might as well also define the _MIN and _MAX in
** terms of explicit values (which agree with those defined by C99).
*/

#define NRRD_LLONG_MAX AIR_LLONG(9223372036854775807)
#define NRRD_LLONG_MIN (-NRRD_LLONG_MAX-AIR_LLONG(1))
#define NRRD_ULLONG_MAX AIR_ULLONG(18446744073709551615)

/*
** Chances are, you shouldn't mess with these
*/

#define NRRD_COMMENT_CHAR '#'
#define NRRD_FILENAME_INCR 32
#define NRRD_COMMENT_INCR 16
#define NRRD_KEYVALUE_INCR 32
#define NRRD_LIST_FLAG "LIST"
#define NRRD_PNM_COMMENT "# NRRD>"    /* this is designed to be robust against
                                         the mungling that xv does, but no
                                         promises for any other image
                                         programs */

#define NRRD_PNG_FIELD_KEY "NRRD"     /* this is the key used for getting nrrd
                                         fields into/out of png comments */
#define NRRD_PNG_COMMENT_KEY "NRRD#"  /* this is the key used for getting nrrd
                                         comments into/out of png comments */

#define NRRD_UNKNOWN  "???"           /* how to represent something unknown in
                                         a field of the nrrd header, when it
                                         being unknown is not an error */
#define NRRD_NONE "none"              /* like NRRD_UNKNOWN, but with an air
                                         of certainty */

#ifdef __cplusplus
}
#endif



#ifdef __cplusplus
extern "C" {
#endif

/*******
******** NONE of these enums should have values set explicitly in their
******** definition.  The values should simply start at 0 (for Unknown)
******** and increase one integer per value.  The _nrrdCheckEnums()
******** sanity check assumes this, and there is no reason to use
******** explicit values for any of the enums.
*******/

/*
******** nrrdIoState* enum
**
** the various things it makes sense to get and set in nrrdIoState struct
** via nrrdIoStateGet and nrrdIoStateSet
*/
enum {
  nrrdIoStateUnknown,
  nrrdIoStateDetachedHeader,
  nrrdIoStateBareText,
  nrrdIoStateCharsPerLine,
  nrrdIoStateValsPerLine,
  nrrdIoStateSkipData,
  nrrdIoStateKeepNrrdDataFileOpen,
  nrrdIoStateZlibLevel,
  nrrdIoStateZlibStrategy,
  nrrdIoStateBzip2BlockSize,
  nrrdIoStateLast
};

/*
******** nrrdFormatType* enum
**
** the different file formats which nrrd supports
*/
enum {
  nrrdFormatTypeUnknown,
  nrrdFormatTypeNRRD,   /* 1: basic nrrd format (associated with any of
                           the magics starting with "NRRD") */
  nrrdFormatTypePNM,    /* 2: PNM image */
  nrrdFormatTypePNG,    /* 3: PNG image */
  nrrdFormatTypeVTK,    /* 4: VTK Structured Points datasets (v1.0 and 2.0) */
  nrrdFormatTypeText,   /* 5: bare ASCII text for 2D arrays */
  nrrdFormatTypeEPS,    /* 6: Encapsulated PostScript (write-only) */
  nrrdFormatTypeLast
};
#define NRRD_FORMAT_TYPE_MAX    6

/*
******** nrrdBoundary* enum
**
** when resampling, how to deal with the ends of a scanline
*/
enum {
  nrrdBoundaryUnknown,
  nrrdBoundaryPad,      /* 1: fill with some user-specified value */
  nrrdBoundaryBleed,    /* 2: copy the last/first value out as needed */
  nrrdBoundaryWrap,     /* 3: wrap-around */
  nrrdBoundaryWeight,   /* 4: normalize the weighting on the existing samples;
                           ONLY sensible for a strictly positive kernel
                           which integrates to unity (as in blurring) */
  nrrdBoundaryMirror,   /* 5: mirror folding */
  nrrdBoundaryLast
};
#define NRRD_BOUNDARY_MAX  5

/*
******** nrrdType* enum
**
** all the different types, identified by integer
**
** 18 July 03: After some consternation, GLK decided to set
** nrrdTypeUnknown and nrrdTypeDefault to the same thing, with the
** reasoning that the only times that nrrdTypeDefault is used is when
** controlling an *output* type (the type of "nout"), or rather,
** choosing not to control an output type.  As output types must be
** known, there is no confusion between being unset/unknown (invalid)
** and being simply default.
*/
enum {
  nrrdTypeUnknown=0,     /*  0: signifies "type is unset/unknown" */
  nrrdTypeDefault=0,     /*  0: signifies "determine output type for me" */
  nrrdTypeChar,          /*  1:   signed 1-byte integer */
  nrrdTypeUChar,         /*  2: unsigned 1-byte integer */
  nrrdTypeShort,         /*  3:   signed 2-byte integer */
  nrrdTypeUShort,        /*  4: unsigned 2-byte integer */
  nrrdTypeInt,           /*  5:   signed 4-byte integer */
  nrrdTypeUInt,          /*  6: unsigned 4-byte integer */
  nrrdTypeLLong,         /*  7:   signed 8-byte integer */
  nrrdTypeULLong,        /*  8: unsigned 8-byte integer */
  nrrdTypeFloat,         /*  9:          4-byte floating point */
  nrrdTypeDouble,        /* 10:          8-byte floating point */
  nrrdTypeBlock,         /* 11: size user defined at run time; MUST BE LAST */
  nrrdTypeLast
};
#define NRRD_TYPE_MAX       11
#define NRRD_TYPE_SIZE_MAX   8    /* max(sizeof()) over all scalar types */
#define NRRD_TYPE_BIGGEST double  /* this should be a basic C type which
                                     requires for storage the maximum size
                                     of all the basic C types */

/*
******** nrrdEncodingType enum
**
** how data might be encoded into a bytestream
*/
enum {
  nrrdEncodingTypeUnknown,
  nrrdEncodingTypeRaw,      /* 1: same as memory layout (modulo endianness) */
  nrrdEncodingTypeAscii,    /* 2: decimal values are spelled out in ascii */
  nrrdEncodingTypeHex,      /* 3: hexidecimal (two chars per byte) */
  nrrdEncodingTypeGzip,     /* 4: gzip'ed raw data */
  nrrdEncodingTypeBzip2,    /* 5: bzip2'ed raw data */
  nrrdEncodingTypeLast
};
#define NRRD_ENCODING_TYPE_MAX 5

/*
******** nrrdZlibStrategy enum
**
** how gzipped data is compressed
*/
enum {
  nrrdZlibStrategyUnknown,
  nrrdZlibStrategyDefault,   /* 1: default (Huffman + string match) */
  nrrdZlibStrategyHuffman,   /* 2: Huffman only */
  nrrdZlibStrategyFiltered,  /* 3: specialized for filtered data */
  nrrdZlibStrategyLast
};
#define NRRD_ZLIB_STRATEGY_MAX  3

/*
******** nrrdCenter enum
**
** node-centered vs. cell-centered
*/
enum {
  nrrdCenterUnknown,         /* 0: no centering known for this axis */
  nrrdCenterNode,            /* 1: samples at corners of things
                                (how "voxels" are usually imagined)
                                |\______/|\______/|\______/|
                                X        X        X        X   */
  nrrdCenterCell,            /* 2: samples at middles of things
                                (characteristic of histogram bins)
                                 \___|___/\___|___/\___|___/
                                     X        X        X       */
  nrrdCenterLast
};
#define NRRD_CENTER_MAX         2

/*
******** nrrdKind enum
**
** For describing the information along one axis of an array.  This is
** most important for clarifying the representation of non-scalar
** data, in order to distinguish between axes that are genuine image
** domain axes, and axes that exist just to store the multiple
** attributes per sample.  One could argue that this information
** should be per-array and not per-axis, but you still have to
** indicate which one of the axes is the attribute axis.  And, if you
** have, say, the gradient of RGB colors, you want the per-pixel 3x3
** array to have those two attribute axes tagged accordingly.
**
** More of these may be added in the future, such as when nrrd
** supports bricking.  Since nrrd is never going to be in the business
** of manipulating the kind information or supporting kind-specific
** semantics, there can be proliferation of nrrdKinds, provided
** pointless redundancy is avoided.
**
**  There is a relationship between some of these (nrrdKindSpace is a
** specific nrrdKindDomain), but currently there is no effort to
** record this meta-kind information.
**
** Keep in sync:
**   enumsNrrd.c: nrrdKind airEnum
**        axis.c: nrrdKindSize()
**        axis.c: _nrrdKindAltered()
**
** NOTE: The nrrdKindSize() function returns the valid size for these.
**
*/
enum {
  nrrdKindUnknown,
  nrrdKindDomain,            /*  1: any image domain */
  nrrdKindSpace,             /*  2: a spatial domain */
  nrrdKindTime,              /*  3: a temporal domain */
  /* -------------------------- end domain kinds */
  /* -------------------------- begin range kinds */
  nrrdKindList,              /*  4: any list of values, non-resample-able */
  nrrdKindPoint,             /*  5: coords of a point */
  nrrdKindVector,            /*  6: coeffs of (contravariant) vector */
  nrrdKindCovariantVector,   /*  7: coeffs of covariant vector (eg gradient) */
  nrrdKindNormal,            /*  8: coeffs of unit-length covariant vector */
  /* -------------------------- end arbitrary size kinds */
  /* -------------------------- begin size-specific kinds */
  nrrdKindStub,              /*  9: axis with one sample (a placeholder) */
  nrrdKindScalar,            /* 10: effectively, same as a stub */
  nrrdKindComplex,           /* 11: real and imaginary components */
  nrrdKind2Vector,           /* 12: 2 component vector */
  nrrdKind3Color,            /* 13: ANY 3-component color value */
  nrrdKindRGBColor,          /* 14: RGB, no colorimetry */
  nrrdKindHSVColor,          /* 15: HSV, no colorimetry */
  nrrdKindXYZColor,          /* 16: perceptual primary colors */
  nrrdKind4Color,            /* 17: ANY 4-component color value */
  nrrdKindRGBAColor,         /* 18: RGBA, no colorimetry */
  nrrdKind3Vector,           /* 19: 3-component vector */
  nrrdKind3Gradient,         /* 20: 3-component covariant vector */
  nrrdKind3Normal,           /* 21: 3-component covector, assumed normalized */
  nrrdKind4Vector,           /* 22: 4-component vector */
  nrrdKindQuaternion,        /* 23: (w,x,y,z), not necessarily normalized */
  nrrdKind2DSymMatrix,       /* 24: Mxx Mxy Myy */
  nrrdKind2DMaskedSymMatrix, /* 25: mask Mxx Mxy Myy */
  nrrdKind2DMatrix,          /* 26: Mxx Mxy Myx Myy */
  nrrdKind2DMaskedMatrix,    /* 27: mask Mxx Mxy Myx Myy */
  nrrdKind3DSymMatrix,       /* 28: Mxx Mxy Mxz Myy Myz Mzz */
  nrrdKind3DMaskedSymMatrix, /* 29: mask Mxx Mxy Mxz Myy Myz Mzz */
  nrrdKind3DMatrix,          /* 30: Mxx Mxy Mxz Myx Myy Myz Mzx Mzy Mzz */
  nrrdKind3DMaskedMatrix,    /* 31: mask Mxx Mxy Mxz Myx Myy Myz Mzx Mzy Mzz */
  nrrdKindLast
};
#define NRRD_KIND_MAX           31

/*
******** nrrdAxisInfo enum
**
** the different pieces of per-axis information recorded in a nrrd
*/
enum {
  nrrdAxisInfoUnknown,
  nrrdAxisInfoSize,                   /*  1: number of samples along axis */
#define NRRD_AXIS_INFO_SIZE_BIT      (1<< 1)
  nrrdAxisInfoSpacing,                /*  2: spacing between samples */
#define NRRD_AXIS_INFO_SPACING_BIT   (1<< 2)
  nrrdAxisInfoThickness,              /*  3: thickness of sample region */
#define NRRD_AXIS_INFO_THICKNESS_BIT (1<< 3)
  nrrdAxisInfoMin,                    /*  4: min pos. assoc. w/ 1st sample */
#define NRRD_AXIS_INFO_MIN_BIT       (1<< 4)
  nrrdAxisInfoMax,                    /*  5: max pos. assoc. w/ last sample */
#define NRRD_AXIS_INFO_MAX_BIT       (1<< 5)
  nrrdAxisInfoSpaceDirection,         /*  6: inter-sample vector in "space" */
#define NRRD_AXIS_INFO_SPACEDIRECTION_BIT (1<< 6)
  nrrdAxisInfoCenter,                 /*  7: cell vs. node */
#define NRRD_AXIS_INFO_CENTER_BIT    (1<< 7)
  nrrdAxisInfoKind,                   /*  8: from the nrrdKind* enum */
#define NRRD_AXIS_INFO_KIND_BIT      (1<< 8)
  nrrdAxisInfoLabel,                  /*  9: string describing the axis */
#define NRRD_AXIS_INFO_LABEL_BIT     (1<< 9)
  nrrdAxisInfoUnits,                  /* 10: from the nrrdUnit* enum */
#define NRRD_AXIS_INFO_UNITS_BIT     (1<<10)
  nrrdAxisInfoLast
};
#define NRRD_AXIS_INFO_MAX               10
#define NRRD_AXIS_INFO_ALL  \
    ((1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10))
#define NRRD_AXIS_INFO_NONE 0

/*
******** nrrdBasicInfo enum
**
** the non-per-axis (or per-array) pieces of information that could
** meaningfully be copied between nrrds (hence the void *data is not
** included).
**
** "Basic" is named after the "basic field specifications" described
** in the NRRD file format definition
*/
enum {
  nrrdBasicInfoUnknown,
  nrrdBasicInfoData,                          /*  1 */
#define NRRD_BASIC_INFO_DATA_BIT             (1<< 1)
  nrrdBasicInfoType,                          /*  2 */
#define NRRD_BASIC_INFO_TYPE_BIT             (1<< 2)
  nrrdBasicInfoBlocksize,                     /*  3 */
#define NRRD_BASIC_INFO_BLOCKSIZE_BIT        (1<< 3)
  nrrdBasicInfoDimension,                     /*  4 */
#define NRRD_BASIC_INFO_DIMENSION_BIT        (1<< 4)
  nrrdBasicInfoContent,                       /*  5 */
#define NRRD_BASIC_INFO_CONTENT_BIT          (1<< 5)
  nrrdBasicInfoSampleUnits,                   /*  6 */
#define NRRD_BASIC_INFO_SAMPLEUNITS_BIT      (1<< 6)
  nrrdBasicInfoSpace,                         /*  7 */
#define NRRD_BASIC_INFO_SPACE_BIT            (1<< 7)
  nrrdBasicInfoSpaceDimension,                /*  8 */
#define NRRD_BASIC_INFO_SPACEDIMENSION_BIT   (1<< 8)
  nrrdBasicInfoSpaceUnits,                    /*  9 */
#define NRRD_BASIC_INFO_SPACEUNITS_BIT       (1<< 9)
  nrrdBasicInfoSpaceOrigin,                   /* 10 */
#define NRRD_BASIC_INFO_SPACEORIGIN_BIT      (1<<10)
  nrrdBasicInfoMeasurementFrame,              /* 11 */
#define NRRD_BASIC_INFO_MEASUREMENTFRAME_BIT (1<<11)
  nrrdBasicInfoOldMin,                        /* 12 */
#define NRRD_BASIC_INFO_OLDMIN_BIT           (1<<12)
  nrrdBasicInfoOldMax,                        /* 13 */
#define NRRD_BASIC_INFO_OLDMAX_BIT           (1<<13)
  nrrdBasicInfoComments,                      /* 14 */
#define NRRD_BASIC_INFO_COMMENTS_BIT         (1<<14)
  nrrdBasicInfoKeyValuePairs,                 /* 15 */
#define NRRD_BASIC_INFO_KEYVALUEPAIRS_BIT    (1<<15)
  nrrdBasicInfoLast
};
#define NRRD_BASIC_INFO_MAX                      15
#define NRRD_BASIC_INFO_ALL  \
    ((1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<8)|(1<<9)|(1<<10)\
     |(1<<11)|(1<<12)|(1<<13)|(1<<14)|(1<<15))
#define NRRD_BASIC_INFO_SPACE (NRRD_BASIC_INFO_SPACE_BIT \
                               | NRRD_BASIC_INFO_SPACEDIMENSION_BIT \
                               | NRRD_BASIC_INFO_SPACEUNITS_BIT \
                               | NRRD_BASIC_INFO_SPACEORIGIN_BIT \
                               | NRRD_BASIC_INFO_MEASUREMENTFRAME_BIT)
#define NRRD_BASIC_INFO_NONE 0

/*
******** nrrdField enum
**
** the various fields we can parse in a NRRD header
**
** other things which must be kept in sync:
** arraysNrrd.c:
**    _nrrdFieldValidInImage[]
**    _nrrdFieldOnePerAxis[]
**    _nrrdFieldValidInText[]
**    _nrrdFieldRequired[]
** parseNrrd.c:
**    _nrrdReadNrrdParseInfo[]
** enumsNrrd.c:
**    nrrdField definition
** simple.c:
**    _nrrdFieldCheck[]
** write.c:
**    _nrrdFieldInteresting()
**    _nrrdSprintFieldInfo()
** to some extent, in this file:
**    nrrdAxisInfo and nrrdBasicInfo enums
** axis.c (for per-axis info):
**    _nrrdAxisInfoCopy()
** methodsNrrd.c:
**    lots of functions, but you knew that . . .
*/
enum {
  nrrdField_unknown,
  nrrdField_comment,           /*  1 */
  nrrdField_content,           /*  2 */
  nrrdField_number,            /*  3 */
  nrrdField_type,              /*  4 */
  nrrdField_block_size,        /*  5 */
  nrrdField_dimension,         /*  6 */
  nrrdField_space,             /*  7 */
  nrrdField_space_dimension,   /*  8 */
  nrrdField_sizes,             /*  9 ----- begin per-axis ----- */
  nrrdField_spacings,          /* 10 */
  nrrdField_thicknesses,       /* 11 */
  nrrdField_axis_mins,         /* 12 */
  nrrdField_axis_maxs,         /* 13 */
  nrrdField_space_directions,  /* 14 */
  nrrdField_centers,           /* 15 */
  nrrdField_kinds,             /* 16 */
  nrrdField_labels,            /* 17 */
  nrrdField_units,             /* 18 ------ end per-axis ------ */
  nrrdField_min,               /* 19 */
  nrrdField_max,               /* 20 */
  nrrdField_old_min,           /* 21 */
  nrrdField_old_max,           /* 22 */
  nrrdField_endian,            /* 23 */
  nrrdField_encoding,          /* 24 */
  nrrdField_line_skip,         /* 25 */
  nrrdField_byte_skip,         /* 26 */
  nrrdField_keyvalue,          /* 27 */
  nrrdField_sample_units,      /* 28 */
  nrrdField_space_units,       /* 29 */
  nrrdField_space_origin,      /* 30 */
  nrrdField_measurement_frame, /* 31 */
  nrrdField_data_file,         /* 32 */
  nrrdField_last
};
#define NRRD_FIELD_MAX            32

/*
******** nrrdHasNonExist* enum
**
** oh look, I'm violating my rules outline above for how the enum values
** should be ordered.  The reason for this is that its just too bizarro to
** have the logical value of both nrrdHasNonExistFalse and nrrdHasNonExistTrue
** to be (in C) true.  For instance, nrrdHasNonExist() should be able to
** return a value from this enum which also functions in a C expressions as
** the expected boolean value.  If for some reason (outide the action of
** nrrdHasNonExist(), nrrdHasNonExistUnknown is interpreted as true, that's
** probably harmlessly conservative.  Time will tell.
*/
enum {
  nrrdHasNonExistFalse,     /* 0: no non-existent values were seen */
  nrrdHasNonExistTrue,      /* 1: some non-existent values were seen */
  nrrdHasNonExistOnly,      /* 2: NOTHING BUT non-existent values were seen */
  nrrdHasNonExistUnknown,   /* 3 */
  nrrdHasNonExistLast
};
#define NRRD_HAS_NON_EXIST_MAX 3

/*
******** nrrdSpace* enum
**
** Identifies the space in which which the origin and direction
** vectors have their coordinates measured.  When a direction is named
** here (like "Left" or "Anterior"), that implies a basis vector that
** points in that direction, along which that coordinate becomes *larger*
** (this is the opposite of MetaIO, for example).
**
** All of these spaces have a well-defined expected dimension, as
** determined by nrrdSpaceDimension(), and setting a nrrd to be in
** such a space, by nrrdSpaceSet(), will automatically set nrrd->spaceDim.
**
** The first six spaces here are PATIENT-ORIENTED spaces, which are
** properly speaking aligned with the patient, and not the scanner
** itself.  But nrrdSpaceScannerXYZ and nrrdSpaceScannerXYZTime are
** DEVICE-ORIENTED spaces, irrespective of the patient, used in a
** previous version of the DICOM standard.  When the two spaces are
** lined up with normal patient orientation in the scanner,
** nrrdSpaceScannerXYZ is the same as nrrdSpaceLeftPosteriorSuperior.
** To quote Part 3 (Information Object Definitions) of the DICOM spec
** (page 275): "If a patient lies parallel to the ground, face-up on
** the table, with his feet-to-head direction same as the
** front-to-back direction of the imaging equipment, the direction of
** the axes of this patient based coordinate system and the equipment
** based coordinate system in previous versions of this Standard will
** coincide."
**
** Keep in sync:
**   enumsNrrd.c: nrrdSpace airEnum
**      simple.c: int nrrdSpaceDimension(int space)
*/
enum {
  nrrdSpaceUnknown,
  nrrdSpaceRightAnteriorSuperior,     /*  1: NIFTI-1 (right-handed) */
  nrrdSpaceLeftAnteriorSuperior,      /*  2: standard Analyze (left-handed) */
  nrrdSpaceLeftPosteriorSuperior,     /*  3: DICOM 3.0 (right-handed) */
  nrrdSpaceRightAnteriorSuperiorTime, /*  4: */
  nrrdSpaceLeftAnteriorSuperiorTime,  /*  5: */
  nrrdSpaceLeftPosteriorSuperiorTime, /*  6: */
  nrrdSpaceScannerXYZ,                /*  7: ACR/NEMA 2.0 (pre-DICOM 3.0) */
  nrrdSpaceScannerXYZTime,            /*  8: */
  nrrdSpace3DRightHanded,             /*  9: */
  nrrdSpace3DLeftHanded,              /* 10: */
  nrrdSpace3DRightHandedTime,         /* 11: */
  nrrdSpace3DLeftHandedTime,          /* 12: */
  nrrdSpaceLast
};
#define NRRD_SPACE_MAX                   12

/*
******** nrrdSpacingStatus* enum
**
** a way of describing how spacing information is known or not known for a
** given axis, as determined by nrrdSpacingCalculate
*/
enum {
  nrrdSpacingStatusUnknown,           /* 0: nobody knows,
                                         or invalid axis choice */
  nrrdSpacingStatusNone,              /* 1: neither axis->spacing nor
                                         axis->spaceDirection is set */
  nrrdSpacingStatusScalarNoSpace,     /* 2: axis->spacing set,
                                         w/out space info */
  nrrdSpacingStatusScalarWithSpace,   /* 3: axis->spacing set, but there *is*
                                         space info, which means the spacing
                                         does *not* live in the surrounding
                                         space */
  nrrdSpacingStatusDirection,         /* 4: axis->spaceDirection set, and
                                         measured according to surrounding
                                         space */
  nrrdSpacingStatusLast
};
#define NRRD_SPACING_STATUS_MAX          4

/*
******** nrrdOriginStatus* enum
**
** how origin information was or was not computed by nrrdOriginCalculate
*/
enum {
  nrrdOriginStatusUnknown,        /* 0: nobody knows, or invalid parms */
  nrrdOriginStatusDirection,      /* 1: chosen axes have spaceDirections */
  nrrdOriginStatusNoMin,          /* 2: axis->min doesn't exist */
  nrrdOriginStatusNoMaxOrSpacing, /* 3: axis->max or ->spacing doesn't exist */
  nrrdOriginStatusOkay,           /* 4: all is well */
  nrrdOriginStatusLast
};


#ifdef __cplusplus
}
#endif



#ifdef __cplusplus
extern "C" {
#endif

/*
******** NRRD_CELL_POS, NRRD_NODE_POS, NRRD_POS
******** NRRD_CELL_IDX, NRRD_NODE_IDX, NRRD_IDX
**
** the guts of nrrdAxisPos() and nrrdAxisIdx(), for converting
** between "index space" location and "position" or "world space" location,
** given the centering, min and max "position", and number of samples.
**
** Unlike nrrdAxisPos() and nrrdAxisIdx(), this assumes that center
** is either nrrdCenterCell or nrrdCenterNode, but not nrrdCenterUnknown.
*/
/* index to position, cell centering */
#define NRRD_CELL_POS(min, max, size, idx)       \
  AIR_AFFINE(0, (idx) + 0.5, (size), (min), (max))

/* index to position, node centering */
#define NRRD_NODE_POS(min, max, size, idx)       \
  AIR_AFFINE(0, (idx), (size)-1, (min), (max))

/* index to position, either centering */
#define NRRD_POS(center, min, max, size, idx)    \
  (nrrdCenterCell == (center)                         \
   ? NRRD_CELL_POS((min), (max), (size), (idx))  \
   : NRRD_NODE_POS((min), (max), (size), (idx)))

/* position to index, cell centering */
#define NRRD_CELL_IDX(min, max, size, pos)       \
  (AIR_AFFINE((min), (pos), (max), 0, (size)) - 0.5)

/* position to index, node centering */
#define NRRD_NODE_IDX(min, max, size, pos)       \
  AIR_AFFINE((min), (pos), (max), 0, (size)-1)

/* position to index, either centering */
#define NRRD_IDX(center, min, max, size, pos)    \
  (nrrdCenterCell == (center)                         \
   ? NRRD_CELL_IDX((min), (max), (size), (pos))  \
   : NRRD_NODE_IDX((min), (max), (size), (pos)))

/*
******** NRRD_SPACING
**
** the guts of nrrdAxisSpacing(), determines the inter-sample
** spacing, given centering, min and max "position", and number of samples
**
** Unlike nrrdAxisSpacing, this assumes that center is either
** nrrdCenterCell or nrrdCenterNode, but not nrrdCenterUnknown.
*/
#define NRRD_SPACING(center, min, max, size)        \
  (nrrdCenterCell == center                         \
   ? ((max) - (min))/AIR_CAST(double, size)         \
   : ((max) - (min))/(AIR_CAST(double, (size)- 1))) \

/*
******** NRRD_COORD_UPDATE
**
** This is for doing the "carrying" associated with gradually incrementing an
** array of coordinates.  Assuming that the given coordinate array "coord" has
** been incremented by adding 1 to coord[0], this macro will propagating the
** change up to higher axes (when the coordinate has reached the size on a
** lower axis.)  In addition, the final statement of the macro prevents the
** last index from going past a valid value.
**
** Assumptions:
** -- coord[] and size[] should both be arrays of unsigned integral values,
**    presumably size_t
** -- size[i] is >= 1 for all i<dim (size 0 is invalid)
** -- dim is an unsigned int (0 is ok; result is a no-op)
** Violating these will create invalid coordinate arrays or generate
** compiler warnings about comparisons between signed and unsigned.
**
** The "ddd" variable name in this and subsequent macros is an effort to
** avoid possible symbol name shadowing.
*/
#define NRRD_COORD_UPDATE(coord, size, dim)                             \
  {                                                                     \
    unsigned int ddd;                                                   \
    for (ddd=0;                                                         \
         ddd+1 < (dim) && (coord)[ddd] >= (size)[ddd];                  \
         ddd++) {                                                       \
      (coord)[ddd] = 0;                                                 \
      (coord)[ddd+1]++;                                                 \
    }                                                                   \
    if (dim) {                                                          \
      (coord)[(dim)-1] = AIR_MIN((coord)[(dim)-1], (size)[(dim)-1]-1);  \
    }                                                                   \
  }

/*
******** NRRD_COORD_INCR
**
** increments coord[idx] (by one) and then calls NRRD_COORD_UPDATE to
** propagate this change as necessary to higher numbered axes.  Does
** nothing if idx>=dim, since that would be an invalid index into
** coord[] and size[]
*/
#define NRRD_COORD_INCR(coord, size, dim, idx)                          \
  if ((idx) < (dim)) {                                                  \
    (coord)[(idx)]++;                                                   \
    NRRD_COORD_UPDATE((coord)+(idx), (size)+(idx), (dim)-(idx));        \
  }

/*
******** NRRD_INDEX_GEN
**
** Given array coordinates "coord" and sizes "size", both of length "dim",
** this calculates the linear index represented by coord (assuming lower
** coordinates are for *faster* axes), and stores it in "I".  Has the same
** assumptions as NRRD_COORD_UPDATE.
*/
#define NRRD_INDEX_GEN(I, coord, size, dim)             \
  {                                                     \
    unsigned int ddd = (dim);                           \
    (I) = 0;                                            \
    while (ddd) {                                       \
      ddd--;                                            \
      (I) = (coord)[ddd] + (size)[ddd]*(I);             \
    }                                                   \
  }

/*
******** NRRD_COORD_GEN
**
** opposite of NRRD_INDEX_GEN: going from linear index "I" to
** coordinate array "coord".
*/
#define NRRD_COORD_GEN(coord, size, dim, I)   \
  {                                           \
    unsigned int ddd;                         \
    size_t myI = (I);                         \
    for (ddd=0; ddd<(dim); ddd++) {           \
      (coord)[ddd] = myI % (size)[ddd];       \
      myI /= (size)[ddd];                     \
    }                                         \
  }

#ifdef __cplusplus
}
#endif



#include <errno.h>
#include <stddef.h>      /* for ptrdiff_t */



#ifdef __cplusplus
extern "C" {
#endif

#define NRRD nrrdBiffKey

/*
******** NrrdAxisInfo struct
**
** all the information which can sensibly be associated with
** one axis of a nrrd.  The only member which MUST be explicitly
** set to something meaningful is "size".
**
** If an axis lies conceptually along some direction in an enclosing
** space of dimension nrrd->spaceDim, then the first nrrd->spaceDim
** entries of spaceDirection[] must be non-NaN, and min, max, spacing,
** and units must NOT be set;  thickness, center, and label can still
** be used.  The mutual exclusion between axis-aligned and general
** direction information is enforced per-axis, not per-array.
**
** The min and max values give the range of positions "represented"
** by the samples along this axis.  In node-centering, "min" IS the
** position at the lowest index.  In cell-centering, the position at
** the lowest index is between min and max (a touch bigger than min,
** assuming min < max).
**
** There needs to be a one-to-one correspondence between these variables
** and the nrrdAxisInfo* enum (nrrdEnums.h), the per-axis header fields
** (see nrrdField* enum in nrrdEnums.h), and the various methods in axis.c
*/
typedef struct {
  size_t size;              /* number of elements along each axis */
  double spacing;           /* if non-NaN, distance between samples */
  double thickness;         /* if non-NaN, nominal thickness of region
                               represented by one sample along the axis. No
                               semantics relative to spacing are assumed or
                               imposed, and unlike spacing, there is no
                               sensible way to alter thickness- it is either
                               copied (as with cropping and slicing) or set to
                               NaN (when resampled). */
  double min, max;          /* if non-NaN, range of positions spanned by the
                               samples on this axis.  Obviously, one can set
                               "spacing" to something incompatible with min
                               and max: the idea is that only one (min and
                               max, or spacing) should be taken to be
                               significant at any time. */
  double spaceDirection[NRRD_SPACE_DIM_MAX];
                            /* the vector, in "space" (as described by
                               nrrd->space and/or nrrd->spaceDim), from one
                               sample to the next sample along this axis.  It
                               is the column vector of the transform from
                               index space to "space" space */
  int center;               /* cell vs. node centering (value should be one of
                               nrrdCenter{Unknown,Node,Cell} */
  int kind;                 /* what kind of information is along this axis
                               (from the nrrdKind* enum) */
  char *label,              /* short info string for each axis */
    *units;                 /* string identifying the unit */
} NrrdAxisInfo;

/*
******** Nrrd struct
**
** The struct used to wrap around the raw data array
*/
typedef struct {
  /*
  ** NECESSARY information describing the main array.  This is
  ** generally set at the same time that either the nrrd is created,
  ** or at the time that the nrrd is wrapped around an existing array
  */

  void *data;                       /* the data in memory */
  int type;                         /* a value from the nrrdType enum */
  unsigned int dim;                 /* the dimension (rank) of the array */

  /*
  ** All per-axis specific information
  */
  NrrdAxisInfo axis[NRRD_DIM_MAX];  /* axis[0] is the fastest axis in the scan-
                                       line ordering, the one who's coordinates
                                       change the fastest as the elements are
                                       accessed in the order in which they
                                       appear in memory */

  /*
  ** Optional information descriptive of whole array, some of which is
  ** meaningfuly for only some uses of a nrrd
  */
  char *content;                    /* brief account of what this data is */
  char *sampleUnits;                /* units of measurement of the values
                                       stored in the array itself (not the
                                       array axes and not space coordinates).
                                       The logical name might be "dataUnits",
                                       but that's perhaps ambiguous.  Note that
                                       these units may apply to non-scalar
                                       kinds (e.g. coefficients of a vector
                                       have the same units) */
  int space;                        /* from nrrdSpace* enum, and often
                                       implies the value of spaceDim */
  unsigned int spaceDim;            /* if non-zero, the dimension of the space
                                       in which the regular sampling grid
                                       conceptually lies.  This is a separate
                                       variable because this dimension can be
                                       different than the array dimension.
                                       The non-zero-ness of this value is in
                                       fact the primary indicator that space
                                       and orientation information is set.
                                       This identifies the number of entries in
                                       "origin" and the per-axis "direction"
                                       vectors that are taken as meaningful */
  char *spaceUnits[NRRD_SPACE_DIM_MAX];
                                    /* units for coordinates of space */
  double spaceOrigin[NRRD_SPACE_DIM_MAX];
                                    /* the location of the center the first
                                       (lowest memory address) array sample,
                                       regardless of node-vs-cell centering */
  double measurementFrame[NRRD_SPACE_DIM_MAX][NRRD_SPACE_DIM_MAX];
                                    /* if spaceDim is non-zero, this may store
                                       a spaceDim-by-spaceDim matrix which
                                       transforms vector/matrix coefficients
                                       in the "measurement frame" to those in
                                       the world space described by spaceDim
                                       (and hopefully space).  Coeff [i][j] is
                                       *column* i & *row* j, which is probably
                                       the *transpose* of what you expect.
                                       There are no semantics linking this to
                                       the "kind" of any axis, for a variety
                                       of reasons */
  size_t blockSize;                 /* for nrrdTypeBlock, block byte size */
  double oldMin, oldMax;            /* if non-NaN, and if nrrd is of integral
                                       type, extremal values for the array
                                       BEFORE it was quantized */
  void *ptr;                        /* never read or set by nrrd; use/abuse
                                       as you see fit */

  /*
  ** Comments.  Read from, and written to, header.
  ** The comment array "cmt" is NOT NULL-terminated.
  ** The number of comments is cmtArr->len.
  */
  char **cmt;
  airArray *cmtArr;

  /*
  ** Key-value pairs.
  */
  char **kvp;
  airArray *kvpArr;
} Nrrd;

struct NrrdIoState_t;
struct NrrdEncoding_t;

/*
******** NrrdFormat
**
** All information and behavior relevent to one datafile format
*/
typedef struct {
  char name[AIR_STRLEN_SMALL];    /* short identifying string */
  int isImage,    /* this format is intended solely for "2D" images, which
                     controls the invocation of _nrrdReshapeUpGrayscale()
                     if nrrdStateGrayscaleImage3D */
    readable,     /* we can read as well as write this format */
    usesDIO;      /* this format can use Direct IO */

  /* tests if this format is currently available in this build */
  int (*available)(void);

  /* (for writing) returns non-zero if a given filename could likely be
     represented by this format */
  int (*nameLooksLike)(const char *filename);

  /* (for writing) returns non-zero if a given nrrd/encoding pair will fit
     in this format */
  int (*fitsInto)(const Nrrd *nrrd, const struct NrrdEncoding_t *encoding,
                   int useBiff);

  /* (for reading) returns non-zero if what has been read in so far
     is recognized as the beginning of this format */
  int (*contentStartsLike)(struct NrrdIoState_t *nio);

  /* reader and writer */
  int (*read)(FILE *file, Nrrd *nrrd, struct NrrdIoState_t *nio);
  int (*write)(FILE *file, const Nrrd *nrrd, struct NrrdIoState_t *nio);
} NrrdFormat;

/*
******** NrrdEncoding
**
** All information and behavior relevent to one way of encoding data
**
** The data readers are responsible for memory allocation.
** This is necessitated by the memory restrictions of direct I/O
*/
typedef struct NrrdEncoding_t {
  char name[AIR_STRLEN_SMALL],    /* short identifying string */
    suffix[AIR_STRLEN_SMALL];     /* costumary filename suffix */
  int endianMatters,
    isCompression;
  int (*available)(void);
  /* The "data" and "elementNum" values have to be passed explicitly
     to read/wrote because they will be different from nrrd->data and
     nrrdElementNumber(nrrd) in the case of multiple data files.  You
     might think that the only other thing required to be passed is
     nrrdElementSize(nrrd), but no, it is in fact best to pass the
     whole Nrrd, instead of just certain attributes.  The stupid details:
             nrrd->dim: needed to know whether to put one value per line
                        in case of 1-D nrrdEncodingAscii
    nrrd->axis[0].size: need for proper formatting of nrrdEncodingAscii
            nrrd->type: needed for nrrdEncodingAscii, since its action is
                        entirely parameterized by type
       nrrd->blockSize: needed for nrrdElementSize in case of nrrdTypeBlock */
  int (*read)(FILE *file, void *data, size_t elementNum,
              Nrrd *nrrd, struct NrrdIoState_t *nio);
  int (*write)(FILE *file, const void *data, size_t elementNum,
               const Nrrd *nrrd, struct NrrdIoState_t *nio);
} NrrdEncoding;

/*
******** NrrdIoState struct
**
** Everything relating to how the nrrd is read and written.
** Multiple parameters for writing are set here (like format, encoding,
** zlib parameters).  Also, this is the place where those few parameters
** of reading are stored (like skipData and keepNrrdDataFileOpen).  Also,
** after the nrrd has been read, it is a potentially useful record of what
** it took to read it in.
*/
typedef struct NrrdIoState_t {
  char *path,               /* allows us to remember the directory
                               from whence this nrrd was "load"ed, or
                               to whence this nrrd is "save"ed, MINUS the
                               trailing "/", so as to facilitate games with
                               header-relative data files */
    *base,                  /* when "save"ing a nrrd into separate
                               header and data, the name of the header
                               file (e.g. "output.nhdr") MINUS the ".nhdr".
                               This is massaged to produce a header-
                               relative data filename.  */
    *line,                  /* buffer for saving one line from file */
    *dataFNFormat,          /* if non-NULL, the format string (containing
                               something like "%d" as a substring) to be
                               used to identify multiple detached datafiles.
                               NB: This is "format" in the sense of a printf-
                               style format string, not in the sense of a
                               file format.  This may need header-relative
                               path processing. */
    **dataFN,               /* ON READ + WRITE: array of data filenames. These
                               are not passed directly to fopen, they may need
                               header-relative path processing. Like the
                               cmtArr in the Nrrd, this array is not NULL-
                               terminated */
    *headerStringWrite;     /* ON WRITE: string from to which the header can
                               be written.  On write, it is assumed allocated
                               for as long as it needs to be (probably via a
                               first pass with learningHeaderStrlen). NOTE:
                               It is the non-NULL-ity of this which signifies
                               the intent to do string-based writing */
  const char
    *headerStringRead;      /* ON READ: like headerStringWrite, but for
                               reading the header from.  NOTE: It is the
                               non-NULL-ity of this which signifies the
                               intent to do string-based reading */
  airArray *dataFNArr;      /* for managing the above */

  FILE *headerFile,         /* if non-NULL, the file from which the NRRD
                               header is being read */
    *dataFile;              /* this used to be a central part of how the
                               I/O code worked, but now it is simply the
                               place to store the dataFile in the case of
                               keepNrrdDataFileOpen */
  unsigned int dataFileDim, /* The dimension of the data in each data file.
                               Together with dataFNArr->len, this determines
                               how many bytes should be in each data file */
    lineLen,                /* allocated size of line, including the
                               last character for \0 */
    charsPerLine,           /* when writing ASCII data in which we
                               intend only to write a huge long list
                               of numbers whose text formatting
                               implies nothing, then how many
                               characters do we limit ourselves to per
                               line */
    valsPerLine,            /* when writing ASCII data in which we DO
                               intend to sigify (or at least hint at)
                               something with the formatting, then
                               what is the max number of values to
                               write on a line */
    lineSkip,               /* if dataFile non-NULL, the number of
                               lines in dataFile that should be
                               skipped over (so as to bypass another
                               form of ASCII header preceeding raw
                               data) */
    headerStrlen,           /* ON WRITE, for NRRDs, if learningHeaderStrlen,
                               the learned strlen of the header so far */
    headerStrpos;           /* ON READ, for NRRDs, if headerStringRead is
                               non-NULL, the current location of reading
                               in the header */
  long int byteSkip;        /* exactly like lineSkip, but bytes
                               instead of lines.  First the lines are
                               skipped, then the bytes */
  /* Note that the NRRD0004 and NRRD0005 file formats indicate that a numbered
     sequence of data filenames should be indexed via a "%d" format
     specification, and that the format doc says nothing about the "min" and
     "max" fields of "data file" being only positive.  So the following three
     dataFN* fields are appropriately (signed) ints, even if all normal usage
     could also be represented with unsigned ints.  Nonetheless, the return
     from _nrrdDataFNNumber(), which gives the total number of file names, is
     still appropriately an unsigned int. This may be revisited if the file
     format itself is adjusted. */
  int dataFNMin,            /* used with dataFNFormat to identify .. */
    dataFNMax,              /* .. all the multiple detached datafiles */
    dataFNStep;             /* how to step from max to min */
  /* On the other hand, dataFNIndex ranges from 0 to (#datafiles-1),
     and not dataFNMin to dataFNMax, so it really should be unsigned */
  unsigned int dataFNIndex; /* which of the data files are being read */
  int pos,                  /* line[pos] is beginning of stuff which
                               still has yet to be parsed */
    endian,                 /* endian-ness of the data in file, for
                               those encoding/type combinations for
                               which it matters (from nrrdEndian) */
    seen[NRRD_FIELD_MAX+1], /* for error checking in header parsing */
    detachedHeader,         /* ON WRITE: request for file (NRRD format only)
                               to be split into distinct header and data.
                               This only has an effect if detaching the header
                               is not already necessary, as it is with multiple
                               data files */
    bareText,               /* when writing a plain text file, is there any
                               effort made to record the nrrd struct
                               info in the text file */
    skipData,               /* if non-zero (all formats):
                               ON READ: don't allocate memory for, and don't
                               read in, the data portion of the file (but we
                               do verify that for nrrds, detached datafiles
                               can be opened).  Note: Does NOT imply
                               keepNrrdDataFileOpen.  Warning: resulting
                               nrrd struct will have "data" pointer NULL.
                               ON WRITE: don't write data portion of file
                               (for nrrds, don't even try to open detached
                               datafiles).  Warning: can result in broken
                               noncomformant files.
                               (be careful with this) */
    skipFormatURL,          /* if non-zero for NRRD format ON WRITE:
                               skip the comment lines that document where
                               to find the NRRD file format specs */
    keepNrrdDataFileOpen,   /* ON READ: when there is only a single dataFile,
                               don't close nio->dataFile when
                               you otherwise would, when reading the
                               nrrd format. Probably used in conjunction with
                               skipData.  (currently for "unu data")
                               ON WRITE: no semantics */
    zlibLevel,              /* zlib compression level (0-9, -1 for
                               default[6], 0 for no compression). */
    zlibStrategy,           /* zlib compression strategy, can be one
                               of the nrrdZlibStrategy enums, default is
                               nrrdZlibStrategyDefault. */
    bzip2BlockSize,         /* block size used for compression,
                               roughly equivalent to better but slower
                               (1-9, -1 for default[9]). */
    learningHeaderStrlen;   /* ON WRITE, for nrrds, learn and save the total
                               length of header into headerStrlen. This is
                               used to allocate a buffer for header */
  void *oldData;            /* ON READ: if non-NULL, pointer to space that
                               has already been allocated for oldDataSize */
  size_t oldDataSize;       /* ON READ: size of mem pointed to by oldData */

  /* The format and encoding.  These are initialized to nrrdFormatUnknown
     and nrrdEncodingUnknown, respectively. USE THESE VALUES for
     any kind of initialization or flagging; DO NOT USE NULL */
  const NrrdFormat *format;
  const NrrdEncoding *encoding;
} NrrdIoState;


/******** defaults (nrrdDefault..) and state (nrrdState..) */
/* defaultsNrrd.c */
NRRDIO_EXPORT int nrrdDefaultWriteEncodingType;
NRRDIO_EXPORT int nrrdDefaultWriteBareText;
NRRDIO_EXPORT unsigned int nrrdDefaultWriteCharsPerLine;
NRRDIO_EXPORT unsigned int nrrdDefaultWriteValsPerLine;
NRRDIO_EXPORT int nrrdDefaultCenter;
NRRDIO_EXPORT double nrrdDefaultSpacing;
NRRDIO_EXPORT int nrrdStateVerboseIO;
NRRDIO_EXPORT int nrrdStateKeyValuePairsPropagate;
NRRDIO_EXPORT int nrrdStateAlwaysSetContent;
NRRDIO_EXPORT int nrrdStateDisableContent;
NRRDIO_EXPORT const char *nrrdStateUnknownContent;
NRRDIO_EXPORT int nrrdStateGrayscaleImage3D;
NRRDIO_EXPORT int nrrdStateKeyValueReturnInternalPointers;
NRRDIO_EXPORT int nrrdStateKindNoop;

/******** all the airEnums used through-out nrrd */
/*
** the actual C enums are in nrrdEnums.h; experience has shown that it
** is not particularly useful to name those enums, since the shortest
** name is best used for the airEnums here
*/
/* enumsNrrd.c */
NRRDIO_EXPORT const airEnum *const nrrdFormatType;
NRRDIO_EXPORT const airEnum *const nrrdType;
NRRDIO_EXPORT const airEnum *const nrrdEncodingType;
NRRDIO_EXPORT const airEnum *const nrrdCenter;
NRRDIO_EXPORT const airEnum *const nrrdKind;
NRRDIO_EXPORT const airEnum *const nrrdField;
NRRDIO_EXPORT const airEnum *const nrrdSpace;
NRRDIO_EXPORT const airEnum *const nrrdSpacingStatus;

/******** arrays of things (poor-man's functions/predicates) */
/* arraysNrrd.c */
NRRDIO_EXPORT const char nrrdTypePrintfStr[NRRD_TYPE_MAX+1][AIR_STRLEN_SMALL];
NRRDIO_EXPORT const size_t nrrdTypeSize[NRRD_TYPE_MAX+1];
NRRDIO_EXPORT const double nrrdTypeMin[NRRD_TYPE_MAX+1];
NRRDIO_EXPORT const double nrrdTypeMax[NRRD_TYPE_MAX+1];
NRRDIO_EXPORT const int nrrdTypeIsIntegral[NRRD_TYPE_MAX+1];
NRRDIO_EXPORT const int nrrdTypeIsUnsigned[NRRD_TYPE_MAX+1];

/******** pseudo-constructors, pseudo-destructors, and such */
/* methodsNrrd.c */
NRRDIO_EXPORT NrrdIoState *nrrdIoStateNew(void);
NRRDIO_EXPORT void nrrdIoStateInit(NrrdIoState *nio);
NRRDIO_EXPORT NrrdIoState *nrrdIoStateNix(NrrdIoState *nio);
NRRDIO_EXPORT void nrrdInit(Nrrd *nrrd);
NRRDIO_EXPORT Nrrd *nrrdNew(void);
NRRDIO_EXPORT Nrrd *nrrdNix(Nrrd *nrrd);
NRRDIO_EXPORT Nrrd *nrrdEmpty(Nrrd *nrrd);
NRRDIO_EXPORT Nrrd *nrrdNuke(Nrrd *nrrd);
NRRDIO_EXPORT int nrrdWrap_nva(Nrrd *nrrd, void *data, int type,
                             unsigned int dim, const size_t *size);
NRRDIO_EXPORT int nrrdWrap_va(Nrrd *nrrd, void *data, int type, unsigned int dim,
                            ... /* size_t sx, sy, .., axis(dim-1) size */);
NRRDIO_EXPORT void nrrdBasicInfoInit(Nrrd *nrrd, int excludeBitflag);
NRRDIO_EXPORT int nrrdBasicInfoCopy(Nrrd *nout, const Nrrd *nin,
                                  int excludeBitflag);
NRRDIO_EXPORT int nrrdCopy(Nrrd *nout, const Nrrd *nin);
NRRDIO_EXPORT int nrrdAlloc_nva(Nrrd *nrrd, int type, unsigned int dim,
                              const size_t *size);
NRRDIO_EXPORT int nrrdAlloc_va(Nrrd *nrrd, int type, unsigned int dim,
                             ... /* size_t sx, sy, .., axis(dim-1) size */);
NRRDIO_EXPORT int nrrdMaybeAlloc_nva(Nrrd *nrrd, int type, unsigned int dim,
                                   const size_t *size);
NRRDIO_EXPORT int nrrdMaybeAlloc_va(Nrrd *nrrd, int type, unsigned int dim,
                                  ... /* size_t sx, sy, .., ax(dim-1) size */);

/******** axis info related */
/* axis.c */
NRRDIO_EXPORT int nrrdKindIsDomain(int kind);
NRRDIO_EXPORT unsigned int nrrdKindSize(int kind);
NRRDIO_EXPORT int nrrdAxisInfoCopy(Nrrd *nout, const Nrrd *nin,
                                 const int *axmap, int excludeBitflag);
NRRDIO_EXPORT void nrrdAxisInfoSet_nva(Nrrd *nin, int axInfo, const void *info);
NRRDIO_EXPORT void nrrdAxisInfoSet_va(Nrrd *nin, int axInfo,
                                    ... /* const void* */);
NRRDIO_EXPORT void nrrdAxisInfoGet_nva(const Nrrd *nrrd, int axInfo, void *info);
NRRDIO_EXPORT void nrrdAxisInfoGet_va(const Nrrd *nrrd, int axInfo,
                                    ... /* ??? */);
NRRDIO_EXPORT double nrrdAxisInfoPos(const Nrrd *nrrd, unsigned int ax,
                                   double idx);
NRRDIO_EXPORT double nrrdAxisInfoIdx(const Nrrd *nrrd, unsigned int ax,
                                   double pos);
NRRDIO_EXPORT void nrrdAxisInfoPosRange(double *loP, double *hiP,
                                      const Nrrd *nrrd, unsigned int ax,
                                      double loIdx, double hiIdx);
NRRDIO_EXPORT void nrrdAxisInfoIdxRange(double *loP, double *hiP,
                                      const Nrrd *nrrd, unsigned int ax,
                                      double loPos, double hiPos);
NRRDIO_EXPORT void nrrdAxisInfoSpacingSet(Nrrd *nrrd, unsigned int ax);
NRRDIO_EXPORT void nrrdAxisInfoMinMaxSet(Nrrd *nrrd, unsigned int ax,
                                       int defCenter);
NRRDIO_EXPORT unsigned int nrrdDomainAxesGet(const Nrrd *nrrd,
                                           unsigned int axisIdx[NRRD_DIM_MAX]);
NRRDIO_EXPORT unsigned int nrrdRangeAxesGet(const Nrrd *nrrd,
                                          unsigned int axisIdx[NRRD_DIM_MAX]);
NRRDIO_EXPORT unsigned int nrrdSpatialAxesGet(const Nrrd *nrrd,
                                            unsigned int
                                            axisIdx[NRRD_DIM_MAX]);
NRRDIO_EXPORT unsigned int nrrdNonSpatialAxesGet(const Nrrd *nrrd,
                                               unsigned int
                                               axisIdx[NRRD_DIM_MAX]);
NRRDIO_EXPORT int nrrdSpacingCalculate(const Nrrd *nrrd, unsigned int ax,
                                     double *spacing,
                                     double vector[NRRD_SPACE_DIM_MAX]);
NRRDIO_EXPORT int nrrdOrientationReduce(Nrrd *nout, const Nrrd *nin,
                                      int setMinsFromOrigin);

/******** simple things */
/* simple.c */
NRRDIO_EXPORT const char *nrrdBiffKey;
NRRDIO_EXPORT unsigned int nrrdSpaceDimension(int space);
NRRDIO_EXPORT int nrrdSpaceSet(Nrrd *nrrd, int space);
NRRDIO_EXPORT int nrrdSpaceDimensionSet(Nrrd *nrrd, unsigned int spaceDim);
NRRDIO_EXPORT unsigned int nrrdSpaceOriginGet(const Nrrd *nrrd,
                                            double vector[NRRD_SPACE_DIM_MAX]);
NRRDIO_EXPORT int nrrdSpaceOriginSet(Nrrd *nrrd, const double *vector);
NRRDIO_EXPORT int nrrdOriginCalculate(const Nrrd *nrrd,
                                    unsigned int *axisIdx,
                                    unsigned int axisIdxNum,
                                    int defaultCenter, double *origin);
NRRDIO_EXPORT int nrrdContentSet_va(Nrrd *nout, const char *func,
                                  const Nrrd *nin, const char *format,
                                  ... /* printf-style arg list */ );
NRRDIO_EXPORT void nrrdDescribe(FILE *file, const Nrrd *nrrd);
NRRDIO_EXPORT int nrrdCheck(const Nrrd *nrrd);
NRRDIO_EXPORT int _nrrdCheck(const Nrrd *nrrd, int checkData, int useBiff);
NRRDIO_EXPORT size_t nrrdElementSize(const Nrrd *nrrd);
NRRDIO_EXPORT size_t nrrdElementNumber(const Nrrd *nrrd);
NRRDIO_EXPORT int nrrdSanity(void);
NRRDIO_EXPORT int nrrdSameSize(const Nrrd *n1, const Nrrd *n2, int useBiff);
NRRDIO_EXPORT void nrrdSpaceVecCopy(double dst[NRRD_SPACE_DIM_MAX],
                                  const double src[NRRD_SPACE_DIM_MAX]);
NRRDIO_EXPORT void nrrdSpaceVecScaleAdd2(double sum[NRRD_SPACE_DIM_MAX],
                                       double sclA,
                                       const double vecA[NRRD_SPACE_DIM_MAX],
                                       double sclB,
                                       const double vecB[NRRD_SPACE_DIM_MAX]);
NRRDIO_EXPORT void nrrdSpaceVecScale(double out[NRRD_SPACE_DIM_MAX],
                                   double scl,
                                   const double vec[NRRD_SPACE_DIM_MAX]);
NRRDIO_EXPORT double nrrdSpaceVecNorm(unsigned int sdim,
                                    const double vec[NRRD_SPACE_DIM_MAX]);
NRRDIO_EXPORT int nrrdSpaceVecExists(unsigned int sdim,
                                   double vec[NRRD_SPACE_DIM_MAX]);
NRRDIO_EXPORT void nrrdSpaceVecSetNaN(double vec[NRRD_SPACE_DIM_MAX]);

/******** comments related */
/* comment.c */
NRRDIO_EXPORT int nrrdCommentAdd(Nrrd *nrrd, const char *str);
NRRDIO_EXPORT void nrrdCommentClear(Nrrd *nrrd);
NRRDIO_EXPORT int nrrdCommentCopy(Nrrd *nout, const Nrrd *nin);

/******** key/value pairs */
/* keyvalue.c */
NRRDIO_EXPORT unsigned int nrrdKeyValueSize(const Nrrd *nrrd);
NRRDIO_EXPORT int nrrdKeyValueAdd(Nrrd *nrrd,
                                const char *key, const char *value);
NRRDIO_EXPORT char *nrrdKeyValueGet(const Nrrd *nrrd, const char *key);
NRRDIO_EXPORT void nrrdKeyValueIndex(const Nrrd *nrrd,
                                   char **keyP, char **valueP,
                                   unsigned int ki);
NRRDIO_EXPORT int nrrdKeyValueErase(Nrrd *nrrd, const char *key);
NRRDIO_EXPORT void nrrdKeyValueClear(Nrrd *nrrd);
NRRDIO_EXPORT int nrrdKeyValueCopy(Nrrd *nout, const Nrrd *nin);

/******** endian related */
/* endianNrrd.c */
NRRDIO_EXPORT void nrrdSwapEndian(Nrrd *nrrd);

/******** getting information to and from files */
/* formatXXX.c */
NRRDIO_EXPORT const NrrdFormat *const nrrdFormatNRRD;
NRRDIO_EXPORT const NrrdFormat *const nrrdFormatPNM;
NRRDIO_EXPORT const NrrdFormat *const nrrdFormatPNG;
NRRDIO_EXPORT const NrrdFormat *const nrrdFormatVTK;
NRRDIO_EXPORT const NrrdFormat *const nrrdFormatText;
NRRDIO_EXPORT const NrrdFormat *const nrrdFormatEPS;
/* format.c */
NRRDIO_EXPORT const NrrdFormat *const nrrdFormatUnknown;
NRRDIO_EXPORT const NrrdFormat *
  const nrrdFormatArray[NRRD_FORMAT_TYPE_MAX+1];

/* encodingXXX.c */
NRRDIO_EXPORT const NrrdEncoding *const nrrdEncodingRaw;
NRRDIO_EXPORT const NrrdEncoding *const nrrdEncodingAscii;
NRRDIO_EXPORT const NrrdEncoding *const nrrdEncodingHex;
NRRDIO_EXPORT const NrrdEncoding *const nrrdEncodingGzip;
NRRDIO_EXPORT const NrrdEncoding *const nrrdEncodingBzip2;
/* encoding.c */
NRRDIO_EXPORT const NrrdEncoding *const nrrdEncodingUnknown;
NRRDIO_EXPORT const NrrdEncoding *
  const nrrdEncodingArray[NRRD_ENCODING_TYPE_MAX+1];

/* parseNrrd.c */
/* this needs the "FILE *file" first arg for the sole reason that
   parsing a "data file: " field which identifies a LIST must then
   read in all the data filenames from the same file */
NRRDIO_EXPORT int (*nrrdFieldInfoParse[NRRD_FIELD_MAX+1])(FILE *file, Nrrd *nrrd,
                                                        NrrdIoState *nio,
                                                        int useBiff);
NRRDIO_EXPORT unsigned int _nrrdDataFNNumber(NrrdIoState *nio);
NRRDIO_EXPORT int _nrrdContainsPercentThisAndMore(const char *str, char thss);
NRRDIO_EXPORT int _nrrdDataFNCheck(NrrdIoState *nio, Nrrd *nrrd, int useBiff);

/* read.c */
NRRDIO_EXPORT int _nrrdOneLine(unsigned int *lenP, NrrdIoState *nio, FILE *file);
NRRDIO_EXPORT int nrrdLineSkip(FILE *dataFile, NrrdIoState *nio);
NRRDIO_EXPORT int nrrdByteSkip(FILE *dataFile, Nrrd *nrrd, NrrdIoState *nio);
NRRDIO_EXPORT int nrrdLoad(Nrrd *nrrd, const char *filename, NrrdIoState *nio);
NRRDIO_EXPORT int nrrdLoadMulti(Nrrd *const *nin, unsigned int ninLen,
                              const char *fnameFormat,
                              unsigned int numStart, NrrdIoState *nio);
NRRDIO_EXPORT int nrrdRead(Nrrd *nrrd, FILE *file, NrrdIoState *nio);
NRRDIO_EXPORT int nrrdStringRead(Nrrd *nrrd, const char *string,
                               NrrdIoState *nio);

/* write.c */
NRRDIO_EXPORT int nrrdIoStateSet(NrrdIoState *nio, int parm, int value);
NRRDIO_EXPORT int nrrdIoStateEncodingSet(NrrdIoState *nio,
                                       const NrrdEncoding *encoding);
NRRDIO_EXPORT int nrrdIoStateFormatSet(NrrdIoState *nio,
                                     const NrrdFormat *format);
NRRDIO_EXPORT int nrrdIoStateGet(NrrdIoState *nio, int parm);
NRRDIO_EXPORT const NrrdEncoding *nrrdIoStateEncodingGet(NrrdIoState *nio);
NRRDIO_EXPORT const NrrdFormat *nrrdIoStateFormatGet(NrrdIoState *nio);
NRRDIO_EXPORT int nrrdSave(const char *filename, const Nrrd *nrrd,
                         NrrdIoState *nio);
NRRDIO_EXPORT int nrrdSaveMulti(const char *fnameFormat,
                              const Nrrd *const *nin, unsigned int ninLen,
                              unsigned int numStart, NrrdIoState *nio);
NRRDIO_EXPORT int nrrdWrite(FILE *file, const Nrrd *nrrd,
                          NrrdIoState *nio);
NRRDIO_EXPORT int nrrdStringWrite(char **stringP, const Nrrd *nrrd,
                                NrrdIoState *nio);

/******** getting value into and out of an array of general type, and
   all other simplistic functionality pseudo-parameterized by type */
/* accessors.c */
NRRDIO_EXPORT double (*nrrdDLoad[NRRD_TYPE_MAX+1])(const void *v);
NRRDIO_EXPORT float  (*nrrdFLoad[NRRD_TYPE_MAX+1])(const void *v);
NRRDIO_EXPORT int    (*nrrdILoad[NRRD_TYPE_MAX+1])(const void *v);
NRRDIO_EXPORT unsigned int (*nrrdUILoad[NRRD_TYPE_MAX+1])(const void *v);
NRRDIO_EXPORT double (*nrrdDStore[NRRD_TYPE_MAX+1])(void *v, double d);
NRRDIO_EXPORT float  (*nrrdFStore[NRRD_TYPE_MAX+1])(void *v, float f);
NRRDIO_EXPORT int    (*nrrdIStore[NRRD_TYPE_MAX+1])(void *v, int j);
NRRDIO_EXPORT unsigned int (*nrrdUIStore[NRRD_TYPE_MAX+1])(void *v,
                                                         unsigned int j);
NRRDIO_EXPORT double (*nrrdDLookup[NRRD_TYPE_MAX+1])(const void *v, size_t I);
NRRDIO_EXPORT float  (*nrrdFLookup[NRRD_TYPE_MAX+1])(const void *v, size_t I);
NRRDIO_EXPORT int    (*nrrdILookup[NRRD_TYPE_MAX+1])(const void *v, size_t I);
NRRDIO_EXPORT unsigned int (*nrrdUILookup[NRRD_TYPE_MAX+1])(const void *v,
                                                          size_t I);
NRRDIO_EXPORT double (*nrrdDInsert[NRRD_TYPE_MAX+1])(void *v, size_t I,
                                                   double d);
NRRDIO_EXPORT float  (*nrrdFInsert[NRRD_TYPE_MAX+1])(void *v, size_t I,
                                                   float f);
NRRDIO_EXPORT int    (*nrrdIInsert[NRRD_TYPE_MAX+1])(void *v, size_t I,
                                                   int j);
NRRDIO_EXPORT unsigned int (*nrrdUIInsert[NRRD_TYPE_MAX+1])(void *v, size_t I,
                                                          unsigned int j);
NRRDIO_EXPORT int    (*nrrdSprint[NRRD_TYPE_MAX+1])(char *, const void *);


/******** permuting, shuffling, and all flavors of reshaping */
/* reorder.c */
NRRDIO_EXPORT int nrrdAxesInsert(Nrrd *nout, const Nrrd *nin, unsigned int ax);
NRRDIO_EXPORT int nrrdInvertPerm(unsigned int *invp, const unsigned int *perm,
                               unsigned int n);
NRRDIO_EXPORT int nrrdAxesPermute(Nrrd *nout, const Nrrd *nin,
                                const unsigned int *axes);
NRRDIO_EXPORT int nrrdShuffle(Nrrd *nout, const Nrrd *nin, unsigned int axis,
                            const size_t *perm);

/******** sampling, slicing, cropping */
/* subset.c */
NRRDIO_EXPORT int nrrdSlice(Nrrd *nout, const Nrrd *nin,
                          unsigned int axis, size_t pos);
NRRDIO_EXPORT int nrrdCrop(Nrrd *nout, const Nrrd *nin,
                         size_t *min, size_t *max);

#ifdef __cplusplus
}
#endif
