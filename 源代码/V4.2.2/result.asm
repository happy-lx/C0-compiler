.data
	a:	.word 0
	c:	.word 3
.text
	j	main
fib:
	sw	$ra,0($sp)
	sw	$fp,-4($sp)
	add	$fp,$sp,$0
	subi	$sp,$sp,24
	lw	$s0,36($fp)
	li	$t2,1
	bne	$s0,$t2,Lable0
	li	$v1,1
	add	$sp,$fp,$0
	lw	$ra,0($sp)
	addi	$sp,$sp,4
	lw	$fp,-8($sp)
	jr	$ra
Lable0:
	li	$t2,1
	sub	$s1,$s0,$t2
	subi	$sp,$sp,4
	sw	$s1,0($sp)
	sw	$s0,-4($sp)
	sw	$s1,-8($sp)
	sw	$s2,-12($sp)
	sw	$s3,-16($sp)
	sw	$s4,-20($sp)
	sw	$s5,-24($sp)
	sw	$s6,-28($sp)
	sw	$s7,-32($sp)
	subi	$sp,$sp,32
	subi	$sp,$sp,4
	jal	fib
	lw	$s7,0($sp)
	lw	$s6,4($sp)
	lw	$s5,8($sp)
	lw	$s4,12($sp)
	lw	$s3,16($sp)
	lw	$s2,20($sp)
	lw	$s1,24($sp)
	lw	$s0,28($sp)
	addi	$sp,$sp,32
	li	$t1,4
	add	$sp,$sp,$t1
	move	$s2,$v1
	mul	$s3,$s0,$s2
	move	$v1,$s3
	add	$sp,$fp,$0
	lw	$ra,0($sp)
	addi	$sp,$sp,4
	lw	$fp,-8($sp)
	jr	$ra
main:
	sw	$ra,0($sp)
	sw	$fp,-4($sp)
	add	$fp,$sp,$0
	subi	$sp,$sp,52
	subi	$sp,$sp,4
	li	$t1,5
	sw	$t1,0($sp)
	sw	$s0,-4($sp)
	sw	$s1,-8($sp)
	sw	$s2,-12($sp)
	sw	$s3,-16($sp)
	sw	$s4,-20($sp)
	sw	$s5,-24($sp)
	sw	$s6,-28($sp)
	sw	$s7,-32($sp)
	subi	$sp,$sp,32
	subi	$sp,$sp,4
	jal	fib
	lw	$s7,0($sp)
	lw	$s6,4($sp)
	lw	$s5,8($sp)
	lw	$s4,12($sp)
	lw	$s3,16($sp)
	lw	$s2,20($sp)
	lw	$s1,24($sp)
	lw	$s0,28($sp)
	addi	$sp,$sp,32
	li	$t1,4
	add	$sp,$sp,$t1
	move	$s0,$v1
	move	$s1,$s0
	li	$t2,0
	div	$s0,$t2
	mflo	$s0
	li	$t2,-1
	move	$s0,$t2
	li	$t2,-2
	move	$s0,$t2
	li	$t1,1
	li	$t2,3
	ble	$t1,$t2,Lable2
	li	$t2,4
	move	$s0,$t2
	j	Lable3
Lable2:
	li	$t2,'h'
	move	$s2,$t2
Lable3:
	li	$a0,'c'
	li	$v0,11
	syscall
