# These must be kept in sync with the C code!
.set IH_STACKFRAME_R3, 0x8 
.set IH_STACKFRAME_LR, 0xc
.set IH_STACKFRAME_TRAMPOLINE, 0x10
.set IH_STACKFRAME_CTX, 0x20
.set IH_STACKFRAME_SIZE, 0x20 + 0x2c8

.globl patchInstructionHookSaveContextAndHandle
patchInstructionHookSaveContextAndHandle:
	# Entry state:
	# * r1 is set up for a stackframe of size IH_STACKFRAME_SIZE
	# * r3 contains original LR
	# * [r1 + IH_STACKFRAME_R3].d contains original R3
	# * lr points to trampoline
	# * [lr + 0x8].d contains handler fn ptr
	# * [lr + 0xc].d contains handler user data

	# Save link register
	stw %r3, (IH_STACKFRAME_LR)(%r1)
	# Save trampoline return point
	mflr %r3
	stw %r3, (IH_STACKFRAME_TRAMPOLINE)(%r1)

	# Write r0
	stw %r0, (IH_STACKFRAME_CTX + 0x0+4*0)(%r1)

	# Write r1-r2, r13-r31, GQRs 1-7, CR, false LR/SRR0, MSR (SRR1), CTR, XER
	addi %r3, %r1, IH_STACKFRAME_CTX
	bl OSSaveContext

	# Correct r1 to before the stackframe was established
	addi %r3, %r1, IH_STACKFRAME_SIZE
	stw %r3, (IH_STACKFRAME_CTX + 0x0+4*1)(%r1)

	# Finish GPRs
	# Write r3-r12 (double-writing r13-r31)
	lwz %r3, (IH_STACKFRAME_R3)(%r1)
	stmw %r3, (IH_STACKFRAME_CTX + 0x0+4*3)(%r1)

	# Zero control fields
	li %r3, 0
	stw %r3, (IH_STACKFRAME_CTX + 0x1a0)(%r1)

	# Write f0-f31, paired single variants and FPCSR
	addi %r5, %r1, IH_STACKFRAME_CTX
	bl __OSSaveFPUContext

	# Correct LR
	lwz %r3, (IH_STACKFRAME_LR)(%r1)
	stw %r3, (IH_STACKFRAME_CTX + 0x84)(%r1)

	# Correct SRR0 to trampoline (keep pointer for access below)
	lwz %r4, (IH_STACKFRAME_TRAMPOLINE)(%r1)
	stw %r4, (IH_STACKFRAME_CTX + 0x198)(%r1)

	# Set saved LR for debugging purposes
	stw %r4, (IH_STACKFRAME_SIZE + 0x4)(%r1)

	# Invoke handler
	# The offset of these must be kept in sync with the C code!
	lwz %r3, 0x8(%r4) # handler fn ptr
	mtlr %r3

	addi %r3, %r1, IH_STACKFRAME_CTX
	lwz %r4, 0xc(%r4) # handler user arg
	blrl

	# Restore f0-f31, paired single variants and FPCSR
	addi %r4, %r1, IH_STACKFRAME_CTX
	bl __OSLoadFPUContext

	# Set control bit to restore all GPRs
	li %r3, 2
	sth %r3, (IH_STACKFRAME_CTX + 0x1a2)(%r1)

	# Disable interrupts prior to OSLoadContext since it must be invoked such
	bl OSDisableInterrupts

	# Restore everything else, jumping back into the trampoline
	addi %r3, %r1, IH_STACKFRAME_CTX
	b OSLoadContext
