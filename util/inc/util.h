#ifndef __UTIL_FILE_H
#define __UTIL_FILE_H

#define SET_BIT(x, bit)    (x) |= 1 << (bit)
#define CLEAR_BIT(x, bit)  (x) &= ~(1 << (bit))
#define IS_BIT_SET(x, bit) (x) & (1 << (bit))

extern void remove_trailing_chars(char *);
#endif /* __UTIL_FILE_H */