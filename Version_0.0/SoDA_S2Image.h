#ifndef _SODA_IMAGE_H
#define _SODA_IMAGE_H

#include <SoDA_S2Type.h>

namespace SoDA {
class S2Image {
	
public :
	S2Image();
	virtual ~S2Image() {};

public :
	enum { RAW = 0,
		   BMP,
		   JPG,
		   UNDEFINED,
	};

	usShort _usS_Row;	// Row Pixel Size of Luminance
	usShort _usS_Col;	// Column Pixel Size of Luminance

	void setImageInfo(usShort row, usShort col, usShort type);

private :
	usShort _usS_Type;	// Type of Image
	
};	//	Image
}	//	namespace SoDA

#endif		//_SODA_IMAGE_H
