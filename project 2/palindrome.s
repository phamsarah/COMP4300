	.text
main:
	LA $0, 12583352
	LI $1, 1024
	LI $3, 8
	SYSCALL
	
	LA $4, 12583096
	LA $5, 12583032	

length_loop:			
	LB $6, $5, 0			
	BEQZ $6, 128 

	ADDI $5, $5, 8	
	B -256		

end_length_loop:
	SUBI $5, $5, 16	

test_loop:
	BGE $4, $5, 384	

	LB $6, $4, 0		
	LB $7, $5, 0		
	BNE $6, $7, 448 

	ADDI $4, $4, 8	
	SUBI $5, $5, 8	
	B -448		

is_palin:			

	LA $0, 12583800
	LI $3, 4
	SYSCALL
	B 256

not_palin:
	LA $0, 12582528	
	LI $3, 4
	SYSCALL
	B 0

exit:				
	LI $3, 10		
	SYSCALL			
	END

	.data
is_palin_msg: .asciiz "The string is a palindrome. "
not_palin_msg: .asciiz "The string is not a palindrome. "
string_space: .space 1024
