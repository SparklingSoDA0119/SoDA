#include <SoDA_S2Type.h>

using namespace SoDA;
using namespace std;

S2Image::S2Image()
{
	_usS_Row = 0;
	_usS_Col = 0;

	_usS_Type = UNDEFINED;
}

void S2Image::setImageInfo(usShort row, usShort col, usShort type)
{
	_usS_Row  = row;
	_usS_Col  = col;
	_usS_Type = type;
}

