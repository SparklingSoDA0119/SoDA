#ifndef _SODA_PRINT_TRIANGLE_H
#define _SODA_PRINT_TRIANGLE_H

#include "Example.h"

namespace SoDA {
	class Triangle {
	public :
		Triangle();
		virtual ~Triangle() {}

	public :
		void printTriangle1(int32 height);
		void printTriangle2(int32 height);
		void printTriangle3(int32 height);
		void printTriangle4(int32 height);
	}; // Triangle
} // namespace SoDA

#endif // _SODA_PRINT_TRIANGLE_H