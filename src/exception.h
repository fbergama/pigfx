// inspired by the uspi project, modified to fit PiGFX

#ifndef EXCEPTION_H__
#define EXCEPTION_H__

#define ARM_OPCODE_BRANCH(distance)	(0xEA000000 | (distance))
#define ARM_DISTANCE(from, to)		((unsigned int *) &(to) - (unsigned int *) &(from) - 2)

#define EXCEPTION_DIVISION_BY_ZERO		0
#define EXCEPTION_UNDEFINED_INSTRUCTION		1
#define EXCEPTION_PREFETCH_ABORT		2
#define EXCEPTION_DATA_ABORT			3

typedef struct TAbortFrame
{
	unsigned int	sp_irq;
	unsigned int	lr_irq;
	unsigned int	r0;
	unsigned int	r1;
	unsigned int	r2;
	unsigned int	r3;
	unsigned int	r4;
	unsigned int	r5;
	unsigned int	r6;
	unsigned int	r7;
	unsigned int	r8;
	unsigned int	r9;
	unsigned int	r10;
	unsigned int	r11;
	unsigned int	r12;
	unsigned int	sp;
	unsigned int	lr;
	unsigned int	spsr;
	unsigned int	pc;
}
TAbortFrame;


#endif
