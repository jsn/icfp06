#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define C   r[ins & 7]
#define B   r[(ins >> 3) & 7]
#define A   r[(ins >> 6) & 7]
#define X   r[(ins >> 25) & 7]

#define OP(x) case x: goto op ## x ;

#define NEXT(expr)      \
    pc expr ; \
    g_assert(pc < zero->ncells) ; \
    ins = zero->cells[pc] ; \
    switch ((ins >> 28) & 15) { \
        OP(0) OP(1) OP(2) OP(3) OP(4) OP(5) OP(6) OP(7) OP(8) OP(9) OP(10) \
        OP(11) OP(12) OP(13) \
        default: DIE("bad instruction %lx", ins) ; \
    }

int main(int ac, const char *av[]) {
    pint r[8] = {0} ;
    static pint pc = 0 ;

    if (ac != 2) DIE("usage: %s <program.um>", *av) ;

    load_file(av[1]) ;

    pint ins ;
    NEXT(= 0) ;

    op0: if (C) A = B ; NEXT(++) ;
    op1: A = array_get(B, C) ; NEXT(++) ;
    op2: array_set(A, B, C) ; NEXT(++) ;
    op3: A = (uint32_t)(B + C) ; NEXT(++) ;
    op4: A = (uint32_t)(B * C) ; NEXT(++) ;
    op5: A = (uint32_t)B / (uint32_t)C ; NEXT(++) ;
    op6: A = (uint32_t)~(B & C) ; NEXT(++) ;
    op7: g_message("HALT") ; exit(0) ; NEXT(++) ;
    op8: B = array_alloc(C) ; NEXT(++) ;
    op9: array_free(C) ; NEXT(++) ;
    op10: fputc((char)C, stdout) ; NEXT(++) ;
    op11: C = vm_fgetc() ; NEXT(++) ;
    op12:
          if (B) {
              array_free((pint)zero) ;
              struct array *b = (struct array *)B ;
              zero = (struct array *)array_alloc(b->ncells) ;
              memcpy(zero->cells, b->cells, b->ncells * sizeof(pint)) ;
          }
          NEXT(= C) ;
    op13: X = ins & 0x1ffffff ; NEXT(++) ;

    return 0 ;
}
