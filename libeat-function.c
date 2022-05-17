/*
 * MIT License
 *
 * Copyright (c) 2022 Jonas Eriksson, Up to Code AB
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dlfcn.h>

#define ENV_PREFIX "LIBEAT_FUNCTION_"

int env_check(const char *check) {
	char *result = getenv(check);
	/* No such env variable */
	if (result == NULL)
		return 0;

	/* Env variable is empty */
	if (*result == 0)
		return 0;

	/* Seems to be there and non-empty */
	return 1;
}

int chmod(const char *path, __mode_t mode) {
	static int (*libc_chmod)(const char *, __mode_t) = NULL;

	if (env_check(ENV_PREFIX "CHMOD"))
		return 0;

	if (libc_chmod == NULL)
		libc_chmod = dlsym(RTLD_NEXT, "chmod");

	return libc_chmod(path, mode);
}

int fchmodat(int fd, const char *path, __mode_t mode, int flag) {
	static int (*libc_fchmodat)(int, const char *, __mode_t, int) = NULL;

	if (env_check(ENV_PREFIX "CHMOD"))
		return 0;

	if (libc_fchmodat == NULL)
		libc_fchmodat = dlsym(RTLD_NEXT, "fchmodat");

	return libc_fchmodat(fd, path, mode, flag);
}

int mknod(const char *pathname, mode_t mode, dev_t dev) {
	static int (*libc_mknod)(const char *, mode_t, dev_t) = NULL;

	if (env_check(ENV_PREFIX "MKNOD"))
		return 0;

	if (libc_mknod == NULL)
		libc_mknod = dlsym(RTLD_NEXT, "mknod");

	return libc_mknod(pathname, mode, dev);
}

int mknodat(int dirfd, const char *pathname, mode_t mode, dev_t dev) {
	static int (*libc_mknodat)(int, const char *, mode_t, dev_t) = NULL;

	if (env_check(ENV_PREFIX "MKNOD"))
		return 0;

	if (libc_mknodat == NULL)
		libc_mknodat = dlsym(RTLD_NEXT, "mknodat");

	return libc_mknodat(dirfd, pathname, mode, dev);
}
