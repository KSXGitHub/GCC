/* { dg-lto-do link } */
/* Forgot to steam in/out the number of labels for asm goto.  PR44184.  */

extern int printf (__const char *__restrict __format, ...);

int x = 1;
int main ()
{
    asm goto ("decl %0; jnz %l[a]" :: "m"(x) : "memory" : a);
    printf ("Hello world\n");
a:
    return 0;
}
