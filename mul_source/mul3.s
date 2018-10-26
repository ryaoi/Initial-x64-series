
; nasm -fmacho64 mul3.s
; gcc main_mul.o mul3.o
;
; using shl instruction
;

	section .text
	global _mul3

_mul3:
	mov rax, 0xbeef
	shl rax, 2
	ret
