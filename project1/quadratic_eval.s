#
# Code to evaluate a quadratic
#

	.text
	
main:
	lw $t0, X
	lw $t1, A
	lw $t2, B
	lw $t3, C

	mul $t4, $t0, $t0 # mul $t0, $t0, $t4 -> X**2, $t4 is destination, we need X later
	mul $t1, $t1, $t4 # mul $t1, $t4, $t4 -> A*X**2, we store new result in t1 and multiply
	mul $t2, $t2, $t0 # mul $t0, $t2, $t5 -> B*X, we store the result in t2 because we need B*X later
	add $t4, $t1, $t2 # add $t4, $t1, $t6 -> A*X**2 + B*X, store in t4
	add $t3, $t4, $t3 # A*X**2 + B*X + C -> add $t3, $t4, $t4

	la $a0, ans        # print a string
	li $v0, 4
	syscall

	move $a0, $t3      # move $a0, $t6 -> now the result
	li $v0, 1
	syscall

	la $a0, nl         # and a newline
	li $v0, 4
	syscall

	li $v0, 10         # g'bye
	syscall

	.data
X:	.word 3
A:	.word 7
B:	.word 5
C:	.word 4
ans:	.asciiz "Answer = "
nl:	.asciiz "\n"
