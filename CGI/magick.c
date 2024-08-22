/* $Id$ */

#include "rv_magick.h"

#include "rv_util.h"

#include "../config.h"

#include <stdio.h>
#include <stdbool.h>

#include <wand/magick_wand.h>

void rv_init_magick(void) {}

bool rv_resize_picture(const char* input, const char* output, char** reason) {
	MagickWand* wand = NULL;
	wand = NewMagickWand();
	if(MagickReadImage(wand, input)) {
		int width = MagickGetImageWidth(wand);
		int height = MagickGetImageHeight(wand);

		double scale = 0;
		if(width >= height) {
			scale = (double)256 / width;
		} else if(width < height) {
			scale = (double)256 / height;
		}

		MagickResizeImage(wand, width * scale, height * scale, LanczosFilter, 1);
		MagickSetCompressionQuality(wand, 80);
		MagickWriteImage(wand, output);

		if(wand) DestroyMagickWand(wand);
		return true;
	}
	if(wand) DestroyMagickWand(wand);
	*reason = rv_strdup("Magick error");
	return false;
}
