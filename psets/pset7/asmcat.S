_main: 
.text                           

    .globl _start              	
			                
_start:

cat: 
	movl 	$4096,%edx          # buffer size 4096
	movl 	$buf,%ecx			# tmp string init 
	movl 	$0,%ebx				# read from stdin
	movl 	$3,%eax 			# system call 3 : read
	int     $0x80               # invoke system call

	cmpl $0, %eax               # compare , check strings
    
	je done	                    # jump if equal -> done (strings match)
	jl err					    # jump if less -> err (read error handling)
    
    # write/partial errors not included 

	movl    %eax,%edx           # length of previous read
	movl    $buf,%ecx           # buffer which contains the string to be cat
	movl    $1,%ebx             # write to stdout
	movl    $4,%eax             # system call 4 : write
	int     $0x80 				# invoke system call
	
	jmp cat              			

done:

	movl    %eax,%ebx           # exit code : 0 : success
	movl    $1,%eax             # exit code : 1 : system call
	int     $0x80               # invoke system call

err:

	mov     $-1,%ebx		    # exit code : -1 : read error  
	movl    $1,%eax
	int     $0x80               # invoke system call

.data                          

buf:                            # temp buffer for read/write
	.ascii    "\n"   

