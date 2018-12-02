//my_mkdir.h
#ifndef __MKDIR_H__
#define __MKDIR_H__

#include "../../util/util.h" //will be included in header


int my_mkdir(int argc, char*argv[]);
void mk_dir(char path[MAX_FILENAME_LEN]);
int sw_kl_mkdir(MINODE *minodePtr, char *child);

#endif