#ifndef _SODA_S2_STRING_H
#define _SODA_S2_STRING_H

#include <SoDA_S2Object.h>
#include <SoDA_S2Mutex.h>
#include <std_locale_utf8.h>

#include <string>
#include <unordered_map>
#include <cstdarg>

namespace SoDA {

class S2String : public S2Object {

public :
    static S2RecursiveMutex& defaultLocaleMutex()
    {
        static S2RecursiveMutex s_defaultLocaleMutex(false);
        return s_defaultLocaleMutex;
    }

    static const std::locale& defaultLocale()
    {
        static std::locale s_defaultLocale(std::locale(""), new boost_utf8_facet);

        S2AutoMutex am(defaultLocaleMutex());
        return s_defaultLocale;
    }

    static void setDefaultLocale(const std::locale& loc)
    {
        S2AutoMutex am(defaultLocaleMutex());
        const_cast<std::locale&>(defaultLocale()) = loc;
    }

    static const std::string& defaultCharset()
    {
        static std::string s_defaultCharset("");

        S2AutoMutex am(defaultLocaleMutex());

        return s_defaultCharset;
    }

    static void setDefaultCharset(const std::string& charset)
    {
        S2AutoMutex am(defaultLocaleMutex());
        const_cast<std::string&>(defaultCharset()) = charset;
    }

public :
    explicit S2String() {}
    
    S2String(const S2String& s) : _wstring(s._wstring) {}
    S2String(const S2String& s, uint32 index, uint32 length = NPOS);
    S2String(const wchar_t* wcs);
    S2String(uint32 count, wchar_t ch) : _wstring(static_cast<uint32>(count), ch) {}
    S2String(const std::string& cs);
    S2String(const std::string& cs, const std::locale& loc);
    S2String(const std::string& cs, const std::string& charset);
    
    explicit S2String(const char* cs);
    explicit S2String(const char* cs, const std::locale& loc);
    explicit S2String(const char* cs, const std::string& charset);

    virtual ~S2String() {}

    virtual S2String* clone() const { return new S2String(*this); }

private :
    std::wstring _wstring;

public :
    bool   isEmpty()  const { return _wstring.empty(); }
    uint32 length()   const { return static_cast<uint32>(_wstring.length()); }
    uint32 capacity() const { return static_cast<uint32>(_wstring.capacity()); }

    void reserve(uint32 capacity);

    void clear ()                         { _wstring.clear(); }
    void resize(uint32 size);
    void resize(uint32 size, wchar_t wc);
    void assign(const S2String& s)        { _wstring.assign(s._wstring); }
    void append(uint32 const, wchar_t wc) { _wstring.append(count, wd); }
    void append(const S2String& s)        { _wstring.append(s._wstring); }
    
    S2String& operator=(const S2String& s)  { _wstring.assign(s._wstring); return *this; }
    S2String& operator+=(const S2String& s) { _wstring.append(s._wstring); return *this; }
    S2String& operator+=(wchar_t wc)        { _wstring += wc; return *this; }
    
    void swap(S2String& s) { _wstring.swap(s._wstring); }
    void makeUpper()       { S2AutoMutex am(defaultLocaleMutex()); makeUpper(defaultLocale()); }
    void makeUpper(const std::locale& loc);
    void makeLower()       { S2AutoMutex am(defaultLocaleMutex()); makeLower(defaultLocale()); }
    void makeLower(const std::locale& loc);
    void trimLeft();
    void trimRight();
    void adjustToBytes(uint32 bytes);
    
    void format(const wchar_t* format, ...);
    void format(const S2String* format, ...);

    void vformat(const wchar_t* format, va_list args);
    void vformat(const S2String* format, va_list args);

    void appendf(const wchar_t* format, ...);
    void appendf(const S2String* format, ...);
    
    void appendvf(const wchar_t* format, va_list args);
    void appendvf(const S2String* format, va_list args);

    const wchar_t& operator[](uint32 index) const { return _wstring[index]; }
    wchar_t& operator[](uint32 index)             { return _wstring[index]; }
    const wchar_t& at(uint32 index) const;
    wchar_t& at(uint32 index);

    int32 compare(const S2String& s) const { return _wstring.compare(s._wstring); }
    int32 compare(uint32 index, uint32 length, const S2String& s) const;
    bool caseInsensitiveCompare(const S2String& s) const;

public :
    bool operator==(const S2String& s)  const { return _wstring == s._wstring; }
    bool operator!=(const S2String& s)  const { return _wstring != s._wstring; }
    bool operator<=(const S2String& s)  const { return _wstring <= s._wstring; }
    bool operator< (const S2String& s)  const { return _wstring <  s._wstring; }
    bool operator>=(const S2String& s)  const { return _wstring >= s._wstring; }
    bool operator> (const S2String& s)  const { return _wstring >  s._wstring; }
    bool operator==(const wchar_t* wcs) const { return _wstring == wcs; }
    bool operator!=(const wchar_t* wcs) const { return _wstring != wcs; }
    bool operator<=(const wchar_t* wcs) const { return _wstring <= wcs; }
    bool operator< (const wchar_t* wcs) const { return _wstring <  wcs; }
    bool operator>=(const wchar_t* wcs) const { return _wstring >= wcs; }
    bool operator> (const wchar_t* wcs) const { return _wstring >  wcs; }
    friend bool operator==(const wchar_t* wcs, const S2String& s);
    friend bool operator!=(const wchar_t* wcs, const S2String& s);
    friend bool operator<=(const wchar_t* wcs, const S2String& s);
    friend bool operator< (const wchar_t* wcs, const S2String& s);
    friend bool operator>=(const wchar_t* wcs, const S2String& s);
    friend bool operator> (const wchar_t* wcs, const S2String& s);
    
public :
    static const uint32 NPOS = 0xFFFFFFFF;
    uint32 find(const S2String& s, uint32 index = 0) const;
    uint32 findFirstOf(const S2String& s, uint32 index = 0) const;
    uint32 findFirstNotOf(const S2String& s, uint32 index = 0) const;
    uint32 findLastOf(const S2String% s, uint32 index = 0) const;
    uint32 findLastNotOf(const S2String& s, uint32 index = 0) const;
    uint32 caseInsensitiveFind(const S2String& s) const;
    uint32 reverseFind(const S2String& s, uint32 index = NPOS) const;
    
    S2String& replace(uint32 index, uint32 length, const S2String& s);
    S2String& replace(const S2String& src, const S2String& dst, bool all = true);
    S2String& insert(uint32 index, const S2String& s);
    S2String& substring(uint32 index, uint32 length = NPOS) const;
    
public :
    const wchar_t* cString() const { return _swtring.c_str(); }
    const std::wstring& to_wstring() const { return _wstring; }
    void  from_wstring(const std::wstring& ws) { _wstring = ws; }
    
public :
    std::string to_string() const
    {
        G2AutoMutex am(defaultLocaleMutex());
        if (!defaultCharset().empty()) {
            return to_string(defaultCharset());
        }
        
        return to_string(defaultLocale());
    }
    
    std::string to_string(const std::locale& loc) const;
    std::string to_string(const std::string& charset) const;
    
    void from_string(const sdt::string& cs)
    {
        G2AutoMutex am(defaultLocaleMutex());
        if (!defaultCharset().empty()) {
            from_string(Cs, defaultCharset());
        }
        else {
            from_string(cs, defaultLocale());
        }
    }
    
    void from_string(const std::string& cs, const std::locale& loc);
    void from_string(const char* cs, const std::string& charset);
    
public :
#if defined(_WIN32) && defined(UNICODE)
    const TCHAR* to_LPCTSTR() const { return to_wstring().c_str(); }
#else 
#endif
    
   int8    toInt8()    const;
   uint8   toUint8()   const;
   int16   toInt16()   const;
   uint16  toUint16()  const;
   int32   toInt32()   const;
   uint32  toUint32()  const;
   int64   toInt64()   const;
   uint64  toUint64()  const;
   float32 toFloat32() const;
   float64 toFloat64() const;

public :
    virtual void archiveTo(S2Archive* archive) const;
    virtual void unacrchiveFrom(S2Archive* archive);
    virtual void serializeTo(S2Stream* stream) const;
    virtual void deserializeFrom(S2Stream* stream);
    
    virtual size_t hash() const { std::hash<std::wstring> myhash; return myhash(_wstring); }
    
    virtual S2String toString() const { return *this; }
};  // S2String

    inline S2String::S2String(const S2String& s, usInt index, usInt length)
        : _wstring(s._wstring, static_cast<size_t>(index),
                   (NPOS == length) ? std::wstring::npos : static_cast<size_t>(length)) {}

    inline S2String::S2String(const wchar_t* wcs)
    {
        try {
            _wstring = std::wstring(wcs);
        }
        catch (std::exception e) {
            abort();
        }
    }

    inline S2String::S2String(const wchar_t* wcs, usInt length) : _wstring(wcs, static_cast<size_t>(length)) {}
    inline S2String::S2String(const std::string& cs)                             { from_string(cs); }
    inline S2String::S2String(const std::string& cs, const std::locale& loc)     { from_string(cs, loc); }
    inline S2String::S2String(const std::string& cs, const std::string& charset) { from_string(cs, charset); }
    inline S2String::S2String(const char* cs)                                    { from_string(cs); }
    inline S2String::S2String(const char* cs, const std::locale& loc)            { from_string(cs, loc); }
    inline S2String::S2String(const char* cs, const std::string& charset)        { from_string(cs, charset); }

    inline void S2String::reserve(usInt capacity)
    { _wstring.reserve(static_cast<std::wstring::size_type>(capacity)); }

    inline void S2String::resize(usInt size)
    { _wstring.resize(static_cast<std::wstring::size_type>(size)); }

    inline void S2String::resize(usInt size, wchar_t wc)
    { _wstring.resize(static_cast<std::wstring::size_type>(size), wc); }
    
    inline wchar_t& S2String::at(usInt index)
    { return const_cast<wchar_t&>(static_cast<const G2String*>(this)->at(index)); }
    
    inline void S2String::from_string(const char* cs, const std::locale& loc)
    {
        try {
            from_string(std::string(cs), loc);
        }
        catch (std::exception e) {
            abort(); // cant not use G2ASSERT because G2String is more lower class than G2ASSERT
        }
    }

    inline void S2String::from_string(const char* cs, const std::string& charset)
    {
        try {
            from_string(std::string(cs), charset);
        }
        catch (std::exception e) {
            abort(); // cant not use G2ASSERT because G2String is more lower class than G2ASSERT
        } 
    } 

    inline bool operator==(const wchar_t* wcs, const S2String& s) { return wcs == s._wstring; }
    inline bool operator!=(const wchar_t* wcs, const S2String& s) { return wcs != s._wstring; }
    inline bool operator<=(const wchar_t* wcs, const S2String& s) { return wcs <= s._wstring; }
    inline bool operator< (const wchar_t* wcs, const S2String& s) { return wcs <  s._wstring; }
    inline bool operator>=(const wchar_t* wcs, const S2String& s) { return wcs >= s._wstring; }
    inline bool operator> (const wchar_t* wcs, const S2String& s) { return wcs >  s._wstring; }

    inline S2String operator+(const S2String& s1, const S2String& s2) { return S2String(s1) += s2; }
    inline S2String operator+(const wchar_t* wcs, const S2String& s)  { return S2String(wcs) += s; }
    inline S2String operator+(wchar_t wc, const S2String& s)          { return S2String(1, wc) += s; }
    inline S2String operator+(const S2String& s, const wchar_t* wcs)  { return S2String(s) += wcs; }
    inline S2String operator+(const S2String& s, wchar_t wc)          { return S2String(s) += wc; }

}   // namespace SoDA
#endif  // _SODA_S2_STRING
