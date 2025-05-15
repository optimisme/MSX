#ifndef BANKS_H
#define BANKS_H

void bank1_content(unsigned char a, unsigned char b, unsigned int *out) __banked;
void bank2_content() __banked;

#endif
