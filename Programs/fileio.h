/*
 * fileio.h
 *
 *  Created on: Apr 16, 2015
 *      Author: Kevin
 */

#ifndef FILEIO_H_
#define FILEIO_H_

#include "osprogram.h"

int write_main(char* argv[], int argc);
int read_main(char* argv[], int argc);
int ioctl_main(char* argv[], int argc);
int open_main(char* argv[], int argc);
int close_main(char* argv[], int argc);
int ls_main(char* argv[], int argc);
int cd_main(char* argv[], int argc);
int cat_main(char* argv[], int argc);
int rm_main(char* argv[], int argc);
int touch_main(char* argv[], int argc);
int mkdir_main(char* argv[], int argc);
int cp_main(char* argv[], int argc);
int mv_main(char* argv[], int argc);
int wc_main(char* argv[], int argc);

#endif /* FILEIO_H_ */
