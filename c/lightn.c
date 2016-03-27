#define G_DISABLE_ASSERT 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include <sys/mman.h>
#include <lightning.h>

#define MAXCODE     (1 << 28)

static pint r[8] ;
static pint pc ;
static pint hint ;
static pint regs[8] = {2, 3, 4, 5, 6, 7, 8, 9} ;

#define R1 JIT_R(0)
#define R2 JIT_R(10)

#define C   JIT_R(regs[ins & 7])
#define B   JIT_R(regs[(ins >> 3) & 7])
#define A   JIT_R(regs[(ins >> 6) & 7])
#define X   JIT_R(regs[(ins >> 25) & 7])

#define LOAD_REGS() for (int i = 0; i < 8; i ++) jit_ldi(JIT_R(regs[i]), &r[i])
#define SAVE_REGS() for (int i = 0; i < 8; i ++) jit_sti(&r[i], JIT_R(regs[i]))

intptr_t (*code)(void *p) ;
void **jumps ;
size_t code_used ;
void *um_leave ;

static void init_code(void) {
    if (jumps) g_free(jumps) ;
    jit_state_t *_jit ;
    jit_word_t ncode ;

    jit_node_t *leave ;

    _jit = jit_new_state() ;

    jit_prolog() ;
    jit_getarg(R1, jit_arg()) ;
    LOAD_REGS() ;
    jit_jmpr(R1) ;
    leave = jit_note(__FILE__, __LINE__) ;
    SAVE_REGS() ;
    jit_retr(R1) ;

    code_used = 0 ;

    jumps = g_malloc0(zero->ncells * sizeof(*jumps)) ;

    jit_realize() ;
    jit_set_data(NULL, 0, JIT_DISABLE_DATA | JIT_DISABLE_NOTE);
    jit_set_code((char *)code + code_used, MAXCODE - code_used) ;

    if (jit_emit() == NULL) DIE("jit_emit()") ;
    jit_get_code(&ncode) ;

    code_used += ncode ;

    um_leave = jit_address(leave) ;
    // jit_disassemble() ;

    jit_clear_state() ;
    jit_destroy_state() ;
    g_message("reloaded") ;
}

/* on return: R1 contains the address of the target cell */
static void inline array_ref(jit_state_t *_jit, int ary, int idx) {
    jit_node_t *t ;

    jit_movr(R2, ary) ;
    t = jit_bnei(R2, 0) ;
    jit_movi(R2, (pint)zero) ;
    jit_patch(t) ;
#ifndef G_DISABLE_ASSERT
    jit_ldr(R1, R2) ;
    t = jit_beqi(R1, 0xdeadfall) ;
    jit_sti(NULL, R1) ;
    jit_patch(t) ;
    jit_addi(R2, R2, 8) ;
    jit_ldr(R1, R2) ;
    t = jit_bltr_u(idx, R1) ;
    jit_sti(NULL, R1) ;
    jit_patch(t) ;
    jit_addi(R2, R2, 8) ;
#else
    jit_addi(R2, R2, 16) ;
#endif
    jit_lshi(R1, idx, 3) ;
    jit_addr(R1, R1, R2) ;
}

static void compile(void) {
    jit_state_t *_jit ;
    jit_word_t ncode = 0 ;
    jit_node_t *t, *t2 ;
    size_t i ;
    size_t n_ops = 0 ;
    int done = 0 ;

    _jit = jit_new_state() ;

    jit_prolog() ;

    for (i = pc; !done; i ++) {
        g_assert(i < zero->ncells) ;
        uint32_t ins = zero->cells[i] ;

        jumps[pc + n_ops] = jit_note("jit", pc + n_ops) ;

        switch (ins >> 28) {
            case  0:
                t = jit_beqi(C, 0) ;
                jit_movr(A, B) ;
                jit_patch(t) ;
                break ;
            case  1:
                array_ref(_jit, B, C) ;
                jit_ldr(A, R1) ;
                break ;
            case  2:
                array_ref(_jit, A, B) ;
                jit_str(R1, C) ;

                t = jit_bnei(A, 0) ;
                jit_lshi(R1, B, 3) ;
                jit_addi(R1, R1, (pint)jumps) ;
                jit_ldr(R1, R1) ;
                t2 = jit_beqi(R1, 0) ;
                jit_sti(&hint, B) ;
                jit_movi(R1, i + 1) ;
                jit_sti(&pc, R1) ;
                jit_movi(R1, 2) ;
                jit_patch_abs(jit_jmpi(), um_leave) ;
                jit_patch(t2) ;
                jit_patch(t) ;
                break ;
            case  3:
                jit_addr(A, B, C) ;
                jit_extr_ui(A, A) ;
                break ;
            case  4:
                jit_mulr(A, B, C) ;
                jit_extr_ui(A, A) ;
                break ;
            case  5:
                jit_divr_u(A, B, C) ;
                jit_extr_ui(A, A) ;
                break ;
            case  6:
                jit_andr(A, B, C) ;
                jit_comr(A, A) ;
                jit_extr_ui(A, A) ;
                break ;
            case  7:
                jit_movi(R1, i) ;
                jit_sti(&pc, R1) ;
                jit_movi(R1, 0) ;
                jit_patch_abs(jit_jmpi(), um_leave) ;
                done = 1 ;
                break ;
            case  8:
                SAVE_REGS() ;
                jit_prepare() ;
                jit_pushargr(C) ;
                jit_finishi(array_alloc) ;
                LOAD_REGS() ;
                jit_retval(B) ;
                break ;
            case  9:
                SAVE_REGS() ;
                jit_prepare() ;
                jit_pushargr(C) ;
                jit_finishi(array_free) ;
                LOAD_REGS() ;
                break ;
            case 10:
                SAVE_REGS() ;
                jit_extr_uc(C, C) ;
                jit_prepare() ;
                jit_pushargr(C) ;
                jit_pushargi((pint)stdout) ;
                jit_finishi(fputc) ;
                LOAD_REGS() ;
                break ;
            case 11:
                SAVE_REGS() ;
                jit_prepare() ;
                jit_finishi(vm_fgetc) ;
                LOAD_REGS() ;
                jit_retval(C) ;
                break ;
            case 12:
                t = jit_bnei(B, 0) ;
                jit_lshi(R1, C, 3) ;
                jit_addi(R1, R1, (pint)jumps) ;
                jit_ldr(R1, R1) ;
                t2 = jit_beqi(R1, 0) ;
                jit_jmpr(R1) ;
                jit_patch(t) ;
                jit_patch(t2) ;
                jit_sti(&hint, B) ;
                jit_sti(&pc, C) ;

                jit_movi(R1, 1) ;
                jit_patch_abs(jit_jmpi(), um_leave) ;
                done = 1 ;
                break ;
            case 13:
                jit_movi(X, ins & 0x1ffffff) ;
                break ;
            default:
                DIE("unknown instruction %x", ins) ;
        }

        n_ops ++ ;
    }

    // mov esi, 0xf004
    // int3

    jit_retr(R1) ;
    jit_realize() ;
    jit_set_data(NULL, 0, JIT_DISABLE_DATA | JIT_DISABLE_NOTE);
    jit_set_code((char *)code + code_used, MAXCODE - code_used) ;

    if (jit_emit() == NULL) DIE("jit_emit()") ;
    jit_get_code(&ncode) ;

    if (ncode + code_used > MAXCODE) DIE("out of MAXCODE") ;

    for (i = 0; i < n_ops; i ++) {
        jumps[pc + i] = jit_address(jumps[pc + i]) ;
        // printf("%d %p\n", pc + i, jumps[pc + i]) ;
    }

    // g_message("compiled %u ops, %u bytes", n_ops, ncode) ;

    code_used += ncode ;

    // jit_disassemble() ;

    jit_clear_state() ;
    jit_destroy_state() ;
}

int main(int ac, const char *av[]) {
    if (ac != 2) DIE("usage: %s <program.um>", *av) ;

    load_file(av[1]) ;

    code = mmap(0, MAXCODE, PROT_EXEC | PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0) ;

    if (code == MAP_FAILED) DIE("map failed") ;

    init_jit(av[0]) ;
    init_code() ;

    for (;;) {
        unsigned rv ;
        g_assert(!jumps[pc]) ;
        compile() ;
        g_assert(jumps[pc]) ;
        rv = (*code)(jumps[pc]) ;

        switch (rv) {
            case 0:
                finish_jit() ;
                g_message("HALT") ;
                exit(0) ;
            case 1:
                if (hint) {
                    array_free((pint)zero) ;
                    struct array *b = (struct array *)hint ;
                    zero = (struct array *)array_alloc(b->ncells) ;
                    memcpy(zero->cells, b->cells, b->ncells * sizeof(pint));
                    init_code() ;
                }
                break ;
            case 2:
                g_message("self-modifying code detected") ;
                init_code() ;
                break ;
            default:
                DIE("unknown return code %u", rv) ;
        }
    }
    return 0 ;
}
