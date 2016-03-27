My collection of VMs for ICFP 2006
==================================

A small collection of VM implementations. Currently there are 4 VMs,
3 written in C and 1 written in golang.

See http://www.boundvariable.org/ for more details about the VM.
VM images (``codex.umz`` and ``sandmark.umz``) are available there.

c/switch.c
    The most naive implementation possible. Takes **32 seconds** on my 
    laptop to run ``sandmark.umz``. Opcode dispatch is just a C ``switch``, 
    hence the name. Branch misprediction happens almost on every opcode 
    dispatch.

c/goto.c
    Best I can do with pure C so far (w/o e.g. assembly tricks).  Takes 
    **25 seconds** on my laptop to run the ``sandmark``. Opcode dispatch is 
    a bit smarter (C ``goto`` with a branchy ``NEXT`` clause, much more 
    friendly to the CPU branch predictor).  

c/jit-dynasm.dasm
    A real JIT, incrementally compiles the VM opcode stream into native 
    code. Handles self-modifying code, etc. x86_64 only (see 
    ``jit-dynasm-x86.dasm`` for a 32bit variant, though).  Uses dynasm_.
    Takes **21 seconds** to run the ``sandmark``.  Unfortunately, 
    ``dynasm`` is actually buggy as hell.  Spent most of the day finding 
    those weird bugs in the generated machine code in GDB. If you wonder 
    why the Assembly language code in ``jit-dynasm.dasm`` is so damn ugly 
    -- well, it's largely because almost any less ugly code triggers some 
    bugs in ``dynasm``.  At this rate it really isn't worth the trouble.  
    Which is a pity, because I like the idea behind ``dynasm`` a lot.

go/switch/switch.go
    Takes **48 seconds** to run the ``sandmark``. Basically a translation 
    of ``switch.c``, a very naive implementation. It should be possible to 
    translate ``goto.c`` to Go too, might improve the performance by I 
    don't know, 10%? Quite a tedious task though, since there are no macros 
    in Go.

.. _dynasm: https://corsix.github.io/dynasm-doc/
