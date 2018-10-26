
; nasm -fmacho64 mul4.s
; gcc main_mul.o mul4.o
;
; using mul instruction
;

	section .text
	global _mul4

_mul4:
	mov rax, 0xbeef
	lea rax, [rax+1*rax]
	ret

