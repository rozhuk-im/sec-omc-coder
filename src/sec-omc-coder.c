/*-
 * Copyright (c) 2021 Rozhuk Ivan <rozhuk.im@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Author: Rozhuk Ivan <rozhuk.im@gmail.com>
 *
 */

/* Based on: https://github.com/fei-ke/OmcTextDecoder */
/* cc sec-omc-coder.c -O0 -DDEBUG -lm -o sec-omc-coder */


#include <sys/param.h>
#include <sys/types.h>
#include <sys/resource.h>
     
#include <inttypes.h>
#include <stdlib.h> /* malloc, exit */
#include <stdio.h> /* snprintf, fprintf */
#include <unistd.h> /* close, write, sysconf */
#include <string.h> /* bcopy, bzero, memcpy, memmove, memset, strerror... */
#include <errno.h>
#include <err.h>
#include <fcntl.h>


static const uint8_t omc_key_shift[256] = {
	1, 1, 0, 2, 2, 4, 5, 0, 4, 7, 1, 6, 5, 3, 3, 1,
	2, 5, 0, 6, 2, 2, 4, 2, 2, 3, 0, 2, 1, 2, 4, 3,
	4, 0, 0, 0, 3, 5, 3, 1, 6, 5, 6, 1, 1, 1, 0, 0,
	3, 2, 7, 7, 5, 6, 7, 3, 5, 1, 0, 7, 6, 3, 6, 5,
	4, 5, 3, 5, 1, 3, 3, 1, 5, 4, 1, 0, 0, 2, 6, 6,
	6, 6, 4, 0, 1, 1, 0, 5, 5, 4, 2, 4, 6, 1, 7, 1,
	2, 1, 1, 6, 5, 4, 7, 6, 5, 1, 6, 7, 0, 2, 6, 3,
	1, 7, 1, 1, 7, 4, 0, 4, 2, 5, 3, 1, 1, 5, 6, 0,
	3, 5, 3, 6, 5, 7, 2, 5, 6, 6, 2, 2, 3, 6, 0, 4,
	3, 2, 0, 2, 2, 3, 5, 3, 3, 2, 5, 5, 5, 1, 3, 1,
	1, 1, 4, 5, 1, 6, 2, 4, 7, 1, 4, 6, 0, 6, 4, 3,
	2, 6, 1, 6, 3, 2, 1, 6, 7, 3, 2, 1, 1, 5, 6, 7,
	2, 2, 2, 7, 4, 6, 7, 5, 3, 1, 4, 2, 7, 1, 6, 2,
	4, 1, 5, 6, 5, 4, 5, 0, 1, 1, 6, 3, 7, 2, 0, 2,
	5, 0, 1, 3, 3, 2, 6, 7, 7, 2, 5, 6, 0, 4, 1, 2,
	5, 3, 7, 6, 5, 2, 5, 2, 0, 1, 3, 1, 4, 3, 4, 2
};
static const uint8_t omc_key_xor[256] = {
	0x41, 0xc5, 0x21, 0xde, 0x6b, 0x1c, 0x95, 0x37,
	0x4e, 0x11, 0xaf, 0x06, 0xb0, 0x87, 0xdd, 0xe9,
	0x48, 0x7a, 0xc1, 0xd5, 0x44, 0x77, 0xb2, 0x91,
	0xc4, 0x1f, 0x3c, 0x39, 0x5c, 0xa8, 0x9c, 0xbb,
	0x96, 0x5b, 0x45, 0x5d, 0x6e, 0x17, 0x5d, 0x35,
	0xd4, 0xcd, 0x40, 0xb0, 0x2e, 0x02, 0xfc, 0x0c,
	0xd3, 0x50, 0xd4, 0xdd, 0x91, 0xe4, 0xbe, 0x8c,
	0x27, 0x02, 0xe5, 0xd3, 0xcc, 0x7d, 0x27, 0x42,
	0xa6, 0x3f, 0x97, 0xbd, 0x54, 0xc7, 0xfc, 0xfc,
	0x65, 0xa6, 0x51, 0x0a, 0xdf, 0x01, 0x43, 0xc7,
	0xb9, 0x12, 0xb6, 0x66, 0x60, 0xa7, 0x40, 0xef,
	0x36, 0xa2, 0xac, 0xbe, 0x0e, 0x77, 0x79, 0x02,
	0xb2, 0xb1, 0x59, 0x3f, 0x5d, 0x6d, 0xb2, 0xcd,
	0x42, 0xdc, 0x20, 0x56, 0x03, 0xc6, 0xf1, 0x5c,
	0x3a, 0x02, 0xa7, 0xb0, 0xf3, 0xff, 0x7a, 0xfc,
	0x30, 0x3f, 0xd4, 0x3b, 0x64, 0xd6, 0xd3, 0x3b,
	0xf9, 0xef, 0xca, 0x22, 0xca, 0x47, 0xc0, 0xe6,
	0xa9, 0xb0, 0xef, 0xd4, 0xda, 0x90, 0x46, 0x0a,
	0x96, 0x5f, 0xe8, 0xfc, 0x8a, 0x2d, 0xab, 0xf3,
	0x55, 0x19, 0x9a, 0x89, 0x0d, 0xdb, 0x74, 0x2e,
	0xbb, 0x3b, 0x2a, 0xa6, 0xda, 0x97, 0x65, 0x89,
	0xdc, 0x61, 0xfd, 0xc2, 0xa5, 0x9f, 0x83, 0x11,
	0x0e, 0x6a, 0xb8, 0x89, 0x63, 0x6f, 0x14, 0x12,
	0xe5, 0x71, 0x40, 0xe8, 0x4a, 0xc4, 0x9c, 0x1a,
	0x38, 0xd4, 0xba, 0x0c, 0xcd, 0x9c, 0xe0, 0xf5,
	0x1a, 0x30, 0x8b, 0x62, 0xa3, 0x33, 0xe7, 0xb1,
	0xe1, 0x61, 0x57, 0x97, 0xc0, 0x07, 0xf3, 0x9b,
	0x21, 0x86, 0x05, 0x98, 0x59, 0xd4, 0x8b, 0x3f,
	0xb0, 0xfa, 0xb9, 0x92, 0xe3, 0x97, 0x74, 0x6b,
	0xa3, 0x5b, 0xd7, 0xf3, 0x14, 0x8d, 0xb2, 0x2b,
	0x4f, 0x86, 0x06, 0x66, 0xe0, 0x34, 0x8a, 0xcd,
	0x48, 0x98, 0x29, 0xda, 0x7c, 0x48, 0x82, 0xdd
};

#define ROTL8(__b, __bits) ((uint8_t)(((__b) << (__bits)) | ((__b) >> (8 - (__bits)))))
#define ROTR8(__b, __bits) ((uint8_t)(((__b) >> (__bits)) | ((__b) << (8 - (__bits)))))

#define LOG_ERR_FMT(_err, _fmt, _args...)				\
	    if (0 != (_err))						\
		fprintf(stderr, "Error: %i - %s - " _fmt "\n",		\
		    (_err), strerror((_err)), ##_args)


static inline void
omc_decode_buf(const uint8_t *src, uint8_t *dst, const size_t size,
    const size_t off) {
	register size_t i, idx;

	for (i = 0; i < size; i ++) {
		idx = (0xff & (i + off));
		dst[i] = (omc_key_xor[idx] ^
		    ROTL8(src[i], omc_key_shift[idx]));
	}
}

static inline void
omc_encode_buf(const uint8_t *src, uint8_t *dst, const size_t size,
    const size_t off) {
	register size_t i, idx;

	for (i = 0; i < size; i ++) {
		idx = (0xff & (i + off));
		dst[i] = ROTR8((omc_key_xor[idx] ^ src[i]), omc_key_shift[idx]);
	}
}


int
main(int argc, char **argv) {
	int error = 0, in_fd = -1, out_fd = -1, op = 0;
	const char *in_fn = "/dev/stdin", *out_fn = "/dev/stdout";
	uint8_t buf[65536];
	ssize_t ior, iow;

	if (1 == argc)
		goto usage;
	if ('-' != argv[1][0])
		goto usage;

	switch (argv[1][1]) {
	case 'd': /* Decode. */
		op = 0;
		break;
	case 'e': /* Encode. */
		op = 1;
		break;
	case 'h':
	default:
usage:
		fprintf(stderr, "Usage: sec-omc-coder -deh [filename]\n"
		    " -d decode OMC to gz.\n"
		    " -e encode gz to OMC.\n"
		    " -h show help.\n"
		    "If no file name specified then read stdin.\n"
		    "Examples\n"
		    "\n"
		    "Unpack: cat cscfeature.xml | sec-omc-coder -d | gzip --decompress\n"
		    "Pack: cat cscfeature.xml | gzip --best | sec-omc-coder -e\n"
		    "\n");
		return (0);
	}

	if (3 == argc) {
		in_fn = argv[2];
		out_fn = argv[2];
	} /* else No arg, stdin -> stdout. */

	/* Open files. */
	in_fd = open(in_fn, O_RDONLY);
	if (-1 == in_fd) {
		error = errno;
		LOG_ERR_FMT(error, "Fail to open in file: %s", in_fn);
		goto err_out;
	}
	out_fd = open(out_fn, O_WRONLY);
	if (-1 == out_fd) {
		error = errno;
		LOG_ERR_FMT(error, "Fail to open out file: %s", out_fn);
		goto err_out;
	}

	/* Process loop. */
	for (size_t processed = 0;;) {
		/* Read. */
		ior = read(in_fd, buf, sizeof(buf));
		if (-1 == ior) {
			error = errno;
			LOG_ERR_FMT(error, "Fail to read file: %s", in_fn);
			goto err_out;
		}
		if (0 == ior) /* EOF. */
			break;
		/* Transform data. */
		if (0 == op) { /* Decode. */
			omc_decode_buf(buf, buf, (size_t)ior, processed);
		} else { /* Encode. */
			omc_encode_buf(buf, buf, (size_t)ior, processed);
		}
		processed += (size_t)ior;
		/* Write. */
		for (ssize_t woff = 0; woff < ior; woff += ior) {
			iow = write(out_fd, (buf + woff),
			    (size_t)(ior - woff));
			if (-1 == iow) {
				error = errno;
				LOG_ERR_FMT(error,
				    "Fail to write file: %s", out_fn);
				goto err_out;
			}
			if (0 == iow) { /* EOF. */
				error = errno;
				LOG_ERR_FMT(error,
				    "Write EOF, file: %s", out_fn);
				goto err_out;
			}
		}
	}

err_out:
	close(out_fd);
	close(in_fd);

	return (error);
}
