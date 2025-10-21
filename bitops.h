#ifndef BITOPS_H
#define BITOPS_H


unsigned long long CreateMask(int position);
unsigned long long ModifyBit(unsigned long long value, int position, int operation);
int GetBit(unsigned long long value, int position);
int CountBits(unsigned long long value);
unsigned long long ShiftLeft(unsigned long long value, int positions);
unsigned long long ShiftRight(unsigned long long value, int positions);
void PrintBinary(unsigned long long value);
void PrintHex(unsigned long long value);

#endif