/*
 * driver_sdcard.h
 *
 *  Created on: Apr 23, 2015
 *      Author: Kevin
 */

#ifndef DRIVER_SDCARD_H_
#define DRIVER_SDCARD_H_

#include "../fsystem.h"
#include "../file.h"

extern const fsys_funmap_t sdcard_funmap;
extern const fd_funmap_t sdcard_file_funmap;

void sdcard_mount();

#endif /* DRIVER_SDCARD_H_ */
