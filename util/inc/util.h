#ifndef __UTIL_FILE_H
#define __UTIL_FILE_H

#define MAX_INPUT_LEN 1024
#define SET_BIT(x, bit)    (x) |= 1 << (bit)
#define CLEAR_BIT(x, bit)  (x) &= ~(1 << (bit))
#define IS_BIT_SET(x, bit) (x) & (1 << (bit))

/* Terminating character */
#define CMD_TERMINATE_CHAR 0x03

extern void remove_trailing_chars(char *);
extern void terminate_command(char *);
#endif /* __UTIL_FILE_H */
