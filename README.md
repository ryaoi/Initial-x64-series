# Initial-x64-series
optimisation for assembly

my pc spec: 2,2 GHz Intel Core i7

amazing book to read : https://www.agner.org/optimize/optimizing_assembly.pdf

## Processor does some efficient stuff (Out of order execution)

All modern x86 processors can execute instructions out of order. Consider this example:

```
; Example 9.1a, Out-of-order execution
mov eax, [mem1]
imul eax, 6
mov [mem2], eax
mov ebx, [mem3]
add ebx, 2
mov [mem4], ebx
```

This piece of code is doing two things that have nothing to do with each other: multiplying
[mem1] by 6 and adding 2 to [mem3]. If it happens that [mem1] is not in the cache then the
CPU has to wait many clock cycles while this operand is being fetched from main memory.
The CPU will look for something else to do in the meantime. It cannot do the second
instruction imul eax,6 because it depends on the output of the first instruction. But the
fourth instruction mov ebx,[mem3] is independent of the preceding instructions so it is
possible to execute mov ebx,[mem3] and add ebx,2 while it is waiting for [mem1]. The
CPUs have many features to support efficient out-of-order execution. Most important is, of
course, the ability to detect whether an instruction depends on the output of a previous
instruction. Another important feature is register renaming. Assume that the we are using
the same register for multiplying and adding in example 9.1a because there are no more
spare registers:

```
; Example 9.1b, Out-of-order execution with register renaming
mov eax, [mem1]
imul eax, 6
mov [mem2], eax
mov eax, [mem3]
add eax, 2
mov [mem4], eax
```

Example 9.1b will work exactly as fast as example 9.1a because the CPU is able to use
different physical registers for the same logical register eax. This works in a very elegant
way. The CPU assigns a new physical register to hold the value of eax every time eax is
written to. This means that the above code is changed inside the CPU to a code that uses
four different physical registers for eax. The first register is used for the value loaded from
[mem1]. The second register is used for the output of the imul instruction. The third register
is used for the value loaded from [mem3]. And the fourth register is used for the output of
the add instruction. The use of different physical registers for the same logical register
enables the CPU to make the last three instructions in example 9.1b independent of the first
three instructions. The CPU must have a lot of physical registers for this mechanism to work
efficiently. The number of physical registers is different for different microprocessors, but
you can generally assume that the number is sufficient for quite a lot of instruction
reordering.

stuff that optimize execution with cpu:

 - Partial registers
 - Micro-operations
 - Execution units
 - Pipelined instructions
 
### Summary

The most important things you have to be aware of in order to take maximum advantage of
out-or-order execution are:
- At least the following registers can be renamed: all general purpose registers, the
stack pointer, the flags register, floating point registers, MMX, XMM and YMM
registers. Some CPUs can also rename segment registers and the floating point
control word.
- Prevent false dependences by writing to a full register rather than a partial register. 
- The INC and DEC instructions are inefficient on some CPUs because they write to
only part of the flags register (excluding the carry flag). Use ADD or SUB instead to
avoid false dependences or inefficient splitting of the flags register.
- A chain of instructions where each instruction depends on the previous one cannot
execute out of order. Avoid long dependency chains. (See page 65).
- Memory operands cannot be renamed.
- A memory read can execute before a preceding memory write to a different address.
Any pointer or index registers should be calculated as early as possible so that the
CPU can verify that the addresses of memory operands are different.
- A memory write cannot execute before a preceding write, but the write buffers can
hold a number of pending writes, typically four or more.
- A memory read can execute before or simultaneously with another preceding read
on most processors.
- The CPU can do more things simultaneously if the code contains a good mixture of
instructions from different categories, such as: simple integer instructions, floating
point addition, multiplication, memory read, memory write.


(page 62 from optimizing_assembly.pdf)

### Break dependency chains

In order to take advantage of out-of-order execution, you have to avoid long dependency
chains.

For :
```
	double list[SIZE], sum = 0.;
	for (long i = 0; i < SIZE; i++) sum += list[i];
 ```
 It took:
 ```
 ┌─[ryaoi@e1r6p14] - [~] - [Mon Oct 29, 18:03]
└─[$] <> time ./a.out
./a.out  0.00s user 0.00s system 79% cpu 0.011 total
```

For :
```
	double list[SIZE], sum1 = 0., sum2 = 0., sum3 = 0., sum4 = 0.;
	for (int i = 0; i < SIZE; i += 4) {
		sum1 += list[i];
		sum2 += list[i+1];
		sum3 += list[i+2];
		sum4 += list[i+3];
	}
	sum1 = (sum1 + sum2) + (sum3 + sum4);
 ```
 
 It took:
 
 ```
 ┌─[ryaoi@e1r6p14] - [~] - [Mon Oct 29, 18:02]
└─[$] <> time ./a.out
./a.out  0.00s user 0.00s system 60% cpu 0.009 total
```

### Replacing conditional jumps with conditional moves
The most important jumps to eliminate are conditional jumps, especially if they are poorly
predicted. The advantage of a conditional move is that it avoids branch mispredictions. But it has the
disadvantage that it increases the length of a dependency chain, while a predicted branch
breaks the dependency chain.

__C code__
```
a = b > c ? d : e;
```

__Branch implemented with conditional jump__
```
 ; Branch implemented with conditional jump
 mov eax, [b]
 cmp eax, [c]
 jng L1
 mov eax, [d]
 jmp L2
L1: mov eax, [e]
L2: mov [a], eax
```

__Branch implemented with conditional move__
```
 ; Branch implemented with conditional move
 mov eax, [b]
 cmp eax, [c]
 mov eax, [d]
 cmovng eax, [e]
 mov [a], eax
 ```

### Replacing conditional jumps with conditional set instructions
An implementation with conditional jumps may be faster than conditional set if the prediction
rate is good and the code is part of a long dependency chain.

__C code__
```
// Set a bool variable on some condition
int b, c;
bool a = b > c;
```

__Implementation with conditional set, only 8bit__
```
mov eax, [b]
cmp eax, [c]
setg al
mov [a], al
```

__Implementation with conditional set, 32 bits__
```
;  Implementation with conditional set, 32 bits
mov eax, [b]
cmp eax, [c]
setg al
movzx eax, al
mov [a], eax
```

### Replacing conditional jumps with bit-manipulation instructions

__Calculate absolute value__
```
; Calculate absolute value of eax
cdq ; Copy sign bit of eax to all bits of edx
xor eax, edx ; Invert all bits if negative
sub eax, edx ; Add 1 if negative
```

__finds the minimum of two unsigned numbers: if (b > a) b = a__
```
sub eax, ebx ; = a-b
sbb edx, edx ; = (b > a) ? 0xFFFFFFFF : 0
and edx, eax ; = (b > a) ? a-b : 0
add ebx, edx ; Result is in ebx
```

__chooses between two numbers: if (a < 0) d = b; else d = c;__
```
test eax, eax          ; check if a < 0
mov edx, ecx           ; edx = c
cmovs edx, ebx ; = (a < 0) ? b : c
```

Conditional moves are not very efficient on Intel processors and not available on old
processors. Alternative implementations may be faster in some cases.
__chooses between two numbers: if (a < 0) d = b; else d = c; without cmov__
```
cdq ; = (a < 0) ? 0xFFFFFFFF : 0
xor ebx, ecx ; b ^ c = bits that differ between b and c
and edx, ebx ; = (a < 0) ? (b ^ c) : 0
xor edx, ecx ; = (a < 0) ? b : c
```






## Multiplication (Integer without taking care of carry)

```
====MULTIPLICATION BY 2=========
[MUL 1] Use 'imul' instruction
[MUL 1] Time : 2.511815
[MUL 2] Use 'mov, add' instruction
[MUL 2] Time : 2.491328
[MUL 3] Use 'shl' instruction
[MUL 3] Time : 2.455770
[MUL 4] Use 'lea' instruction
[MUL 4] Time : 2.471328
====MULTIPLICATION BY 5=========
[MUL 5] Use 'imul' instruction
[MUL 5] Time : 2.590941
[MUL 6] Use 'lea' instruction
[MUL 6] Time : 2.526823
```
