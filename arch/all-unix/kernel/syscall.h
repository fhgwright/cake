#ifndef SYSCALL_H_
#define SYSCALL_H_

typedef enum {
    sc_CAUSE,
    sc_DISPATCH,
    sc_SWITCH,
    sc_SCHEDULE
} syscall_type_t;

#endif /*SYSCALL_H_*/
