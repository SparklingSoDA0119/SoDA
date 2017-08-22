#include "SoDA_File.h"
#include "SoDA_Const.h"


using namespace SoDA;
using namespace std;

File::File()
	: _pFile(NULL)
	, _fileName(NULL)
	, _status(File::FILE_CLOSE), _fileType(File::EMPTY)
{

}

File::~File()
{

}

void File::init()
{
	_pFile = NULL;
	if (_fileName != NULL) {
		delete [] _fileName;
		_fileName = new char[MAX_NAME_LENGTH];
	}
	else {
		_fileName = new char[MAX_NAME_LENGTH];
	}

	_status   = FILE_CLOSE;
	_fileType = EMPTY;
}

RawFile::RawFile()
{
}

RawFile::~RawFile()
{
	if (_fileName != NULL) {
		delete [] _fileName;
	}
}

void File::setFileName(char* fileName)
{
	for (int32 i = 0; i < MAX_NAME_LENGTH; i++) {
		_fileName[i] = fileName[i];
	}
}

int32 RawFile::openFile()
{
	int32 ret = 0;

	if ((_pFile == NULL) && (_status == FILE_CLOSE)) {
		_pFile = fopen(_fileName, "rb");
	}

	if (_pFile == NULL) {
		cout << "Can not open " << _fileName << endl;
		ret = FAIL;
	}
	else {
		_status = FILE_OPEN_AND_NOT_READ;
		ret = SUCCESS;
	}

	return ret;
}

int32 RawFile::openFile(char* fileName, int32 openType)
{
	int32 retVal = FAIL;
	
	setFileName(fileName);
	
	char type[3];

	switch(openType) {
		case READ_ONLY :
			type[0] = 'r';
			type[1] = 'b';
			type[2] = '\0';
			break;
		case READ_AND_WRITE :
		case WRITE_ONLY :
			type[0] = 'w';
			type[1] = 'b';
			type[2] = '\0';
			break;
	}

	if ((_pFile == NULL) && (_status == FILE_CLOSE)) {
		_pFile = fopen(fileName, type);
	}

	if (_pFile == NULL) {
		cout << "Can not open " << fileName << endl;
	}
	else {
		_status = FILE_OPEN_AND_NOT_READ;
		retVal = SUCCESS;
	}

	return retVal;
}

int32 RawFile::readFile(uint8* buffer, int32 size)
{
	int32 retVal = FAIL;

	if ((_status != FILE_CLOSE) && (_pFile != NULL)) {
		if(!fread(buffer, sizeof(uint8), size, _pFile)) {
			cout << "Can not read file " << _fileName << endl;
			retVal = FAIL;
		}
		else {
			_status = FILE_OPEN_AND_READ;
			retVal = SUCCESS;
		}
	}

	return retVal;
}

int32 RawFile::writeFile(uint8* buffer, int32 size)
{
	int32 ret = 0;

	ret = openFile(_fileName, WRITE_ONLY);
	fwrite(buffer, sizeof(unsigned char), size, _pFile);
	fclose(_pFile);

	return ret;
}