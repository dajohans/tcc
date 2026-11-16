.intel_syntax noprefix
.global _start

.data
	
.section .rodata

.text
#BEGIN#########################################################################
_start:
	mov edi, 42 # First argument
	call main # Basically: main(42)
	mov rdi, rax # This doesn't seem to put the value of rax into rdi
	mov rax, 60 # Syscall number for exit()
	syscall
#END###########################################################################

#BEGIN#########################################################################
main:
	mov r8, 3
	mov r9, 5
	imul r9, r8
	mov rax, r9
	ret
#END###########################################################################

