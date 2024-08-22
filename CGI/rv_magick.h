/* $Id$ */

#ifndef __RV_MAGICK_H__
#define __RV_MAGICK_H__

#include <stdbool.h>

void rv_init_magick(void);
bool rv_resize_picture(const char* input, const char* output, char** reason);

#endif
