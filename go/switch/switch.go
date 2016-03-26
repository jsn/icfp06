package main

import (
    "fmt"
    "flag"
    "log"
    "os"
    "encoding/binary"
    "bufio"
)

type pint uint32
type array []pint

var arrays = make([]array, 0)
var free = make([]pint, 0)

func dieOn(err error) {
    if err != nil {
        log.Fatal(err)
    }
}

var stdin = bufio.NewReader(os.Stdin)

func a_alloc(size pint) (v pint) {
    a := make(array, size)

    if l := len(free); l > 0 {
        v = free[l - 1]
        free = free[:l - 1]
        arrays[v] = a
    } else {
        v = pint(len(arrays))
        arrays = append(arrays, a)
    }
    return
}

func getc() pint {
    r, _, err := stdin.ReadRune()
    dieOn(err)
    return pint(r)
}

func putc(c pint) {
    _, err := os.Stdout.Write([]byte{byte(c)})
    dieOn(err)
}

func init() {
    flag.Parse()
    args := flag.Args()

    if len(args) == 0 {
        log.Fatal("no args!")
    }

    file, err := os.Open(flag.Arg(0))
    dieOn(err)
    defer file.Close()

    st, err := file.Stat()
    dieOn(err)

    size := st.Size()

    if size % 4 != 0 {
        log.Fatal("bad file size: ", size)
    }

    arrays = append(arrays, make(array, size / 4))
    dieOn(binary.Read(file, binary.BigEndian, &arrays[0]))
}

func main() {
    fmt.Println(len(arrays[0]), "platters loaded")

    var r [8]pint
    var pc pint

    for {
        ins := arrays[0][pc]
        c, b, a := &r[ins & 7], &r[(ins >> 3) & 7], &r[(ins >> 6) & 7]

        switch (ins >> 28) & 15 {
        case 0:
            if *c != 0 { *a = *b }
        case 1:
            *a = arrays[*b][*c]
        case 2:
            arrays[*a][*b] = *c
        case 3:
            *a = *b + *c
        case 4:
            *a = *b * *c
        case 5:
            *a = *b / *c
        case 6:
            *a = ^(*b & *c)
        case 7:
            fmt.Println("*** HALT ***")
            return
        case 8:
            *b = a_alloc(*c)
        case 9:
            free = append(free, *c)
            arrays[*c] = nil
        case 10:
            putc(*c)
        case 11:
            *c = getc()
        case 12:
            if *b != 0 {
                ba := arrays[*b]
                arrays[0] = make(array, len(ba))
                copy(arrays[0], ba)
            }
            pc = *c
            continue
        case 13:
            r[(ins >> 25) & 7] = ins & 0x1ffffff
        default:
            log.Fatal("unknown instruction", ins)
        }
        pc ++
    }
}
