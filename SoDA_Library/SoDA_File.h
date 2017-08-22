#ifndef _SODA_FILE_H_
#define _SODA_FILE_H_

#include <stdio.h>
#include <string>
#include <iostream>
#include "SoDA_Type.h"

using namespace std;
namespace SoDA {

	class File {
	public :
		File();
		~File();

	protected :
		FILE* _pFile;

		char* _fileName;
		
		int32 _status;
		int32 _fileType;

	public :
		enum {
			FILE_CLOSE = 0,
			FILE_OPEN_AND_NOT_READ,
			FILE_OPEN_AND_READ,
			FILE_OPEN,
			FILE_WRITE_DONE,
		};

		enum {
			READ_ONLY = 0,
			READ_AND_WRITE,
			WRITE_ONLY,
		};

		enum {
			EMPTY = 0,
			RAW_IMAGE_FILE,
			BMP_IMAGE_FILE,
			TEXT_FILE,
		};

	public :
		void init();
		void setFileName(char* fileName);
		FILE* getFilePointer() { return _pFile; }

		virtual int32 openFile(char* fileName, int32 openType) = 0;
		virtual int32 readFile(uint8* buffer, int32 size) = 0;
		
		
	};	// class File

	class RawFile : public File {
	public :
		RawFile();
		~RawFile();

	private :
	public :
		int32 openFile();
	    int32 openFile(char* fileName, int32 openType);
		int32 readFile(uint8* buffer, int32 size);
		int32 writeFile(uint8* buffer, int32 size);
	}; // class RawFile
}	// namespace SoDA

#endif // _SODA_FILE_H_