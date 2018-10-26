
; nasm -fmacho64 mul1.s
; gcc main_mul.o mul1.o
;
; using mul instruction
;

	section .text
	global _mul1

_mul1:
	mov rax, 0xbeef
	imul rax, 2
	ret
