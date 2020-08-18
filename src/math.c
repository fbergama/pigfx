/*
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          ReactOS CRT
 * FILE:             lib/sdk/crt/math/cos.c
 * PURPOSE:          Generic C Implementation of cos
 * PROGRAMMER:       Timo Kreuzer (timo.kreuzer@reactos.org)
 */

#ifdef _MSC_VER
#pragma warning(suppress:4164) /* intrinsic not declared */
#pragma function(cos)
#endif /* _MSC_VER */

#define PRECISION 9
#define M_PI 3.141592653589793238462643

static double cos_off_tbl[] = {0.0, -M_PI/2., 0, -M_PI/2.};
static double cos_sign_tbl[] = {1,-1,-1,1};

double
cos(double x)
{
    int quadrant;
    double x2, result;

    /* Calculate the quadrant */
    quadrant = (int)(x * (2./M_PI));

    /* Get offset inside quadrant */
    x = x - quadrant * (M_PI/2.);

    /* Normalize quadrant to [0..3] */
    quadrant = quadrant & 0x3;

    /* Fixup value for the generic function */
    x += cos_off_tbl[quadrant];

    /* Calculate the negative of the square of x */
    x2 = - (x * x);

    /* This is an unrolled taylor series using <PRECISION> iterations
     * Example with 4 iterations:
     * result = 1 - x^2/2! + x^4/4! - x^6/6! + x^8/8!
     * To save multiplications and to keep the precision high, it's performed
     * like this:
     * result = 1 - x^2 * (1/2! - x^2 * (1/4! - x^2 * (1/6! - x^2 * (1/8!))))
     */

    /* Start with 0, compiler will optimize this away */
    result = 0;

#if (PRECISION >= 10)
    result += 1./(1.*2*3*4*5*6*7*8*9*10*11*12*13*14*15*16*17*18*19*20);
    result *= x2;
#endif
#if (PRECISION >= 9)
    result += 1./(1.*2*3*4*5*6*7*8*9*10*11*12*13*14*15*16*17*18);
    result *= x2;
#endif
#if (PRECISION >= 8)
    result += 1./(1.*2*3*4*5*6*7*8*9*10*11*12*13*14*15*16);
    result *= x2;
#endif
#if (PRECISION >= 7)
    result += 1./(1.*2*3*4*5*6*7*8*9*10*11*12*13*14);
    result *= x2;
#endif
#if (PRECISION >= 6)
    result += 1./(1.*2*3*4*5*6*7*8*9*10*11*12);
    result *= x2;
#endif
#if (PRECISION >= 5)
    result += 1./(1.*2*3*4*5*6*7*8*9*10);
    result *= x2;
#endif
    result += 1./(1.*2*3*4*5*6*7*8);
    result *= x2;

    result += 1./(1.*2*3*4*5*6);
    result *= x2;

    result += 1./(1.*2*3*4);
    result *= x2;

    result += 1./(1.*2);
    result *= x2;

    result += 1;

    /* Apply correct sign */
    result *= cos_sign_tbl[quadrant];

    return result;
}

float cosf(float x)
{
    return ((float)cos((double)x));
}

 /*
 * COPYRIGHT:        See COPYING in the top level directory
 * PROJECT:          ReactOS CRT
 * FILE:             lib/sdk/crt/math/sin.c
 * PURPOSE:          Generic C Implementation of sin
 * PROGRAMMER:       Timo Kreuzer (timo.kreuzer@reactos.org)
 */

#ifdef _MSC_VER
#pragma warning(suppress:4164) /* intrinsic not declared */
#pragma function(sin)
#endif /* _MSC_VER */

/*#define PRECISION 9
#define M_PI 3.141592653589793238462643*/

static double sin_off_tbl[] = {0.0, -M_PI/2., 0, -M_PI/2.};
static double sin_sign_tbl[] = {1,-1,-1,1};

double
sin(double x)
{
    int quadrant;
    double x2, result;

    /* Calculate the quadrant */
    quadrant = (int)(x * (2./M_PI));

    /* Get offset inside quadrant */
    x = x - quadrant * (M_PI/2.);

    /* Normalize quadrant to [0..3] */
    quadrant = (quadrant - 1) & 0x3;

    /* Fixup value for the generic function */
    x += sin_off_tbl[quadrant];

    /* Calculate the negative of the square of x */
    x2 = - (x * x);

    /* This is an unrolled taylor series using <PRECISION> iterations
     * Example with 4 iterations:
     * result = 1 - x^2/2! + x^4/4! - x^6/6! + x^8/8!
     * To save multiplications and to keep the precision high, it's performed
     * like this:
     * result = 1 - x^2 * (1/2! - x^2 * (1/4! - x^2 * (1/6! - x^2 * (1/8!))))
     */

    /* Start with 0, compiler will optimize this away */
    result = 0;

#if (PRECISION >= 10)
    result += 1./(1.*2*3*4*5*6*7*8*9*10*11*12*13*14*15*16*17*18*19*20);
    result *= x2;
#endif
#if (PRECISION >= 9)
    result += 1./(1.*2*3*4*5*6*7*8*9*10*11*12*13*14*15*16*17*18);
    result *= x2;
#endif
#if (PRECISION >= 8)
    result += 1./(1.*2*3*4*5*6*7*8*9*10*11*12*13*14*15*16);
    result *= x2;
#endif
#if (PRECISION >= 7)
    result += 1./(1.*2*3*4*5*6*7*8*9*10*11*12*13*14);
    result *= x2;
#endif
#if (PRECISION >= 6)
    result += 1./(1.*2*3*4*5*6*7*8*9*10*11*12);
    result *= x2;
#endif
#if (PRECISION >= 5)
    result += 1./(1.*2*3*4*5*6*7*8*9*10);
    result *= x2;
#endif
    result += 1./(1.*2*3*4*5*6*7*8);
    result *= x2;

    result += 1./(1.*2*3*4*5*6);
    result *= x2;

    result += 1./(1.*2*3*4);
    result *= x2;

    result += 1./(1.*2);
    result *= x2;

    result += 1;

    /* Apply correct sign */
    result *= sin_sign_tbl[quadrant];

    return result;
}
 
float sinf(float x)
{
    return ((float)sin((double)x));
}
 
/*
 * COPYRIGHT:       BSD - See COPYING.ARM in the top level directory
 * PROJECT:         ReactOS CRT library
 * PURPOSE:         Portable implementation of sqrt
 * PROGRAMMER:      Timo Kreuzer (timo.kreuzer@reactos.org)
 */

#include <math.h>
//#include <assert.h>

double
sqrt(
    double x)
{
    const double threehalfs = 1.5;
    const double x2 = x * 0.5;
    long long bits;
    double inv, y;

    /* Handle special cases */
    if (x == 0.0)
    {
        return x;
    }
    else if (x < 0.0)
    {
        return -NAN;
    }

    /* Convert into a 64  bit integer */
    bits = *(long long *)&x;

    /* Check for !finite(x) */
    if ((bits & 0x7ff7ffffffffffffLL) == 0x7ff0000000000000LL)
    {
        return x;
    }

    /* Step 1: quick approximation of 1/sqrt(x) with bit magic
       See http://en.wikipedia.org/wiki/Fast_inverse_square_root */
    bits = 0x5fe6eb50c7b537a9ll - (bits >> 1);
    inv = *(double*)&bits;

    /* Step 2: 3 Newton iterations to approximate 1 / sqrt(x) */
    inv = inv * (threehalfs - (x2 * inv * inv));
    inv = inv * (threehalfs - (x2 * inv * inv));
    inv = inv * (threehalfs - (x2 * inv * inv));

    /* Step 3: 1 additional Heron iteration has shown to maximize the precision.
       Normally the formula would be: y = (y + (x / y)) * 0.5;
       Instead we use the inverse sqrt directly */
    y = ((1 / inv) + (x * inv)) * 0.5;

    //assert(y == (double)((y + (x / y)) * 0.5));
    /* GCC BUG: While the C-Standard requires that an explicit cast to
       double converts the result of a computation to the appropriate
       64 bit value, our GCC ignores this and uses an 80 bit FPU register
       in an intermediate value, so we need to make sure it is stored in
       a memory location before comparison */
//#if DBG
//    {
//        volatile double y1 = y, y2;
//        y2 = (y + (x / y)) * 0.5;
//        assert(y1 == y2);
//    }
//#endif

    return y;
}

float
sqrtf(float x)
{
   return (float)sqrt((double)x);
}
