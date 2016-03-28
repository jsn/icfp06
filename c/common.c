#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct array *zero ;

static void *freelist = NULL ;

pint array_alloc(pint ncells) {
    struct array *a ;
    if (ncells == 3 && freelist) {
        a = (void *)(((char *)freelist) - 8) ;
        freelist = (void *)a->cells[0] ;
        memset(a->cells, 0, 3 * sizeof(pint)) ;
    } else {
        a = g_malloc0(sizeof(*a) + sizeof(pint) * ncells) ;
    }
    a->magic = 0xdeadfall ;
    a->ncells = ncells ;
    return (pint)a ;
}

void array_free(pint i) {
    g_assert(i) ;
    struct array *a = (struct array *)i ;
    if (a->ncells == 3) {
        a->cells[0] = (pint)freelist ;
        freelist = ((char *)a) + 8 ;
        return ;
    }
    g_assert(a->magic == 0xdeadfall) ;
    a->magic = 0 ;
    g_free(a) ;
}

static FILE *in_f, *out_f ;
static char in_buff[1024] ;
static size_t in_curr ;

pint vm_fgetc(void) {
    while (!in_buff[in_curr]) {
        in_curr = 0 ;

        FILE *fp = in_f ? in_f : stdin ;
        if (fgets(in_buff, sizeof(in_buff), fp) == NULL) {
            if (in_f) {
                in_f = NULL ;
                in_buff[0] = in_curr = 0 ;
                continue ;
            }
            return (pint)-1 ;
        }

        if (in_f) fputs(in_buff, stdout) ;

        if (in_buff[0] == '~') {
            if (in_buff[++ in_curr] == '~') break ;

            switch (in_buff[in_curr]) {
                case '>':
                    {
                        int trunc = 1 ;

                        if (in_buff[in_curr + 1] == '>') {
                            in_curr ++ ;
                            trunc = 0 ;
                        }

                        char *fname = g_strstrip(in_buff + in_curr + 1) ;

                        if (out_f) fclose(out_f) ;

                        if (fname[0]) {
                            out_f = fopen(fname, trunc ? "w" : "a") ;
                            if (!out_f)
                                g_message("fopen(`%s') failed", fname) ;
                        } else
                            out_f = NULL ;
                        break ;
                    }
                case '<':
                    {
                        char *fname = g_strstrip(in_buff + in_curr + 1) ;

                        if (!fname[0]) {
                            g_message("ignoring request for empty file name") ;
                            break ;
                        }

                        if (in_f) fclose(in_f) ;

                        in_f = fopen(fname, "r") ;

                        if (!in_f) g_message("fopen(`%s') failed", fname) ;
                        break ;
                    }
                default:
                    g_message("unknown command: %s", in_buff + 1) ;
            }

            in_buff[0] = in_curr = 0 ;
        } else { /* not a ~ */
            if (out_f) fputs(in_buff, out_f) ;
        }
    }
    return (pint)in_buff[in_curr ++] ;
}

void load_file(const char *fname) {
    gsize fsize, i ;
    uint32_t *fcontent ;
    GError *err = NULL ;

    if (!g_file_get_contents(fname, (gchar **)&fcontent, &fsize, &err))
        DIE("reading `%s': %s", fname, err->message) ;

    if (fsize % 4) DIE("weird program size: %lu bytes", (pint)fsize) ;
    fsize /= 4 ;

    zero = (struct array *)array_alloc(fsize) ;

    for (i = 0; i < fsize; i ++) zero->cells[i] = g_ntohl(fcontent[i]) ;
    g_free(fcontent) ;

    g_message("%lu dwords loaded", (pint)fsize) ;
}
