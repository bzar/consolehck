/*
Copyright (c) 2011-2013 Steven Arnow
'utf8.c' - This file is part of libdarnit

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.

	3. This notice may not be removed or altered from any source
	distribution.
*/

#include "utf8.h"
#include <stdlib.h>
#include <stdio.h>

/* Internal, should not be called from your code */
int utf8GetCharLength(const unsigned char *str) {
	if (!((*str & 0xC0) ^ 0x80))
		return 0;
	if (!(*str & 0x80))
		return 1;
	if (!((*str & 0xE0) ^ 0xC0))
		return 2;
	if (!((*str & 0xF0) ^ 0xE0))
		return 3;
	if (!((*str & 0xF8) ^ 0xF0))
		return 4;
	
	return 0;
}


/* Internal, should not be called from your code */
int utf8Validate(const unsigned char *str) {
	int i, j;

	j = utf8GetCharLength(str);
	if (j == 0)
		return -1;
	
	for (i = 1; i < j; i++)
		if (((str[i] & 0xC0) ^ 0x80))
			return -1;
	if (str[0] == 0xC0 || str[i] == 0xC1)
		return -1;
	if (str[0] >= 0xF5)
		return -1;
	
	return 0;
}


/* Okay to call */
unsigned int utf8GetChar(const char *str_s) {
	const unsigned char *str = (const unsigned char *) str_s;
	int i;
	unsigned int chr, len, shift;

	if ((len = utf8GetCharLength(str)) == 0)
		return UTF8_REPLACEMENT_CHAR;
	if (!utf8Validate(str) == -1)
		return UTF8_REPLACEMENT_CHAR;

	shift = 1;
	if (len > 1) shift += len;
		chr = (*str & (0xFF >> shift));
	chr <<= (len-1) * 6;

	for (i = 1; i < len; i++) 
		chr += ((unsigned int) str[i] & 0x3F) << (len - i - 1) * 6;

	return (chr > UTF8_CHAR_LIMIT) ? UTF8_REPLACEMENT_CHAR : chr;
}


/* Okay to call */
int utf8GetValidatedCharLength(const char *str_s) {
	int len;

	if (*str_s == 0)
		return 0;
	
	len = utf8GetCharLength((const unsigned char *) str_s);
	if (utf8Validate((const unsigned char *) str_s) == -1)
		return 1;
	
	return (len > 0) ? len : 1;
}


/* Okay to call */
int utf8FindCharIndex(const char *str_s, unsigned int pos) {
	int i, j;

	for (i = j = 0; str_s[i] != 0 && j != pos; j++)
		i += utf8GetValidatedCharLength(&str_s[i]);
	return (j != pos) ? -1 : i;
}


/* Okay to call */
const char *utf8FindStartByCharacterPos(const char *str_s, unsigned int pos) {
	int i;

	i = utf8FindCharIndex(str_s, pos);
	return (i != -1) ? &str_s[i] : NULL;
}


/* Okay to call */
int utf8CountedStringSize(const char *str_s, unsigned int chars) {
	int i, j;

	for (i = j = 0; i < chars; i++)
		j += utf8GetValidatedCharLength(&str_s[j]);
	return j;
}


/* Okay to call */
int utf8GetGlyphsInString(const char *str_s) {
	int i, j;

	for (i = j = 0; str_s[i] != 0; j++)
		i += utf8GetValidatedCharLength(&str_s[i]);
	return j;
}


/* Okay to call, I guess */
int utf8EncodedLength(unsigned int ch) {
	if (ch < 0x80)
		return 1;
	if (ch < 0x800)
		return 2;
	if (ch < 0x10000)
		return 3;
	return 4;
}


/* Okay to call */
int utf8Encode(unsigned int ch, char *str_s, int buf_len) {
	int i, j;
	unsigned char *str = (unsigned char *) str_s;

	if ((i = j = utf8EncodedLength(ch)) == 1) {
		*str = ch;
		return 1;
	}

	if (buf_len < i)
		return 0;

	for (; j > 1; j--)
		str[j-1] = 0x80 | (0x3F & (ch >> ((i - j) * 6)));

	*str = (~0) << (8 - i);
	*str |= (ch >> (i * 6 - 6));


	return i;
}


// Copyright (c) 2008-2009 Bjoern Hoehrmann <bjoern@hoehrmann.de>
// See http://bjoern.hoehrmann.de/utf-8/decoder/dfa/ for details.

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

static const unsigned char utf8d[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
  8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
  0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
  0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
  0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
  1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
  1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
  1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
};

unsigned int inline utf8Decode(unsigned int* state, unsigned int* codep, unsigned int byte) {
  unsigned int type = utf8d[byte];

  *codep = (*state != UTF8_ACCEPT) ?
    (byte & 0x3fu) | (*codep << 6) :
    (0xff >> type) & (byte);

  *state = utf8d[256 + *state*16 + type];
  return *state;
}

int utf8CountCodePoints(unsigned char const* s, int *count) {
  unsigned int codepoint;
  unsigned int state = 0;

  for (*count = 0; *s; ++s)
    if (!utf8Decode(&state, &codepoint, *s))
      *count += 1;

  return state != UTF8_ACCEPT;
}

// Added 2013 by Teemu Erkkola, public domain
int unicodeStringLength(unsigned int const* codepoints)
{
  int size = 0;
  while(codepoints[size] != 0) ++size;
  return size;
}

int utf8EncodedStringLength(unsigned int const* codepoints)
{
  unsigned int const* p = codepoints;
  int encodedLength = 0;
  while(*p != 0)
  {
    encodedLength += utf8EncodedLength(*p);
    ++p;
  }

  return encodedLength;
}

void utf8EncodeString(unsigned int const* codepoints, char* result)
{
  unsigned int const* p = codepoints;

  unsigned int pos = 0;
  while(*p != 0)
  {
    int len = utf8EncodedLength(*p);
    utf8Encode(*p, result + pos, len);
    pos += len;
    ++p;
  }
}

void utf8DecodeString(const char *chars, unsigned int* result)
{
  unsigned int codepoint;
  unsigned int state = 0;
  int i = 0;
  char const* s;
  for (s = chars; *s; ++s)
  {
    if (!utf8Decode(&state, &codepoint, *s))
    {
      result[i] = codepoint;
      ++i;
    }
  }
}
