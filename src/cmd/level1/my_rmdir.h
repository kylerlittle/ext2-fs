#ifndef __RMDIR_H__
#define __RMDIR_H__

#include "../../util/util.h" //will be included in header

int isEmpty(MINODE *minodePtr);
void rm_child(MINODE *parent_minodePtr, char *name);
int my_rmdir(int argc, char *argv[]);

#endif