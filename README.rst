My collection of VMs for ICFP 2006
==================================

A small collection of VM implementations. Currently there are 6 VMs:

  * 2 interpreters written in C,
  * 2 JIT implementations (also in C),
  * 1 (interpreter) written in golang,
  * and 1 (interpreter) written in nim.

See http://www.boundvariable.org/ for more details about the VM.
VM images (``codex.umz`` and ``sandmark.umz``) are available there.

VMs written in C
----------------

The VMs in this section share some code (``common.h`` and ``common.c``), 
specifically, arrays allocation / deallocation, IO functions, etc. When 
comparing the benchmark execution times below, keep in mind that each of 
these programs spends ~9 seconds in calloc / free (according to ``gprof``).  
So, for example, opcode dispatch and execution in ``lightn.c`` is actually 
almost a decimal order of magnitude faster than in ``switch.c`` (to be more 
precise, (32 - 9) / (11.5 - 9) = 9.2 times faster), and about twice as fast 
as in ``dasm-x64.dasm``.

c/switch.c
    The most naive implementation possible. Takes **32 seconds** on my 
    laptop to run ``sandmark.umz``. Opcode dispatch is just a C ``switch``, 
    hence the name. Branch misprediction happens almost on every opcode 
    dispatch.

c/goto.c
    Best I can do with pure C so far (w/o e.g. assembly tricks).  Takes 
    **22.5 seconds** on my laptop to run the ``sandmark``. Opcode dispatch 
    is a bit smarter (C ``goto`` with a branchy ``NEXT`` clause, much more 
    friendly to the CPU branch predictor).  

c/dasm-x64.dasm
    A real JIT, incrementally compiles the VM opcode stream into native 
    code. Handles self-modifying code, etc. x86_64 only (see 
    ``dasm-x86.dasm`` for a 32bit variant, though).  Uses dynasm_.
    Takes **14 seconds** to run the ``sandmark``.  Unfortunately, 
    ``dynasm`` is actually buggy as hell.  Spent most of the day finding 
    those weird bugs in the generated machine code in GDB. If you wonder 
    why the Assembly language code in ``dasm-x64.dasm`` is so damn ugly 
    -- well, it's largely because almost any less ugly code triggers some 
    bugs in ``dynasm``.  At this rate it really isn't worth the trouble.  
    Which is a pity, because I like the idea behind ``dynasm`` a lot.

c/lightn.c
    Another JIT implementation, this time using `GNU Lightning`_. This one 
    takes **~11.5 seconds** to run the ``sandmark``. I used a non-portable 
    x86_64 subset of what ``lightning`` provides (specifically, 10 general 
    purpose registers; portable code should use 6). Try not to do that, 
    though; apparently there's a bug in ``lightning`` clobbering some 
    registers sometimes. Perhaps a portable code wouldn't trigger that bug.  
    Overall, ``lightning`` rules. I will probably prefer it do ``dynasm`` 
    in my future projects.

    The speedup (comparing to ``dynasm`` version) is mostly because with 
    ``lightning`` I was able to map all VM registers to real x86_64 CPU 
    registers. It's probably impossible with ``dynasm``.

Other VMs
---------

go/switch/switch.go
    Takes **46 seconds** to run the ``sandmark``. Basically a translation 
    of ``switch.c``, a very naive implementation. It should be possible to 
    translate ``goto.c`` to Go too, might improve the performance by I 
    don't know, 10%? Quite a tedious task though, since there are no macros 
    in Go.

nim/switch.nim
    Takes **40 seconds** to run the ``sandmark``. Also a translation of
    ``switch.c``. Nim documentation mentions a pragma 
    (``{.computedGoto.}``) that is specifically designed to optimize
    switch VM dispatch wrt branch predictions, and it actually works (but 
    you can't use e.g. ``continue`` in your switch).

.. _dynasm: https://corsix.github.io/dynasm-doc/
.. _GNU Lightning: https://www.gnu.org/software/lightning/manual/lightning.html
