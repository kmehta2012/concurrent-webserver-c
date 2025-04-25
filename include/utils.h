#ifndef UTILS_H
#define UTILS_H

#define MAX(x,y) x > y ? x : y
#define MIN(x,y) x < y ? x : y

/**
 * Macro arguments are macro-expanded before being passed to the macro body.
 * However, if an argument is used with the # stringification operator inside the macro, 
 * the argument is not expanded before it gets turned into a string.
 */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#endif 