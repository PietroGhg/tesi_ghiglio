/* +++Date last modified: 05-Jul-1997 */

/*
**  SNIPMATH.H - Header file for SNIPPETS math functions and macros
*/

#ifndef SNIPMATH__H
#define SNIPMATH__H

#include <math.h>
#include <stdlib.h>                             /* For free()           */
#include <string.h>                             /* For NULL & strlen()  */

typedef enum {Error_ = -1, Success_, False_ = 0, True_} Boolean_T;

/*#if !defined(WIN32) && !defined(_WIN32) && !defined(__NT__) \
      && !defined(_WINDOWS)
      #if !defined(OS2)*/
  typedef unsigned char  BYTE;
  typedef unsigned long  DWORD;
/* #endif*/
 typedef unsigned short WORD;
/*#else
 #define WIN32_LEAN_AND_MEAN
 #define NOGDI
 #define NOSERVICE
 #undef INC_OLE1
 #undef INC_OLE2
 #include <windows.h>
 #define HUGE
 #endif*/

#define NUL '\0'
#define LAST_CHAR(s) (((char *)s)[strlen(s) - 1])
#define TOBOOL(x) (!(!(x)))
#define FREE(p) (free(p),(p)=NULL)

#if defined(__cplusplus) && __cplusplus

/*
** Safe C++ inline versions
*/

/* round to integer */

inline int iround(double x)
{
      return (int)floor(x + 0.5);
}

/* round number n to d decimal points */

inline double fround(double n, unsigned d)
{
      return floor(n * pow(10., d) + .5) / pow(10., d);
}

#else

/*
** NOTE: These C macro versions are unsafe since arguments are referenced
**       more than once.
**
**       Avoid using these with expression arguments to be safe.
*/

/*
** round to integer
*/

#define iround(x) floor((x) + 0.5)

/*
** round number n to d decimal points
*/

#define fround(n,d) (floor((n)*pow(10.,(d))+.5)/pow(10.,(d)))

#endif

/*
**  Callable library functions begin here
*/

void    SetBCDLen(int n);                             /* Bcdl.C         */
long    BCDtoLong(char *BCDNum);                      /* Bcdl.C         */
void    LongtoBCD(long num, char BCDNum[]);           /* Bcdl.C         */
double  bcd_to_double(void *buf, size_t len,          /* Bcdd.C         */
                      int digits);
int     double_to_bcd(double arg, char *buf,          /* Bcdd.C         */
                      size_t length, size_t digits );
DWORD   ncomb1 (int n, int m);                        /* Combin.C       */
DWORD   ncomb2 (int n, int m);                        /* Combin.C       */
void    SolveCubic(double a, double b, double c,      /* Cubic.C        */
                  double d, int *solutions,
                  double *x);
DWORD   dbl2ulong(double t);                          /* Dbl2Long.C     */
long    dbl2long(double t);                           /* Dbl2Long.C     */
double  dround(double x);                             /* Dblround.C     */

/* Use #defines for Permutations and Combinations     -- Factoryl.C     */

#define log10P(n,r) (log10factorial(n)-log10factorial((n)-(r)))
#define log10C(n,r) (log10P((n),(r))-log10factorial(r))

double  log10factorial(double N);                     /* Factoryl.C     */

double  fibo(unsigned short term);                    /* Fibo.C         */
double  frandom(int n);                               /* Frand.C        */
double  ipow(double x, int n);                        /* Ipow.C         */
int     ispow2(int x);                                /* Ispow2.C       */
long    double ldfloor(long double a);                /* Ldfloor.C      */
int     initlogscale(long dmax, long rmax);           /* Logscale.C     */
long    logscale(long d);                             /* Logscale.C     */

float   MSBINToIEEE(float f);                         /* Msb2Ieee.C     */
float   IEEEToMSBIN(float f);                         /* Msb2Ieee.C     */
int     perm_index (char pit[], int size);            /* Perm_Idx.C     */
int     round_div(int n, int d);                      /* Rnd_Div.C      */
long    round_ldiv(long n, long d);                   /* Rnd_Div.C      */
double  rad2deg(double rad);                          /* Rad2Deg.C      */
double  deg2rad(double deg);                          /* Rad2Deg.C      */

#ifndef PI
 #define PI         (4*atan(1))
#endif

#define deg2rad(d) ((d)*PI/180)
#define rad2deg(r) ((r)*180/PI)

#ifndef PHI
 #define PHI      ((1.0+sqrt(5.0))/2.0)         /* the golden number    */
 #define INV_PHI  (1.0/PHI)                     /* the golden ratio     */
#endif

/*
**  File: ISQRT.C
*/

struct int_sqrt {
      unsigned sqrt,
               frac;
};

void usqrt(unsigned long x, struct int_sqrt *q);


#endif /* SNIPMATH__H */
