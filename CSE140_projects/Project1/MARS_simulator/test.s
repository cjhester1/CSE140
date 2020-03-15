#
# Note that this is by no means a comprehensive test!
#

		.text
		addiu	$a0,$0,3
		addiu	$a1,$0,2
		ori	$t0,0x00401004
		sw	$a1,0($t0)
		lw	$a2,0($t0)
		andi	$a3,$a0,100
		lui	$t2,100
		slt	$t3,$a1,$a2
		addi	$0,$0,0 #unsupported instruction, terminate

