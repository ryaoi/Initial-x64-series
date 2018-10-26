
; nasm -fmacho64 mul6.s
; gcc main_mul.o mul6.o
;
; using mul instruction
;

	section .text
	global _mul6

_mul6:
	mov rax, 0xbeef
	lea rax, [rax+4*rax]
	ret
