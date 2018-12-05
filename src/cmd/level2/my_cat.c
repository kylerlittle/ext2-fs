#include "my_cat.h"
#include "my_read.h"
#include "my_open.h"
#include "my_close.h"

//refer to 360 project help cat section
/*
==========================  HOW TO cat ======================================
cat filename:

   char mybuf[1024], dummy = 0;  // a null char at end of mybuf[ ]
   int n;

1. int fd = open filename for READ;
2. while( n = read(fd, mybuf[1024], 1024)){
       mybuf[n] = 0;             // as a null terminated string
       // printf("%s", mybuf);   <=== THIS works but not good
       spit out chars from mybuf[ ] but handle \n properly;
   } 
3. close(fd);
*/
int my_cat(int argc, char *argv[])
{
    if (argc < 1)
    {
        printf("my_cat: ERROR -- need a file to proceed\n");
        return -1;
    }
    return sw_kl_cat(argv[0]);
}

int sw_kl_cat(char *pathname)
{
    int n, i;
    char mybuf[BLKSIZE];
    int fd = 0;
    char dummy = 0;


    fd = sw_kl_open(pathname, 0); //make sure to open for read
    //printf("fd: %d\n", fd);
    while (n=sw_kl_read(fd, mybuf, BLKSIZE))
    {
        //printf("in while\n");
        char *cp = mybuf; //char ptr
        while ((cp < mybuf) && (*cp != 0))
        {
            //this will print the buffer
            if (*cp == '\\' && *(cp + 1) == 'n') //this checks if we are at the end of the buffer
            {
                //rmemeber that '\\' is escape
                printf("\n");
                cp += 2;
            }
            else
            {
                putchar(*cp);
                cp++;
            }
        }
       
    }
    sw_kl_close(fd);
    printf("\n");
}
