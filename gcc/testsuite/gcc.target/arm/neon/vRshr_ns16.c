/* Test the `vRshr_ns16' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vRshr_ns16 (void)
{
  int16x4_t out_int16x4_t;
  int16x4_t arg0_int16x4_t;

  out_int16x4_t = vrshr_n_s16 (arg0_int16x4_t, 1);
}

/* { dg-final { scan-assembler "vrshr\.s16\[ 	\]+\[dD\]\[0-9\]+, \[dD\]\[0-9\]+, #\[0-9\]+!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
/* { dg-final { cleanup-saved-temps } } */
