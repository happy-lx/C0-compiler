.data
	max:	.word 1000
	msg1:	.asciiz " "
.text
	j	main
main:
	sw	$ra,0($sp)
	sw	$fp,-4($sp)
	add	$fp,$sp,$0
	subi	$sp,$sp,36
	li	$t1,1
	sw	$t1,-8($fp)
Lable0:
	li	$t1,1
	sw	$t1,-12($fp)
Lable1:
	lw	$t1,-12($fp)
	lw	$t2,-12($fp)
	mul	$t3,$t1,$t2
	sw	$t3,-16($fp)
	lw	$t1,-16($fp)
	lw	$t2,-8($fp)
	bne	$t1,$t2,Lable2
	li	$v0,4
	la	$a0,msg1
	syscall
	lw	$a0,-8($fp)
	li	$v0,1
	syscall
	j	Lable3
Lable2:
Lable3:
	lw	$t1,-12($fp)
	li	$t2,1
	add	$t3,$t1,$t2
	sw	$t3,-24($fp)
	lw	$t1,-24($fp)
	sw	$t1,-12($fp)
	lw	$t1,-12($fp)
	lw	$t2,-8($fp)
	ble	$t1,$t2,Lable1
	lw	$t1,-8($fp)
	li	$t2,1
	add	$t3,$t1,$t2
	sw	$t3,-32($fp)
	lw	$t1,-32($fp)
	sw	$t1,-8($fp)
	lw	$t1,-8($fp)
	la	$s1,max
	lw	$t2,0($s1)
	ble	$t1,$t2,Lable0
