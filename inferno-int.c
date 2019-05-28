
/* asm2bf / inferno
 *
 * Copyright (C) Kamila Palaiologos Szewczyk, 2019.
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
        if (ip < 0 || ip >= lof) {
            puts("Mismatched brackets");
            exit(EXIT_FAILURE);
        }
        ipvalue = src[ip];
        if (ipvalue == 91)
            level += dir;
        else if (ipvalue == 93)
            level -= dir;
    } while (level > 0);
}

void syscall(unsigned short int * mem, long mp) {
    /* Basic asm2bf register */
    unsigned short int * r1 = &mem[5];
    unsigned short int * r2 = &mem[6];
    unsigned short int * r3 = &mem[7];
    unsigned short int * r4 = &mem[8];
    
    /* For compatibility with bfasm-experimental */
    unsigned short int * r5 = &mem[9];
    
    static unsigned short int offset = 0;
    
    switch(*r1) {
        case 0:
            offset = *r2;
        case 1:
            exit(*r2);
            break;
        case 2:
            *r1 = fork();
            break;
        case 3:
            *r1 = read(*r2, mem + *r3 + offset, *r4);
            break;
        case 4:
            *r1 = write(*r2, mem + *r3 + offset, *r4);
            break;
        case 5:
            *r1 = open(mem + *r2 + offset, *r3, *r4);
            break;
        case 6:
            close(*r2);
            break;
        case 7:
            *r1 = creat(mem + *r2 + offset, *r3);
            break;
        case 8:
            *r1 = link(mem + *r2 + offset, mem + *r3 + offset);
            break;
        case 9:
            *r1 = unlink(mem + *r2 + offset);
            break;
        case 10:
            *r1 = chdir(mem + *r2 + offset);
            break;
        case 11:
            *r1 = time(mem + *r2 + offset);
            break;
        case 12:
            *r1 = mknod(mem + *r2 + offset, *r3, *r4);
            break;
        case 13:
            *r1 = chmod(mem + *r2 + offset, *r3);
            break;
        case 14:
            *r1 = lseek(*r2, *r3, *r4);
            break;
        case 15:
            *r1 = getpid();
            break;
        case 16:
            *r1 = stime(mem + *r2 + offset);
            break;
        case 17:
            *r1 = pause();
            break;
        case 18:
            *r1 = ioctl(*r2, *r3);
            break;
        case 19:
            *r1 = rename(mem + *r2 + offset, mem + *r3 + offset);
            break;
        case 20:
            *r1 = clock();
            break;
        case 21:
            *r1 = time(NULL);
            break;
        case 22:
            *r1 = rmdir(mem + *r2 + offset);
            break;
    }
}

int main(int argc, char * argv[]) {
    FILE * infile;
    long mp = 0, maxmp = 1023;
    int n;
    unsigned short int * mem;
    if (argc != 2 || argv[1][0] == '-' || argv[1][0] == '/') {
        puts("Usage: bfi src.b");
        return EXIT_FAILURE;
    }
    infile = fopen(argv[1], "rb");
    if (infile == NULL) {
        puts("Error opening input file");
        return EXIT_FAILURE;
    }
    if (fseek(infile, 0L, SEEK_END) != 0) {
        puts("Error determining length of input file");
        return EXIT_FAILURE;
    }
    lof = ftell(infile);
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
    src = (char *) calloc(lof + 2, sizeof(char));
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
    mem = calloc(1024, sizeof(unsigned short int));
    if (mem == NULL) {
        puts("Out of memory");
        return EXIT_FAILURE;
    }
    while (++ip < lof) {
        switch (src[ip]) {
            case '>':
                if (mp >= maxmp) {
                    mem = realloc(mem, (maxmp + 1024) * sizeof(unsigned short int));
                    if (mem == NULL) {
                        puts("Out of memory");
                        return EXIT_FAILURE;
                    }
                    for (n = 1; n <= 1024; n++)
                        mem[maxmp + n] = 0;
                    maxmp += 1024;
                }
                mp++;
                break;
            case '<':
                if (mp <= 0) {
                    printf("Access Violation, ip=%d", ip);
                    return EXIT_FAILURE;
                }
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
            case '$':
                syscall(mem, mp);
                break;
        }
    }
    return EXIT_SUCCESS;
}
