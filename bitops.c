#include "bitops.h"
#include <stdio.h>

unsigned long long CreateMask(int position) {
    return 1ULL << position;
}

unsigned long long ModifyBit(unsigned long long value, int position, int operation) {
    unsigned long long mask = 1ULL << position;

    switch (operation) {
        case 0: 
            return value & ~mask;
        case 1: 
            return value | mask;
        case 2: 
            return value ^ mask;
        default: 
            return value;
    }
}

int GetBit(unsigned long long value, int position) {
    return (value >> position) & 1ULL;
}

int CountBits(unsigned long long value) {
    int count = 0;
    while (value > 0) {
        value &= (value - 1);
        count++;
    }
    return count;
}

unsigned long long ShiftLeft(unsigned long long value, int positions) {
    return value << positions;
}

unsigned long long ShiftRight(unsigned long long value, int positions) {
    return value >> positions;
}

void PrintBinary(unsigned long long value) {
    for (int i = 63; i >= 0; i--) {
        printf("%d", (value >> i) & 1ULL);
        if (i % 8 == 0 && i > 0) {
            printf(" ");
        }
    }
    printf("\n");
}

void PrintHex(unsigned long long value) {
    printf("0x%llX\n", value);
}
