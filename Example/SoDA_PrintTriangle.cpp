#include "SoDA_PrintTriangle.h"

using namespace std;
using namespace SoDA;

Triangle::Triangle()
{
}

/*********************************************
result :
*
**
***
**********************************************/
void Triangle::printTriangle1(int32 height)
{
	if (height < 1) {
		puts("It is not possible to print triagle\n");
	}
	else {
		int32 i, j;

		for (i = 0; i < height; i++) {
			for (j = 0; j < i + 1; j++) {
				printf("*");
			}
			printf("\n");
		}
	}
}

/*********************************************
result :
***
**
*
**********************************************/
void Triangle::printTriangle2(int32 height)
{
	if (height < 1) {
		puts("It is not possible to print triagle\n");
	}
	else {
		int32 i, j;

		for (i = 0; i < height; i++) {
			for (j = 0; j < height - i; j++) {
				printf("*");
			}
			printf("\n");
		}
	}
}

/*********************************************
result :
  *
 **
***
**********************************************/
void Triangle::printTriangle3(int32 height)
{
	if (height < 1) {
		puts("It is not possible to print triagle\n");
	}
	else {
		int32 i, j, k;

		for (i = 0; i < height; i++) {
			for (j = 0; j < height - 1 - i; j++) {
				printf(" ");
			}
			
			for (k = 0; k < i + 1; k++) {
				printf("*");
			}
			printf("\n");
		}
	}
}

/*********************************************
result :
   *
  ***
 *****
*******
**********************************************/
void Triangle::printTriangle4(int32 height)
{
	if (height < 1) {
		puts("It is not possible to print triagle\n");
	}
	else {
		int32 i, j, k, l;

		for (i = 0; i < height; i++) {
			for (j = 0; j < height - 1 - i; j++) {
				printf(" ");
			}
			
			for (k = 0; k < i + 1; k++) {
				printf("*");
			}

			if (i != 0) {
				for (l = 0; l < i; l++) {
					printf("*");
				}
			}
			printf("\n");
		}
	}
}