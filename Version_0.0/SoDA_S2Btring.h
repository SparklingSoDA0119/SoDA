#ifndef _SODA_STRING_H
#define _SODA_STRING_H

#include "G:\ProjectHS\SoDA\Version_0.0\SoDA_Type.h"

#if defined(_WIN32)
	#define vsnprintf	_vsnprintf
#endif


namespace SoDA {
	template<sInt BUFFER_SIZE>

	class String {
	public :
		String() { makeEmpty(); }
		String(const char* s) { operator=(s); }

	private :
		char _buffer[BUFFER_SIZE];

	public :
		usInt length()  const { return static_cast<usInt>(strlen(_buffer)); }
		bool isEmpty() const { return ('\0' == _buffer[0]); }
		
		bool operator==(const char* s)				  const { return !strcmp(_buffer, s); }
		bool operator==(const String<BUFFER_SIZE>& s) const { return operator==(s._buffer); }
		bool operator!=(const char* s)				  const { return !operator==(s); }
		bool operator!=(const String<BUFFER_SIZE>& s) const { return !operator==(s); }

		const char* operator= (const char* s);
		const char* operator= (const String<BUFFER_SIZE>& s) { return operator=(s._buffer); }
		const char* operator+=(const char* s)				 { return append(s); }
		const char* operator+=(const String<BUFFER_SIZE>& s) { return operator+=(s._buffer); }

		const char* append(const char* s);
		const char* append(const String<BUFFER_SIZE>& s) { return operator+=(s._buffer); }
		const char* printf(const char* fmt, ...);
		const char* appendf(const char* fmt, ...);

		void makeEmpty() { _buffer[0] = '\0'; }
		void trimLeft ();
		void trimRight();

		operator char*() { return _buffer; }
		
		const char* cString() const { return _buffer; }
	};		// String

	template<sInt BUFFER_SIZE>
	const char* String<BUFFER_SIZE>::operator=(const char* s)
	{
		strncpy(_buffer, s, BUFFER_SIZE - 1);
		_buffer[BUIFFER_SIZE - 1] = '\0';

		return _buffer;
	}

	template<sInt BUFFER_SIZE>
	const char* String<BUFFER_SIZE>::append(const char* s)
	{
		uint32 len = length();
		if (len < BUFFER_SIZE - 1) {
			strncpy(_buffer + len, s, BUFFER_SIZE - len - 1);
			_buffer[BUFFER_SIZE - 1] = '\0';
		}

		return _buffer;
	}

	template<sInt BUFFER_SIZE>
	const char* String<BUFFER_SIZE>::printf(const char* fmt, ...)
	{
		va_list vargs;
		va_start(vargs, fmt);
		vsnprintf(_buffer, BUFFER_SIZE, fmt, vargs);
		_buffer[BUFFER_SIZE - 1] = '\0';
		va_end(vargs);

		return _buffer;
	}

	template<sInt BUFFER_SIZE>
	const char* String<BUFFER_SIZE>::appendf(const char* fmt, ...)
	{
		usInt len = static_cast<usInt>(strlen(_buffer));
		if (len < BUFFER_SIZE - 1) {
			va_list vargs;
			va_start(vargs, fmt);
			vsnprintf(_buffer + len, BUFFER_SIZE - len, fmt, vargs);
			_buffer[BUFFER_SIZE -1] = '\0';
			va_end(vargs);
		}

		return _buffer;
	}

	template<sInt BUFFER_SIZE>
	void String<BUFFER_SIZE>::trimLeft()
	{
		const char* = _buffer;
		while ((*s != '\0') && isspace(*s)) {
			s++;
		}

		if (s != _buffer) {
			uint32 len = strlen(_buffer);
			memmove(_buffer, s, len - (s - _buffer) + 1);
		}
	}

	template<sInt BUFFER_SIZE>
	void String<BUFFER_SIZE>::trimRight()
	{
		char* s = _buffer + strlen(_buffer) - 1;
		while ((_buffer < s) && isspace(*s)) {
			s--;
		}
		*(s + 1) = '\0';
	}

	typedef String< 16> String16;
	typedef String< 32> String32;
	typedef String< 64> String64;
	typedef String<128> String128;
	typedef String<256> String256;
	typedef String<512> String512;
}	//	namespace SoDA

#endif	// _SODA_STRING_H