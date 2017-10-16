import os, streams, sequtils, posix

type
  Pint = uint32
  Array = seq[Pint]

proc die(s: string) =
  echo s
  quit -1

var arrays: seq[ref[Array]] = @[]
var free: Array = @[]

proc allocArray(size: int): Pint =
  let x = new Array
  x[] = repeat(Pint 0, size)

  if free.len == 0:
    arrays &= x
    return Pint arrays.high

  let a = free.pop
  assert arrays[int a] == nil
  arrays[int a] = x
  return a

proc freeArray(a: Pint) =
  free &= a
  arrays[int a] = nil

proc loadFile(fname: string): Pint =
  let fs = int getFileSize(fname)
  
  if fs mod sizeof(Pint) != 0:
    die "weird file size: " & $fs

  let a = allocArray(fs div sizeof(Pint))
  if newFileStream(fname).readData(addr arrays[int a][0], fs) != fs:
    die "short read"

  for p in arrays[int a][].mitems: p = ntohl p
  return a

proc main() =
  let argv = commandLineParams()
  if argv.len != 1:
    die "please specify the UM program"

  doAssert loadFile(argv[0]) == 0
  var pc = Pint 0
  var r: array[Pint 8, Pint]
  var zero = arrays[0]

  template C(): untyped = r[ins and 7]
  template B(): untyped = r[(ins shr 3) and 7]
  template A(): untyped = r[(ins shr 6) and 7]
  template A2(): untyped = r[(ins shr 25) and 7]

  while true:
    {.computedGoto.}
    var ins = zero[int pc]

    case (ins shr 28) and 15:
      of 0:
        if C != 0: A = B
      of 1:
        A = arrays[int B][int C]
      of 2:
        arrays[int A][int B] = C
      of 3:
        A = B + C
      of 4:
        A = B * C
      of 5:
        A = B div C
      of 6:
        A = not(B and C)
      of 7:
        echo "HALT"
        return
      of 8:
        B = allocArray(int C)
      of 9:
        freeArray C
      of 10:
        doAssert stdout.writeBytes([uint8 C], 0, 1) == 1
      of 11:
        var b = [uint8 0]
        let read = stdin.readBytes(b, 0, 1)

        C =
          if read == 0:
            not Pint 0
          else:
            b[0]
      of 12:
        if B != 0:
          arrays[0][] = arrays[int B][]
          zero = arrays[0]
        pc = C - 1
      of 13:
        A2 = Pint(ins and 0x1ffffff)
      of 14, 15:
        die "unknown opcode"

    inc pc

main()
