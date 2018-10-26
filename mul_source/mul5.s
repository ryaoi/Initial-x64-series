
; nasm -fmacho64 mul5.s
; gcc main_mul.o mul5.o
;
; using imul instruction
;

	section .text
	global _mul5

_mul5:
	mov rax, 0xbeef
	imul rax, 5
	ret
