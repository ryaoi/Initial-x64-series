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


`#define SIZE 0xF0000`


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


## Optimizing for size 

### Choosing shorter instructions

I will show how short its going to be on 64bits

Instructions with pointers take one byte less when they have only a base pointer (except
ESP, RSP or R12) and a displacement than when they have a scaled index register, or both
base pointer and index register, or ESP, RSP or R12 as base pointer. Examples:
```
  400080:	67 8b 45 0c          	mov    eax,DWORD PTR [ebp+0xc]
  400084:	67 8b 44 24 0c       	mov    eax,DWORD PTR [esp+0xc]
```

Instructions with BP, EBP, RBP or R13 as base pointer and no displacement and no index
take one byte more than with other registers:
```
  400089:	67 8b 03             	mov    eax,DWORD PTR [ebx]
  40008c:	67 8b 45 00          	mov    eax,DWORD PTR [ebp+0x0]
```

Instructions in 64-bit mode need a REX prefix if at least one of the registers R8 - R15 or XMM8
- XMM15 are used. Instructions that use these registers are therefore one byte longer than
instructions that use other registers, unless a REX prefix is needed anyway for other
reasons:
```
  400098:	8b 03                	mov    eax,DWORD PTR [rbx]
  40009a:	41 8b 00             	mov    eax,DWORD PTR [r8]
```

### Using shorter constants and addresses
Data constants may also take less space if they are between -128 and +127. Most
instructions with immediate operands have a short form where the operand is a signextended
single byte. Examples:
```
  400080:	68 c8 00 00 00       	push   0xc8 ; push 200
  400085:	6a 64                	push   0x64 ; push 100
```

```
  400087:	81 c3 80 00 00 00    	add    ebx,0x80               ;  add  128 
  40008d:	83 eb 80             	sub    ebx,0xffffff80         ;  sub -128
```

Shorter alternatives for MOV register,constant are often useful. Examples:
```
  400080:	b8 00 00 00 00       	mov    eax,0x0    ; 5 bytes
  400085:	29 c0                	sub    eax,eax    ; 2 bytes
```

```
  400087:	b8 01 00 00 00       	mov    eax,0x1    ; 5 bytes
 
  40008c:	29 c0                	sub    eax,eax    ; 2 bytes
  40008e:	ff c0                	inc    eax        ; 2 bytes so total 4 bytes

  400090:	6a 01                	push   0x1        ; 2 bytes
  400092:	58                   	pop    rax        ; 1 byte so total 3 bytes
  
  
  
    400085:	48 29 c0             	sub    rax,rax ; <- sub rax, rax takes 3 bytes so better use 'sub eax,eax'
    
```

```
  400093:	b8 ff ff ff ff       	mov    eax,0xffffffff  ; 5 bytes
  400098:	83 c8 ff             	or     eax,0xffffffff  ; 3 bytes
```


### Cache

Align stuff by 16 bytes. code and data.

### Loop

The loop overhead is the instructions needed for jumping back to the beginning of the loop
and to determine when to exit the loop. Optimizing these instructions is a fairly general
technique that can be applied in many situations. 
gnu already does some optimization about this so there is no need to talk about this.

Example:
```
0000000000000660 <main>:
 660:	55                   	push   rbp
 661:	48 89 e5             	mov    rbp,rsp
 664:	c7 45 f8 0a 00 00 00 	mov    DWORD PTR [rbp-0x8],0xa
 66b:	c7 45 fc 00 00 00 00 	mov    DWORD PTR [rbp-0x4],0x0
 672:	eb 04                	jmp    678 <main+0x18>
 674:	83 45 fc 01          	add    DWORD PTR [rbp-0x4],0x1
 678:	8b 45 fc             	mov    eax,DWORD PTR [rbp-0x4]
 67b:	3b 45 f8             	cmp    eax,DWORD PTR [rbp-0x8]
 67e:	7c f4                	jl     674 <main+0x14>
 680:	b8 00 00 00 00       	mov    eax,0x0
 685:	5d                   	pop    rbp
 686:	c3                   	ret
 687:	66 0f 1f 84 00 00 00 	nop    WORD PTR [rax+rax*1+0x0]
 68e:	00 00
```

`jl     674 <main+0x14>` jump under the other `jmp` instruction so it's all good!

### Problematic Instructions

#### lea

The LEA instruction is useful for many purposes because it can do a shift operation, two
additions, and a move in just one instruction.

```
lea eax, [ebx+8*ecx-1000]
```

which is same as :
```
mov eax, ecx
shl eax, 3
add eax, ebx
sub eax, 1000
```

The processors have no documented addressing mode with a scaled index register and
nothing else. Therefore, an instruction like lea eax,[ebx*2] is actually coded as lea
eax,[ebx*2+00000000H] with an immediate displacement of 4 bytes. The size of this
instruction can be reduced by writing lea eax,[ebx+ebx]. If you happen to have a register
that is zero (like a loop counter after a loop) then you may use it as a base register to
reduce the code size:

```
67 8d 04 9d 00 00 00 	lea    eax,[ebx*4+0x0]   ; lea eax, [ebx*4]     ; 8 bytes
67 8d 04 99          	lea    eax,[ecx+ebx*4]   ; lea eax, [ecx+ebx*4] ; 4 bytes
```

```
  400080:	8d 04 9d 00 00 00 00 	lea    eax,[rbx*4+0x0] ; lea eax, [rbx*4]  	; 7bytes
  400087:	8d 04 99             	lea    eax,[rcx+rbx*4] ; lea eax,[rcx+rbx*4]	; 3 bytes
```

### INC and DEC

Use ADD and SUB when optimizing for speed. Use INC and DEC when optimizing for size or
when no penalty is expected.

### Bit test

BT, BTC, BTR, and BTS instructions should preferably be replaced by instructions like TEST,
AND, OR, XOR, or shifts on older processors. Bit test
instructions are useful when optimizing for size.

### Integer multiplication (all processors)

An integer multiplication takes from 3 to 14 clock cycles, depending on the processor. It is
therefore often advantageous to replace a multiplication by a constant with a combination of
other instructions such as SHL, ADD, SUB, and LEA. For example IMUL EAX,5 can be
replaced by LEA EAX,[EAX+4*EAX].

### Division (all processors) 

#### Integer division by a power of 2 (all processors)

```
; Divide unsigned integer by 2^N
shr eax, N
```

```
; Divide signed integer by 2^N
cdq
shr edx,32-N
add eax, edx
sar eax, N
```

### Integer division by a constant (all processors)

b = (the number of significant bits in d) - 1
r = w + b
f = 2r
/ d
If f is an integer then d is a power of 2: go to case A.
If f is not an integer, then check if the fractional part of f is < 0.5
If the fractional part of f < 0.5: go to case B.
If the fractional part of f > 0.5: go to case C.
case A (d = 2b):
result = x SHR b
case B (fractional part of f < 0.5):
round f down to nearest integer
149
result = ((x+1) * f) SHR r
case C (fractional part of f > 0.5):
round f up to nearest integer
result = (x * f) SHR r

Example:
Assume that you want to divide by 5.
5 = 101B.
w = 32.
b = (number of significant binary digits) - 1 = 2
r = 32+2 = 34
f = 234 / 5 = 3435973836.8 = 0CCCCCCCC.CCC...(hexadecimal)

The fractional part is greater than a half: use case C.
Round f up to 0CCCCCCCDH.
The following code divides EAX by 5 and returns the result in EDX:
```
; Divide unsigned integer eax by 5
mov edx, 0CCCCCCCDH
mul edx
shr edx, 2 
```

```

  segment .text
  global _start

_start:
	push rbp
	mov rbp, rsp
	push 11
	pop  rax
	mov edx, 0CCCCCCCDH
	mul edx
	shr edx, 2
	leave
	ret
	
gdb-peda$ info register edx
edx            0x2	0x2
gdb-peda$
```

the result is good!

### String instructions (all processors) 

REP MOVSD and REP STOSD are quite fast if the repeat count is not too small. Always use
the largest word size possible (DWORD in 32-bit mode, QWORD in 64-bit mode), and make sure
that both source and destination are aligned by the word size. In many cases, however, it is
faster to use XMM registers. Moving data in XMM registers is faster than REP MOVSD and
REP STOSD in most cases, especially on older processors. See page 164 for details.

On many processors, REP MOVS and REP STOS can perform fast by moving 16 bytes or an
entire cache line at a time. This happens only when certain conditions are met. Depending
on the processor, the conditions for fast string instructions are, typically, that the count must
be high, both source and destination must be aligned, the direction must be forward, the
distance between source and destination must be at least the cache line size, and the
memory type for both source and destination must be either write-back or write-combining
(you can normally assume the latter condition is met).







## Some examples on my machine

### Multiplication (Integer without taking care of overflow)

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
