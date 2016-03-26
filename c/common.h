#ifndef __VM_H__
#define __VM_H__

#include <stdint.h>
#include <glib.h>

#define DIE(...) { g_critical(__VA_ARGS__) ; exit(-1) ; }

typedef uintptr_t pint ;

struct array {
    pint magic ;
    pint ncells ;
    pint cells[0] ;
} ;

extern struct array *zero ;

#define ARRAY(i)    ((i) ? ((struct array *)(i)) : zero)

static inline pint array_get(pint ai, pint i) {
    struct array *a = ARRAY(ai) ;
    g_assert(a->magic == 0xdeadfall) ;
    g_assert(i < a->ncells) ;
    return a->cells[i] ;
}

static inline void array_set(pint ai, pint i, pint v) {
    struct array *a = ARRAY(ai) ;
    g_assert(a->magic == 0xdeadfall) ;
    g_assert(i < a->ncells) ;
    a->cells[i] = v ;
}

pint array_alloc(pint ncells) ;
void array_free(pint i) ;
pint vm_fgetc(void) ;

void load_file(const char *fname) ;

#endif
