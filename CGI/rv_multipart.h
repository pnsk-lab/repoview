/* $Id$ */

#ifndef __RV_MULTIPART_H__
#define __RV_MULTIPART_H__

struct multipart_entry {
	unsigned char* data;
	char* name;
	unsigned long long length;
};

void rv_parse_multipart(unsigned char* buffer, char* boundary, unsigned long long length);
struct multipart_entry* rv_get_multipart(const char* name);
void rv_free_multipart(void);

#endif
