.data
	msg1:	.asciiz "Please input ten numbers:\n"
.text
	j	main
main:
	sw	$ra,0($sp)
	sw	$fp,-4($sp)
	add	$fp,$sp,$0
	subi	$sp,$sp,128
	li	$v0,4
	la	$a0,msg1
	syscall
	li	$t2,0
	move	$s0,$t2
Lable1:
	li	$t2,10
	bge	$s0,$t2,Lable0
	li	$v0,5
	syscall
	move	$s2,$v0
	move	$t1,$s0
	sll	$t1,$t1,2
	la	$t2,-8($fp)
	sub	$t3,$t2,$t1
	sw	$s2,0($t3)
	li	$t2,1
	add	$s0,$s0,$t2
	j	Lable1
Lable0:
	li	$t2,0
	move	$s0,$t2
Lable3:
	li	$t2,9
	bge	$s0,$t2,Lable2
	li	$t2,0
	move	$s1,$t2
Lable5:
	li	$t1,9
	sub	$s4,$t1,$s0
	bge	$s1,$s4,Lable4
	move	$t1,$s1
	sll	$t1,$t1,2
	la	$t2,-8($fp)
	sub	$t3,$t2,$t1
	lw	$t2,0($t3)
	move	$s5,$t2
	li	$t2,1
	add	$s6,$s1,$t2
	move	$t1,$s6
	sll	$t1,$t1,2
	la	$t2,-8($fp)
	sub	$t3,$t2,$t1
	lw	$t2,0($t3)
	move	$s7,$t2
	ble	$s5,$s7,Lable6
	move	$t1,$s1
	sll	$t1,$t1,2
	la	$t2,-8($fp)
	sub	$t3,$t2,$t1
	lw	$t2,0($t3)
	sw	$t2,-96($fp)
	lw	$t2,-96($fp)
	move	$s2,$t2
	li	$t2,1
	add	$s3,$s1,$t2
	move	$t1,$s3
	sll	$t1,$t1,2
	la	$t2,-8($fp)
	sub	$t3,$t2,$t1
	lw	$t2,0($t3)
	sw	$t2,-104($fp)
	move	$t1,$s1
	sll	$t1,$t1,2
	la	$t2,-8($fp)
	sub	$t3,$t2,$t1
	lw	$t2,-104($fp)
	sw	$t2,0($t3)
	sw	$s3,-108($fp)
	lw	$t1,-108($fp)
	sll	$t1,$t1,2
	la	$t2,-8($fp)
	sub	$t3,$t2,$t1
	sw	$s2,0($t3)
Lable6:
	li	$t2,1
	add	$s1,$s1,$t2
	j	Lable5
Lable4:
	li	$t2,1
	add	$s0,$s0,$t2
	j	Lable3
Lable2:
	li	$t2,0
	move	$s0,$t2
Lable9:
	li	$t2,10
	bge	$s0,$t2,Lable8
	move	$t1,$s0
	sll	$t1,$t1,2
	la	$t2,-8($fp)
	sub	$t3,$t2,$t1
	lw	$t2,0($t3)
	sw	$t2,-124($fp)
	lw	$a0,-124($fp)
	li	$v0,1
	syscall
	li	$t2,1
	add	$s0,$s0,$t2
	j	Lable9
Lable8:
