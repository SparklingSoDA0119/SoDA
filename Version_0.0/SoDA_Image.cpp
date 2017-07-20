#include "G:\ProjectHS\SoDA\Version_0.0\SoDA_Image.h"

using namespace SoDA;
using namespace std;

Image::Image()
{
	_usS_Row = 0;
	_usS_Col = 0;

	_usS_Type = UNDEFINED;
}

void Image::setImageInfo(usShort row, usShort col, usShort type)
{
	_usS_Row  = row;
	_usS_Col  = col;
	_usS_Type = type;
}

