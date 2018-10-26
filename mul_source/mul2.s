
; nasm -fmacho64 mul2.s
;
; using mov, add instruction
;

	section .text
	global _mul2

_mul2:
	mov rax, 0xbeef
	mov rdx, rax
	add rax, rdx
	ret
