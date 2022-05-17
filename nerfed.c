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

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <getopt.h>
#include <errno.h>

/* Use included .so unless path is given */
#ifndef PRELOAD_SO_PATH
extern const uint8_t libeat_function_so[];
extern const size_t libeat_function_so_len;
#define TEMPFILE_PATTERN "/tmp/libeat-function.XXXXXX.so"
#endif

#define STRINGIFY(x) _STRINGIFY(x)
#define _STRINGIFY(x) #x

#define NERF_ENV(part) ("LIBEAT_FUNCTION_" part "=1")

int main(int argc, char *argv[]) {
	/* Parse arguments */
	bool options_help = false;
	bool options_chmod = false;
	bool options_mknod = false;
	while (1) {
		int this_option_optind = optind ? optind : 1;
		int option_index = 0;
		static struct option long_options[] = {
			{"help",    no_argument,       0,  0 },
			{"chmod",   no_argument,       0,  0 },
			{"mknod",   no_argument,       0,  0 },
			{0,         0,                 0,  0 }
		};

		/* Check that we are still within supplied arguments */
		if (this_option_optind >= argc) {
			fprintf(stderr, "nerfed: missing command\n");
			options_help = true;
			break;
		}

		/* Only allow options before command by aborting at the first
		 * non-option argument */
		if (argv[this_option_optind][0] != '-')
			break;

		int c = getopt_long(argc, argv, "hcn",
				long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 0:
			/* Handle long-opts */
			switch (option_index) {
			case 0:
				options_help = true;
			case 1:
				options_chmod = true;
			case 2:
				options_mknod = true;
			}
			break;

		case 'h':
			options_help = true;
			break;

		case 'c':
			options_chmod = true;
			break;

		case 'n':
			options_mknod = true;
			break;

		case '?':
			options_help = true;
			break;

		default:
			options_help = true;
		}

		/* Quick exit if help requested */
		if (options_help)
			break;
	}

	if (options_help) {
		/* Show help and exit */
		fprintf(stderr,
				"Usage: nerfed [OPTIONS] <COMMAND> [COMMAND OPTIONS]\n"
				"Overloads chosen libc functions using LD_PRELOAD and runs command.\n"
				"Nerfing means to have the functions not do anything, and return success.\n"
				"\n"
				"  -c, --chmod  Nerf chmod, fchmodat\n"
				"  -m, --mknod  Nerf mknod, mknodat\n"
				"  -h, --help   Show this help\n"
				"\n"
				"https://github.com/zqad/nerfed/\n");
		return 1;
	}

	/* Act on options, use environment to communicate with LD_PRELOAD
	 * library */
	if (options_chmod) {
		if (putenv(NERF_ENV("CHMOD")) < 0) {
			perror("putenv");
			return 1;
		}
	}
	if (options_mknod) {
		if (putenv(NERF_ENV("MKNOD")) < 0) {
			perror("putenv");
			return 1;
		}
	}

#ifdef PRELOAD_SO_PATH
	/* Prepare preload environment to point to the .so on disk */
	char ld_preload_env[] = "LD_PRELOAD=" STRINGIFY(PRELOAD_SO_PATH);
#else
	/* Write the .so to a temporary file, and use that as LD_PRELOAD */

	/* Create tempfile */
	char file[] = TEMPFILE_PATTERN;
	int fd = mkstemps(file, 3);
	if (fd < 0) {
		perror("Failed to create tempfile");
		return 1;
	}
	
	/* Write .so content to temp file */
	int written = 0;
	while (written < libeat_function_so_len) {
		int r = write(fd, &libeat_function_so[written], libeat_function_so_len - written);
		if (r < 0) {
			perror("Failed to write LD_PRELOAD so");
			return 1;
		}
		written += r;
	}
	close(fd);

	/* Set up preload environment */
	char ld_preload_env[] = "LD_PRELOAD=" TEMPFILE_PATTERN;
	strcpy(&ld_preload_env[11], file);
#endif
	
	/* Deploy LD_PRELOAD environment */
	if (putenv(ld_preload_env) < 0) {
		perror("putenv");
		return 1;
	}

#ifndef PRELOAD_SO_PATH
	/* Run command in a child to be able to clean up temp file */
	int child = fork();
	if (child < 0) {
		perror("fork");
		return 1;
	}

	if (child > 0) {
		/* Still in parent. Cleanup and replicate exit status */
		int status, r;
		r = waitpid(child, &status, 0);
		unlink(file);
		if (r < 0) {
			perror("waitpid");
			return 1;
		}
		return WEXITSTATUS(status);
	}
#endif

	/* exec requested command */
	execvp(argv[optind], &argv[optind]);

	/* Exec failed */
	fprintf(stderr, "nerfed: %s: %s\n", argv[optind], strerror(errno));
	return 127;
}
