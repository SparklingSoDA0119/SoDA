#include "SoDA_Image.h"
#include "SoDA_Const.h"
#include <iostream>
#include <stdio.h>

using namespace SoDA;
using namespace std;

Image::Image()
	: _frame(NULL), _luma(NULL), _chromaCb(NULL), _chromaCr(NULL)
	, _imgWidth(0), _imgHeight(0), _imgType(0), _colorType(0)
	, _cbWidth(0), _cbHeight(0), _crWidth(0), _crHeight(0)
{
}

Image::~Image()
{
	if (_frame != NULL) {
		delete [] _frame;
	}

	if (_luma != NULL) {
		delete [] _luma;
	}

	if (_chromaCb != NULL) {
		delete [] _chromaCb;
	}

	if (_chromaCr != NULL) {
		delete [] _chromaCr;
	}
}

uint8* Image::memAlloc(int32 width  = 0, int32 height = 0)
{
	int32 ret = 0;

	uint8* pName = NULL;
	if (pName == NULL) {
		if (width == 0 && height == 0) {
			cout << "Getting memory space fail!!!" << endl;
			ret = WRONG_WIDTH_AND_HEIGHT;
		}
		else if (width != 0 && height == 0) {
			pName = new uint8[width];
		}
		else if (width == 0 && height != 0) {
			pName = new uint8[height];
		}
		else {
			pName = new uint8[width * height];
		}
	}
	else {
		delete [] pName;
		pName = memAlloc(width, height);
	}

	if (pName != NULL) {
		for (int32 i = 0; i < width * height; i++) {
			pName[i] = 0;
		}		
	}

	return pName;
}

void Image::setSizeOfColorSpace(int32 width, int32 height)
{
	switch(_colorType) {
		case MONO :
			break;
		case COLOR_420 :
			setCbWidth(width / 4);
			setCbHeight(height / 4);
			setCrWidth(width / 4);
			setCrHeight(height / 4);
			break;
		default :
			break;
	}
}

RawImage::RawImage()
{
	_inputFile.init();
	_outputFile.init();
}

int32 RawImage::loadInputImage(int32 width, int32 height, int32 colorType)
{
	setImageWidth(width);
	setImageHeight(height);
	setImageColorType(colorType);

	int32 ret = 0;
	int32 size = 0;
	ret = _inputFile.openFile();

	if (colorType == MONO) {
		size = width * height;
		_frame = memAlloc(width, height);

		ret = _inputFile.readFile(_frame, size);
		
		ret = copyFrameToLuma(width, height);
	}
	else if (colorType == COLOR_420) {
		setSizeOfColorSpace(width, height);
		int32 allWidth = width + _cbWidth + _crWidth;
		int32 allHeight = height + _cbHeight + _crHeight;
		size = allWidth * allHeight;

		_frame = memAlloc(allWidth, allHeight);
		
		ret = _inputFile.readFile(_frame, size);
	}
	else {
	}

	return ret;
}

int32 RawImage::copyFrameToLuma(int32 width, int32 height)
{
	if (_frame == NULL) {
		return 0;
	}
	else {
		if (_luma == NULL) {
			_luma = new uint8[width * height];
		}

		int32 i = 0;
		int32 j = 0;

		for (i = 0; i < width; i++) {
			for (j = 0; j < height; j++) {
				_luma[(j * width) + i] = _frame[(j * width) + i];
			}
		}

		return 1;
	}
}

int32 RawImage::writeOutputFile()
{
	int32 width = _imgWidth + _cbWidth + _crWidth;
	int32 height = _imgHeight + _cbHeight + _crHeight;
	_outputFile.writeFile(_frame, width * height);

	return 0;
}
