// inspired by the uspi project, modified to fit PiGFX

#ifndef EXCEPTION_H__
#define EXCEPTION_H__

#define ARM_OPCODE_BRANCH(distance)	(0xEA000000 | (distance))
#define ARM_DISTANCE(from, to)		((unsigned int *) &(to) - (unsigned int *) &(from) - 2)

#define EXCEPTION_DIVISION_BY_ZERO		0
#define EXCEPTION_UNDEFINED_INSTRUCTION		1
#define EXCEPTION_PREFETCH_ABORT		2
#define EXCEPTION_DATA_ABORT			3

#endif
