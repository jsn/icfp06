#define G_DISABLE_ASSERT 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#define C   r[ins & 7]
#define B   r[(ins >> 3) & 7]
#define A   r[(ins >> 6) & 7]
#define A2  r[(ins >> 25) & 7]

int main(int ac, const char *av[]) {
    pint r[8] = {0};
    pint pc = 0 ;

    if (ac != 2) DIE("usage: %s <program.um>", *av) ;

    load_file(av[1]) ;

    for (;;) {
        g_assert(pc < zero->ncells) ;
        pint ins = zero->cells[pc] ;

        switch ((ins >> 28) & 15) {
            case  0: if (C) A = B ; break ;
            case  1: A = array_get(B, C) ; break ;
            case  2: array_set(A, B, C) ; break ;
            case  3: A = (uint32_t)(B + C) ; break ;
            case  4: A = (uint32_t)(B * C) ; break ;
            case  5: A = ((uint32_t)B) / ((uint32_t)C) ; break ;
            case  6: A = (uint32_t)~(B & C) ; break ;
            case  7: g_message("HALT") ; exit(0) ; break ;
            case  8: B = array_alloc(C) ; break ;
            case  9: array_free(C) ; break ;
            case 10: fputc((char)C, stdout) ; break ;
            case 11: C = vm_fgetc() ; break ;
            case 12: {
                         if (B) {
                            array_free((pint)zero) ;
                            struct array *b = (struct array *)B ;
                            zero = (struct array *)array_alloc(b->ncells) ;
                            memcpy(zero->cells, b->cells,
                                    b->ncells * sizeof(pint)) ;
                         }
                         pc = C ;
                         continue ;
                     }
            case 13: A2 = ins & 0x1ffffff ; break ;
            default:
                     DIE("unknown instruction %lx", ins) ;
        }
        pc ++ ;
    }

    return 0 ;
}
