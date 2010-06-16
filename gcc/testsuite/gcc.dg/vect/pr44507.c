/* { dg-require-effective-target vect_int } */

#include <stdlib.h>
#include "tree-vect.h"

int seeIf256ByteArrayIsConstant(
  unsigned char *pArray)
{
  int index;
  unsigned int curVal, orVal, andVal;
  int bytesAreEqual = 0;

  if (pArray != 0)
    {
      for (index = 0, orVal = 0, andVal = 0xFFFFFFFF;
           index < 64;
           index += (int)sizeof(unsigned int))
        {
          curVal = *((unsigned long *)(&pArray[index]));
          orVal = orVal | curVal;
          andVal = andVal & curVal;
        }

      if (!((orVal == andVal) 
            && ((orVal >> 8) == (andVal & 0x00FFFFFF))))
        abort ();
    }
     
  return 0;
}


int main(int argc, char** argv)
{
  unsigned char array1[64] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  };

  argv = argv;
  argc = argc;

  check_vect ();

  return seeIf256ByteArrayIsConstant(&array1[0]);
}

/* { dg-final { cleanup-tree-dump "vect" } } */

