/* $Id$ */

#include "rv_avatar.h"

#include <stdio.h>
#include <stdlib.h>

#include <png.h>

void rv_avatar_generate(const char* name) {
	FILE* f = fopen(name, "wb");

	if(f == NULL) return;

	png_structp pngp = NULL;
	png_infop infop = NULL;
	png_bytep row = NULL;

	pngp = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	infop = png_create_info_struct(pngp);

	if(setjmp(png_jmpbuf(pngp))) {
		goto closeall;
	}

closeall:
	fclose(f);
	if(infop != NULL) png_free_data(pngp, infop, PNG_FREE_ALL, -1);
	if(pngp != NULL) png_destroy_write_struct(&pngp, (png_infopp)NULL);
	if(row != NULL) free(row);
}
