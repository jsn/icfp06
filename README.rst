My collection of VMs for ICFP 2006
==================================

A small collection of VM implementations. Currently there are 5 VMs:
  * 2 interpreters written in C,
  * 2 JIT implementations (also in C),
  * and 1 (interpreter) written in golang.

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

c/dasm-x64.dasm
    A real JIT, incrementally compiles the VM opcode stream into native 
    code. Handles self-modifying code, etc. x86_64 only (see 
    ``dasm-x86.dasm`` for a 32bit variant, though).  Uses dynasm_.
    Takes **16 seconds** to run the ``sandmark``.  Unfortunately, 
    ``dynasm`` is actually buggy as hell.  Spent most of the day finding 
    those weird bugs in the generated machine code in GDB. If you wonder 
    why the Assembly language code in ``dasm-x64.dasm`` is so damn ugly 
    -- well, it's largely because almost any less ugly code triggers some 
    bugs in ``dynasm``.  At this rate it really isn't worth the trouble.  
    Which is a pity, because I like the idea behind ``dynasm`` a lot.

c/lightn.c
    Another JIT implementation, this time using `GNU Lightning`_. This one 
    takes **~12.5 seconds** to run the ``sandmark``. I used a non-portable 
    x86_64 subset of what ``lightning`` provides (specifically, 10 general 
    purpose registers; portable code should use 6). Try not to do that, 
    though; apparently there's a bug in ``lightning`` clobbering some 
    registers sometimes. Perhaps a portable code wouldn't trigger that bug.  
    Overall, ``lightning`` rules. I will probably prefer it do ``dynasm`` 
    in my future projects.

    The speedup (comparing to ``dynasm`` version) is mostly because with 
    ``lightning`` I was able to map all VM registers to real x86_64 CPU 
    registers. It's probably impossible with ``dynasm``.

go/switch/switch.go
    Takes **48 seconds** to run the ``sandmark``. Basically a translation 
    of ``switch.c``, a very naive implementation. It should be possible to 
    translate ``goto.c`` to Go too, might improve the performance by I 
    don't know, 10%? Quite a tedious task though, since there are no macros 
    in Go.

.. _dynasm: https://corsix.github.io/dynasm-doc/
.. _GNU Lightning: https://www.gnu.org/software/lightning/manual/lightning.html
