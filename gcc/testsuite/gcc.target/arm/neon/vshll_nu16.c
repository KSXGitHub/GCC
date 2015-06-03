/* Test the `vshll_nu16' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vshll_nu16 (void)
{
  uint32x4_t out_uint32x4_t;
  uint16x4_t arg0_uint16x4_t;

  out_uint32x4_t = vshll_n_u16 (arg0_uint16x4_t, 1);
}

/* { dg-final { scan-assembler "vshll\.u16\[ 	\]+\[qQ\]\[0-9\]+, \[dD\]\[0-9\]+, #\[0-9\]+!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
