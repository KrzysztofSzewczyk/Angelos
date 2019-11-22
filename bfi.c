
/* asm2bf
 *
 * Copyright (C) Krzysztof Palaiologos Szewczyk, 2019.
 * License: MIT
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * src;
long lof, ip = -1;

void match(int dir) {
    int level = 1;
    char ipvalue;
    do {
        ip += dir;
		#ifndef BFI_NOCHECKS
        if (ip < 0 || ip >= lof) {
            puts("Mismatched brackets");
            exit(EXIT_FAILURE);
        }
		#endif
        ipvalue = src[ip];
        if (ipvalue == 91)
            level += dir;
        else if (ipvalue == 93)
            level -= dir;
    } while (level > 0);
}

int main(int argc, char * argv[]) {
    FILE * infile;
    long mp = 0, maxmp = 1023;
    int n, ic;
    unsigned short int * mem;
	#ifndef BFI_NOCHECKS
    if (argc != 2) {
        puts("Usage: bfi src.b");
        return EXIT_FAILURE;
    }
	#endif
    infile = fopen(argv[1], "rb");
	#ifndef BFI_NOCHECKS
    if (infile == NULL) {
        puts("Error opening input file");
        return EXIT_FAILURE;
    }
	
    if (fseek(infile, 0L, SEEK_END) != 0) {
        puts("Error determining length of input file");
        return EXIT_FAILURE;
    }
	#else
	fseek(infile, 0L, SEEK_END);
	#endif
    lof = ftell(infile);
	#ifndef BFI_NOCHECKS
    if (lof == -1) {
        puts("Error determining length of input file");
        return EXIT_FAILURE;
    }
    if (fseek(infile, 0L, SEEK_SET) != 0) {
        puts("Error determining length of input file");
        return EXIT_FAILURE;
    }
    if (lof == 0)
        return EXIT_SUCCESS;
	#else
	fseek(infile, 0L, SEEK_SET);
	#endif
    src = (char *) calloc(lof + 2, sizeof(char));
	#ifndef BFI_NOCHECKS
    if (src == NULL) {
        puts("Program too big to fit in memory");
        return EXIT_FAILURE;
    }
    if (fread(src, sizeof(char), lof, infile) < (unsigned) lof) {
        puts("Error reading input file");
        return EXIT_FAILURE;
    }
    if (fclose(infile) == -1) {
        puts("Error closing input file");
        return EXIT_FAILURE;
    }
	#else
		fread(src, sizeof(char), lof, infile);
		fclose(infile);
	#endif
    mem = calloc(1024, sizeof(unsigned short int));
	#ifndef BFI_NOCHECKS
    if (mem == NULL) {
        puts("Out of memory");
        return EXIT_FAILURE;
    }
	#endif
    while (++ip < lof) {
        switch (src[ip]) {
            case '>':
                if (mp >= maxmp) {
                    mem = realloc(mem, (maxmp + 1024) * sizeof(unsigned short int));
					#ifndef BFI_NOCHECKS
                    if (mem == NULL) {
                        puts("Out of memory");
                        return EXIT_FAILURE;
                    }
					#endif
                    for (n = 1; n <= 1024; n++)
                        mem[maxmp + n] = 0;
                    maxmp += 1024;
                }
                mp++;
                break;
            case '<':
				#ifndef BFI_NOCHECKS
                if (mp <= 0) {
                    printf("Access Violation, ip=%d", ip);
                    return EXIT_FAILURE;
                }
				#endif
                mp--;
                break;
            case '+':
                mem[mp]++;
                break;
            case '-':
                mem[mp]--;
                break;
            case '.':
                putchar(mem[mp]);
                break;
            case ',':
                n = getchar();
                if (n < 0)
                    n = 0;
                mem[mp] = n;
                break;
            case '[':
                if (src[ip + 1] == '-' && src[ip + 2] == ']') {
                    mem[mp] = 0;
                    ip += 2;
                } else if (mem[mp] == 0)
                    match(1);
                break;
            case ']':
                match(-1);
                ip--;
                break;
			case '*':
				for(ic = 0; ic < 256; ic += 4)
					fprintf(stderr, "%4X %4X %4X %4X\n", mem[ic], mem[ic + 1], mem[ic + 2], mem[ic + 3]);
        }
    }
    return EXIT_SUCCESS;
}
