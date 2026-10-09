.intel_syntax noprefix
# Setting .global to _start appears to be required for the resulting
# executable to be a valid ELF executable. So in the code generation
# step, always enter in _start and from there we call main().
.global _start

# The string can technically be stored in .text, but if it is put
# their then it has memory permissions readable and executable.
# Readable makes sense, but not executable. It does not have writable,
# which may be desired for some data. In the .data section, the data
# is readable and writable.
.data
string:
	.ascii "Hello, world\n"
	
# There is also the .rodata section for read-only data. Data stored
# there is readable but not writable and not executable.
# NOTE: For some reason the ".section" is implicit for some sections
# like .text and .data, but not for .rodata. It is possible to also
# write .section .data, for example.
.section .rodata
const_string:	
	.ascii "This is a constant string\n"

# Executable code should be contained in the .text section, which we
# enter here.
.text
#BEGIN#########################################################################
_start:
# 	call test_function
	mov edi, 42 # First argument
	call main # Basically: main(42)
	# The return value is available in eax or rax

	mov rax, 1 # This is the syscall number to call, 1 = write
	mov rdi, 1 # This is the first argument to write, which is the file descriptor to write to
	lea rsi, [rip + string] # This is the second argument to write, the string to write
	mov rdx, 13 # This is the third argument to write, number of bytes to write
# So we are essentially calling write(1, string, 13)
# Alternatively, we call syscall(1, 1, string, 13)
# 	mov rsi, const_string
# 	mov rdx, 26
	syscall

# Calling an appropriate exit syscall is also required for a valid ELF
# executable. The assembly code generated for main() does not need to
# call this syscall, but the code under the _start label needs to end
# with the appropriate exit syscall. This is done below.
	mov rax, 60 # Syscall number for exit()
	xor rdi, rdi # Could also do mov rdi, 0 but apparently xor is more efficient
# Here we are essentially calling exit(0), or syscall(60, 0)
	syscall
#END###########################################################################

#BEGIN#########################################################################
main:
	mov rax, 0
	ret
#END###########################################################################



#BEGIN#########################################################################
test_function:
	ret
#END###########################################################################




