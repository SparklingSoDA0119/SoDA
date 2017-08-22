#ifndef _SODA_IMAGE_H_
#define _SODA_IMAGE_H_

#include "SoDA_Type.h"
#include "SoDA_File.h"
#include <string>

using namespace std;

namespace SoDA {

	class Image {
	public :
		Image();
		~Image();

	protected :
		uint8* _frame;
		uint8* _luma;
		uint8* _chromaCb;
		uint8* _chromaCr;
		
		int32 _imgWidth;
		int32 _imgHeight;
		int32 _cbWidth;
		int32 _cbHeight;
		int32 _crWidth;
		int32 _crHeight;

		int32 _imgType;
		
		int32 _colorType;
		bool  _isFileOpen;

	protected :
		//void setImageSize(int32 width, int32 height);
		//void setImageInfo(int32 width, int32 height, int32 colorType);
		void setImageWidth    (int32 width)		{ _imgWidth = width; }
		void setImageHeight	  (int32 height)	{ _imgHeight = height; }
		void setImageColorType(int32 colorType) { _colorType = colorType; }
		void setCbWidth(int32 cbWidth) { _cbWidth = cbWidth; }
		void setCbHeight(int32 cbHeight) { _cbHeight = cbHeight; }
		void setCrWidth(int32 crWidth) { _crWidth = crWidth; }
		void setCrHeight(int32 crHeight) { _crHeight = crHeight; }

		

		void setSizeOfColorSpace(int32 width, int32 height);

		uint8* memAlloc(int32 width, int32 height);		

	public :
		enum {
			MONO= 0,
			COLOR_400,
			COLOR_420,
			COLOR_422,
			COLOR_444,
		};

		enum {
			WRONG_WIDTH_AND_HEIGHT = 0,
			MEM_ALLOC_SUCCESS,
		};
	}; // class Image

	class RawImage : public Image {
	public :
		RawImage();
		~RawImage() {}

	private :
		RawFile _inputFile;
		RawFile _outputFile;

	public :
		void setInputFile(char* fileName) { _inputFile.setFileName(fileName); }
		void setOutputFile(char* fileName) { _outputFile.setFileName(fileName); }
		int32 loadInputImage(int32 width, int32 height, int32 colorType);
		int32 copyFrameToLuma(int32 width, int32 height);
		int32 writeOutputFile();
	};	// class RawImage
}	// namespace SoDA

#endif // _SODA_IMAGE_H_