.data
	x:	.word 10
.text
	j	main
factorial:
	sw	$ra,0($sp)
	sw	$fp,-4($sp)
	add	$fp,$sp,$0
	subi	$sp,$sp,24
	lw	$t1,4($fp)
	li	$t2,0
	bgt	$t1,$t2,Lable0
	li	$v1,0
	add	$sp,$fp,$0
	lw	$ra,0($sp)
	addi	$sp,$sp,4
	lw	$fp,-8($sp)
	jr	$ra
	j	Lable1
Lable0:
Lable1:
	lw	$t1,4($fp)
	li	$t2,1
	bne	$t1,$t2,Lable2
	li	$v1,1
	add	$sp,$fp,$0
	lw	$ra,0($sp)
	addi	$sp,$sp,4
	lw	$fp,-8($sp)
	jr	$ra
	j	Lable3
Lable2:
Lable3:
	lw	$t1,4($fp)
	li	$t2,1
	sub	$t3,$t1,$t2
	sw	$t3,-16($fp)
	subi	$sp,$sp,4
	lw	$t1,-16($fp)
	sw	$t1,0($sp)
	subi	$sp,$sp,4
	jal	factorial
	li	$t1,4
	add	$sp,$sp,$t1
	sw	$v1,-20($fp)
	lw	$t1,4($fp)
	lw	$t2,-20($fp)
	mul	$t3,$t1,$t2
	sw	$t3,-24($fp)
	lw	$v1,-24($fp)
	add	$sp,$fp,$0
	lw	$ra,0($sp)
	addi	$sp,$sp,4
	lw	$fp,-8($sp)
	jr	$ra
main:
	sw	$ra,0($sp)
	sw	$fp,-4($sp)
	add	$fp,$sp,$0
	subi	$sp,$sp,12
	subi	$sp,$sp,4
	la	$t1,x
	lw	$t1,0($t1)
	sw	$t1,0($sp)
	subi	$sp,$sp,4
	jal	factorial
	li	$t1,4
	add	$sp,$sp,$t1
	sw	$v1,-12($fp)
	lw	$t1,-12($fp)
	sw	$t1,-8($fp)
	lw	$a0,-8($fp)
	li	$v0,1
	syscall
