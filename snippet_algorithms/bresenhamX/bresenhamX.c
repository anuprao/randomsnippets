// gcc bresenhamX.c -o bresenhamX

#include <stdio.h>

#define	PIXEL	int

void scale_linear(PIXEL Target, PIXEL Source, int SrcWidth, int TgtWidth)
{
	int NumPixels = TgtWidth;
	int IntPart = SrcWidth / TgtWidth;
	int FractPart = SrcWidth % TgtWidth;
	int E = 0;

	printf("I:%d\tF:%d\n", IntPart, FractPart);
	while (NumPixels-- > 0)
	{
		//*Target++ = *Source;
		printf("%2d >> %2d\tE=%d\n", Source, Target, E);
		Target++;

		Source += IntPart;
		E += FractPart;

		if (E >= TgtWidth)
		{
			E -= TgtWidth;
			Source++;
		}
	}
}

int main(void)
{
	printf("Dedicated to Bresenham\n");
	
	scale_linear((PIXEL)0x0, (PIXEL)0x0, 20, 26);
	
	printf("Done.\n");
	return 0;
}

/*

Dedicated to Bresenham
I:1	F:6
 0 >>  0	E=0
 1 >>  1	E=6
 2 >>  2	E=12
 4 >>  3	E=4
 5 >>  4	E=10
 7 >>  5	E=2
 8 >>  6	E=8
10 >>  7	E=0
11 >>  8	E=6
12 >>  9	E=12
14 >> 10	E=4
15 >> 11	E=10
17 >> 12	E=2
18 >> 13	E=8
Done.


Dedicated to Bresenham
I:0	F:20
 0 >>  0	E=0
 0 >>  1	E=20
 1 >>  2	E=14
 2 >>  3	E=8
 3 >>  4	E=2
 3 >>  5	E=22
 4 >>  6	E=16
 5 >>  7	E=10
 6 >>  8	E=4
 6 >>  9	E=24
 7 >> 10	E=18
 8 >> 11	E=12
 9 >> 12	E=6
10 >> 13	E=0
10 >> 14	E=20
11 >> 15	E=14
12 >> 16	E=8
13 >> 17	E=2
13 >> 18	E=22
14 >> 19	E=16
15 >> 20	E=10
16 >> 21	E=4
16 >> 22	E=24
17 >> 23	E=18
18 >> 24	E=12
19 >> 25	E=6
Done.

*/

