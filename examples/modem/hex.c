/*
 * Copyright (c) 2014-2016 IBM Corporation.
 * All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of the <organization> nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "lmic.h"
#include "modem.h"

static const int8_t hexval[] = {
    /*00-1F*/ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /*20-3F*/ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,
    /*40-5F*/ -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /*60-7F*/ -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /*80-9F*/ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /*A0-BF*/ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /*C0-DF*/ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    /*E0-FF*/ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
};

uint8_t gethex (uint8_t* dst, const uint8_t* src, uint16_t len) {
    uint8_t n = 0;
    if(len & 1) { // odd number of digits
	return 0;
    }
    while(len--) {
	int8_t v = hexval[*src++];
	if(v < 0) { // bad hex digit
	    return 0;
	}
	*dst = (*dst << 4) | v; // shift nibble
	if((len & 1) == 0) { // advance at every second digit
	    dst++;
	    n++;
	}
    }
    return n;
}

uint8_t puthex (uint8_t* dst, const uint8_t* src, uint8_t len) {
    uint8_t l = len;
    while(len--) {
	*dst++ = "0123456789ABCDEF"[*src >> 4];
	*dst++ = "0123456789ABCDEF"[*src & 0xF];
	src++;
    }
    return 2*l;
}

uint8_t int2hex (uint8_t* dst, uint32_t v) {
    uint8_t tmp[4];
    tmp[0] = v >> 24;
    tmp[1] = v >> 16;
    tmp[2] = v >>  8;
    tmp[3] = v;
    puthex(dst, tmp, 4);
    return 8;
}

uint8_t hex2int (uint32_t* n, const uint8_t* src, uint8_t len) {
    *n = 0;
    while(len--) {
	int8_t v = hexval[*src++];
	if(v < 0) { // bad hex digit
	    return 0;
	}
	*n = (*n << 4) | v; // shift nibble
    }
    return 1;
}

uint8_t dec2int (uint32_t* n, const uint8_t* src, uint8_t len) {
    *n = 0;
    while(len--) {
	uint8_t v = *src++;
	if(v < '0' || v > '9') { // bad decimal digit
	    return 0;
	}
	*n = (*n * 10) + v; // shift digit
    }
    return 1;
}

void reverse (uint8_t* dst, const uint8_t* src, uint8_t len) {
    // works in-place (but not arbitrarily overlapping)
    for(uint8_t i=0, j=len-1; i < j; i++, j--) {
	uint8_t x = src[i];
	dst[i] = src[j];
	dst[j] = x;
    }
}

uint8_t tolower (uint8_t c) {
    if(c >= 'A' && c <= 'Z') {
	c += 'a' - 'A'; // make lower case
    }
    return c;
}

uint8_t toupper (uint8_t c) {
    if(c >= 'a' && c <= 'z') {
	c -= 'a' - 'A'; // make upper case
    }
    return c;
}

uint8_t cpystr (uint8_t* dst, const char* src) {
    uint8_t n = 0;
    while( (*dst++ = *src++) != 0 ) n++;
    return n;
}

// compare buffer with nul-terminated string (case-insensitive)
uint8_t cmpstr (uint8_t* buf, uint8_t len, char* str) {
    while(len--) {
	if(tolower(*buf++) != tolower(*str++)) {
	    return 0;
	}
    }
    return (*str == 0);
}
