#include <SoDA_S2String.h>
#include <SoDA_S2Exception.h>
#include <SoDA_S2Archive.h>
#include <SoDA_S2TreedArchive.h>
#include <SoDA_S2Stream.h>

#include <locale>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cstring>

#include <stdio.h>

using namespace std;
namespace Soda {
    void _s2_wstring_vformat(std::wstring* result, const std::wstring& format, va_list args)
    {
        static const std::wstring types(L"@diouxXDOUeEfFgGaACcSspn");

        result->clear();

        std::wstring tf;
        const uint32 TS_N = 256;
        wchar_t ts[TS_N];
        boost::scoped_array<wchar_t> tss;
        std::wstring::const_iterator si = format.begin();
        while (true) {
            i = std::find(i, format.end(), L'%');
            if (i == format.end()) {
                result->append(si, format.end());

                break;
            }

            if (*(i + 1) == L'%') {
                result->append(si, i + 1);
                i += 2;
                si = i;

                continue;
            }

            t = std::find_first_of(i, format.end(), types.begin(), types.end());
            if (t == format.end()) {
                std::wcerr << __FUNCTION__ << L"(): Unknown type " << "\"%" << *(i + 1) << "\"" << std::endl;
                abort();
                
                return;
            }

            result->append(si, i);

            if (*t == L'@') {
                S2Object* object = av_arg(args, S2Object*);
                if (object) {
                    result->append(object->toString().to_wstrint());
                }
                else {
                    result->append(L"(null)");
                }
            }
            else {
                tf.assign(i, t + 1);
                int32 dist = static_cast<int32>(std::distance(i, t));
                if (*t == L'd' || *t == L'i' || *t == L'o' ||
                    *t == L'u' || *t == L'x' || *t == L'X') {
                    if (4 <= dist && *(t - 3) == 'I' && *(t - 2) == '6' && *(t - 1) == '4') {
                    #if !defined(_WIN32)
                        tf.replace(tf.length() - 4, 3, L"ll");
                    #endif
                        _snwprintf(ts, TS_N, tf.c_str(), va_arg(args, int64);
                    }
                    else if (3 <= dist && *(t - 2) == L'l' && *(t - 1) == L'l') {
                    #if defined(_WIN32)
                        tf.replace(tf.length() - 3, 2, L"I64");
                    #endif
                        _snwprintf(ts, TS_N, tf.c_str(), va_arg(args, int64));
                    }
                    else if (2 <= dist && *(t - 1) == L'l') {   // "%..ld"
                        _snwprintf(ts, TS_N, tf.c_str(), va_arg(args, long));
                    }
                    else {                                      // "%..d"
                        _snwprintf(ts, TS_N, tf.c_str(), va_arg(args, int));
                    }
                }
                else if (*t == L'e' || *t == L'E' || *t == L'f' || *t == L'F' ||
                         *t == L'g' || *t == L'G' || *t == L'a' || *t == L'A') {
                    _snwprintf(ts, TS_N, tf.c_str(), va_arg(args, double));
                }
                else if (*t == L'c') {
                #if defined(_WIN32)
                    tf[tf.length() - 1] = L'C';
                #else
                    tf[tf.length() - 1] = L'c';
                #endif
                    _snwprintf(ts, TS_N, tf.c_str(), va_arg(args, int));
                }
                else if (*t == L'C') {
                #if defined(_WIN32)
                    tf[tf.length() - 1] = L'c';
                #else
                    tf[tf.length() - 1] = L'C';
                #endif
                    // wchar_t는 wint_t로 읽어와야 한다.
                    _snwprintf(ts, TS_N, tf.c_str(), va_arg(args, wint_t));
                }
                else if (*t == L's') {
                    // S2에서 "%s"는 항상 char* 이다. (*nix와 동일)
                    // Win32에서 wprintf로 char*를 쓸 때는 "%S"를 사용해야 한다.
                #if defined(_WIN32)
                    tf[tf.length() - 1] = L'S';
                #else
                    tf[tf.length() - 1] = L's';
                #endif
                    char* s = va_arg(args, char*);
                    size_t tssn = std::strlen(s) + 1;
                    if (TS_N < tssn) {
                        tss.reset(new wchar_t[tssn]);
                        _snwprintf(tss.get(), tssn, tf.c_str(), s);
                    }
                    else {
                        _snwprintf(ts, TS_N, tf.c_str(), s);
                    }
                }
                else if (*t == L'S') {
                #if defined(_WIN32)
                    tf[tf.length() - 1] = L's';
                #else
                    tf[tf.length() - 1] = L'S';
                #endif
                    wchar_t* ws = va_arg(args, wchar_t*);
                    size_t tssn = std::wcslen(ws) + 1;
                    if (TS_N < tssn) {
                        tss.reset(new wchar_t[tssn]);
                        _snwprintf(tss.get(), tssn, tf.c_str(), ws);
                    }
                    else {
                        _snwprintf(ts, TS_N, tf.c_str(), ws);
                    }
                }
                else if (*t == L'p') {
                    _snwprintf(ts, TS_N, tf.c_str(), va_arg(args, void*));
                }
                if (tss.get()) {
                    result->append(tss.get());
                    tss.reset();
                }
                else {
                    ts[TS_N - 1] = L'\0';
                    result->append(ts);
                }
            }
            i = t + 1;
            si = i;
        }
    }
}   // namespace SoDA

using namespace SoDA;

void S2String::format(const S2String* format, ...)
{
    va_list args;
    va_start(args, format);
    _s2_wstring_vformat(&_wstring, format->_wstring, args);
    va_end(args);
}


void S2String::vformat(const wchar_t* format, va_list args)
{
    _s2_wstring_vformat(&_wstring, format, args);
}


void S2String::vformat(const S2String* format, va_list args)
{
    _s2_wstring_vformat(&_wstring, format->_wstring, args);
}

void S2String::appendf(const wchar_t* format, ...)
{
    va_list args;
    va_start(args, format);
    std::wstring ws;
    _s2_wstring_vformat(&ws, format, args);
    va_end(args);

    _wstring += ws;
}

void S2String::appendf(const S2String* format)
{
    va_list args;
    va_start(args, format);
    std::wstring ws;
    _s2_wstring_vformat(&ws, format->_wstring, args);
    va_end(args);

    _wstring += ws;
}


void S2String::appendvf(const wchar_t* format, va_list args)
{
    std::wstring ws;
    _s2_wstring_vformat(&ws, format, args);

    _wstring += ws;
}

void S2String::appendvf(const S2String* format, va_list args)
{
    std::wstring ws;
    _s2_wstring_vformat(&ws, format->_wstring, args);

    _wstring += ws;
}

void S2String::makeUpper(const std::locale& loc)
{
    std::wstring r;
    for (int i = 0; i < _wstring.size(); i++) {
        r.push_back(std::toupper<wchar_t>(_wstring[i], loc));
    }
    _wstring.swap(r);
}

void S2String::makeLower(const std::locale& loc)
{
    std::wstring r;
    for (int i = 0; i < _wstring.size(); i++) {
        r.push_back(std::tolower<wchar_t>(_wstring[i], loc));
    }
    _wstring.swap(r);
}

void S2String::trimLeft()
{
    for (std::wstring::iterator i = _wstring.begin(); ; ++i) {
        if (i == _wstring.end()) {
            _wstring.clear();
            break;
        }
        else if (!std::isspace(*i, defaultLocale())) {
            _wstring.erase(_wstring.begin(), i);
            break;
        }
    }
}

void S2String::trimRight()
{
    for (std::wstring::iterator i = _wstring.end() - 1; ; --i) {
        if (!std::isspace(*i,  defaultLocale())) {
            _wstring.erase(i + 1, _wstring.end());
            break;
        }
        else if (i == _wstring.begin()) {
            _wstring.clear();
            break;
        }
    }
}

void S2String::adjustToBytes(uint32 bytes)
{
    if (0 == bytes) {
        _wstring = L"";
    }
    else {
        std::string s = to_string();
        if (bytes < s.length()) {
            while (bytes < s.length()) {
                _wstring.resize(_wstring.length() - 1);
                s = to_string();
            }
            from_string(s);
        }
    }
}

#define THROW_RANGE_EXCEPTION(domain, index)                        \
    S2RangeException::Info info(0, static_cast<int32>(_wstring.length()) - 1, index);   \
    throw S2RangeException(domain, info)

#if !defined(_WIN32)
const uint32 S2String::NPOS;
#endif

const wchar_t& S2String::at(uint32 index) const
{
    if (_wstring.length() <= index) {
        THROW_RANGE_EXCEPTION(L"S2String::at()", index);
    }
    return _wstring[index];
}

int32 S2String::compare(uint32 index, uint32 length, const S2String& s) const
{
    if (_wstring.length() <= index) {
        THROW_RANGE_EXCEPTION(L"S2String::compare()", index);
    }
    return _wstring.compare(index, length, s._wstring);
}

bool S2String::caseInsensitiveCompare(const S2String& s) const
{
    struct lt_str_2 : public std::binary_function<std::wstring, std::wstring, bool> {
        bool operator()(const std::wstring& x, const std::wstring& y, const std::locale& loc) const
        {
            return std::lexicographical_compare(x.begin(),
                                                x.end(),
                                                y.begin(),
                                                y.end(),
                                                boost::algorithm::is_iequal(loc));
        }
    };

    lt_str_2 comp;

    return comp(_wstring, s._wstring, defaultLocale());
}

uint32 S2String::find(const S2String& s, uint32 index) const
{
    std::wstring::size_type i = _wstring.find(s._wstring, index);
    return (i == std::wstring::npos) ? S2String::NPOS : static_cast<uint32>(i);
}

uint32 S2String::findFirstOf(const S2String& s, uint32 index) const
{
    std::wstring::size_type i = _wstring.find_first_of(s._wstring, index);
    return (i == std::wstring::npos) ? S2String::NPOS : static_cast<uint32>(i);
}

uint32 S2String::findFirstNotOf(const S2String& s, uint32 index) const
{
    std::wstring::size_type i = _wstring.find_first_not_of(s._wstring, index);
    return (i == std::wstring::npos) ? S2String::NPOS : static_cast<uint32>(i);
}

uint32 S2String::findLastOf(const S2String& s, uint32 index) const
{
    std::wstring::size_type i = _wstring.find_last_of(s._wstring, index);
    return (i == std::wstring::npos) ? S2String::NPOS : static_cast<uint32>(i);
}

uint32 S2String::findLastNotOf(const S2String& s, uint32 index) const
{
    std::wstring::size_type i = _wstring.find_last_not_of(s._wstring, index);
    return (i == std::wstring::npos) ? S2String::NPOS : static_cast<uint32>(i);
}

uint32 S2String::caseInsensitiveFind(const S2String& s) const
{
    std::wstring oriUpper;
    for (int32 i = 0; i < _wstring.size(); i++) {
        oriUpper.push_back(std::toupper<wchar_t>(_wstring[i], defaultLocale()));
    }
    std::wstring sUpper;
    for (int32 i = 0; i < s.length(); i++) {
        sUpper.push_back(std::toupper<wchar_t>(s[i], defaultLocale()));
    }
    std::wstring::size_type i = oriUpper.find_first_of(sUpper, 0);
    return (i == std::wstring::npos) ? S2String::NPOS : static_cast<uint32>(i);
}

uint32 S2String::reverseFind(const S2String& s, uint32 index) const
{
    std::wstring::size_type pos = (index == S2String::NPOS) ?
        std::wstring::npos : static_cast<std::wstring::size_type>(index);
    std::wstring::size_type i = _wstring.rfind(s._wstring, pos);
    return (i == std::wstring::npos) ? S2String::NPOS : static_cast<uint32>(i);
}

S2String& S2String::replace(uint32 index, uint32 length, const S2String& s)
{
    if (_wstring.length() <= index) {
        THROW_RANGE_EXCEPTION(L"S2String::replace()", index);
    }
    _wstring.replace(index, length, s._wstring);
    return *this;
}

S2String& S2String::replace(const S2String& src, const S2String& dst, bool all)
{
    uint32 srcLength = src.length();
    uint32 dstLength = dst.length();
    uint32 i = 0;
    do {
        i = find(src, i);
        if (i == NPOS) {
            break;
        }
        _wstring.replace(i, srcLength, dst._wstring);
        i += dstLength;
    } while (all);
    return *this;
}

S2String& S2String::insert(uint32 index, const S2String& s)
{
    if (_wstring.length() <= index) {
        THROW_RANGE_EXCEPTION(L"S2String::insert()", index);
    }
    _wstring.insert(index, s._wstring);
    return *this;
}

S2String S2String::substring(uint32 index, uint32 length) const
{
    if (_wstring.length() <= index) {
        THROW_RANGE_EXCEPTION(L"S2String::substring()", index);
    }
    else if (length != NPOS && _wstring.length() < index + length) {
        THROW_RANGE_EXCEPTION(L"S2String::substring()", index + length);
    }
    return _wstring.substr(index, length).c_str();
}

void S2String::from_string(const std::string& cs, const std::locale& loc)
{
    typedef std::codecvt<wchar_t, char, std::mbstate_t> codecvt_t;

    const codecvt_t& codecvt = std::use_facet<codecvt_t>(loc);
    std::mbstate_t state = std::mbstate_t();
    std::vector<wchar_t> buffer(cs.length() + 1);
    const char* in = cs.c_str();
    wchar_t* out = &buffer[0];
    if (std::codecvt_base::error ==
        codecvt.in(state,
                   cs.c_str(), cs.c_str() + cs.length(), in,
                   &buffer[0], &buffer[0] + buffer.size(), out)) {
        buffer[0] = L'\0';
    }
    _wstring = &buffer[0];
}

void S2String::from_string(const std::string& cs, const std::string& charset)
{
}

std::string S2String::to_string(const std::locale& loc) const
{
    typedef std::codecvt<wchar_t, char, std::mbstate_t> codecvt_t;

    const std::wstring& ws = _wstring;
    codecvt_t const& codecvt = std::use_facet<codecvt_t>(loc);
    std::mbstate_t state = std::mbstate_t();
    std::vector<char> buffer((ws.length() + 1) * codecvt.max_length());
    const wchar_t* in = ws.c_str();
    char* out = &buffer[0];
    if (std::codecvt_base::error ==
        codecvt.out(state,
                    ws.c_str(), ws.c_str() + ws.length(), in,
                    &buffer[0], &buffer[0] + buffer.size(), out)) {
        buffer[0] = '\0';
    }
    return std::string(&buffer[0]);
}

std::string S2String::to_string(const std::string& charset) const
{
    return "";
}

int8 S2String::toInt8() const
{
    int value = 0;
    try {
        value = std::stoi(_wstring);
        if (value > 127 || value < -128) {
            throw S2BadCastException(L"S2String::toInt8()");
        }
    }
    catch (std::invalid_argument&) {
        throw S2BadCastException(L"S2String::toInt8()");
    }
    catch (std::out_of_range&) {
        throw S2BadCastException(L"S2String::toInt8()");
    }
    return value;
}

uint8 S2String::toUint8() const
{
    unsigned long value = 0;
    try {
        value = std::stoul(_wstring);
        if (value > 255) {
            throw S2BadCastException(L"S2String::toUInt8()");
        }
    }
    catch (std::invalid_argument&) {
        throw S2BadCastException(L"S2String::toUInt8()");
    }
    catch (std::out_of_range&) {
        throw S2BadCastException(L"S2String::toUInt8()");
    }
    return value;
}

int16 S2String::toInt16() const
{
#if defined(_IDIS_S2_USE_BOOST)
    try {
        return boost::lexical_cast<int16>(_wstring);
    }
    catch (boost::bad_lexical_cast&) {
        throw S2BadCastException(L"S2String::toInt16()");
    }
#else
    int value = 0;
    try {
        value = std::stoi(_wstring);
        if (value > 32767 || value < -32768) {
            throw S2BadCastException(L"S2String::toInt16()");
        }
    }
    catch (std::invalid_argument&) {
        throw S2BadCastException(L"S2String::toInt16()");
    }
    catch (std::out_of_range&) {
        throw S2BadCastException(L"S2String::toInt16()");
    }
    return value;
#endif
}

uint16 S2String::toUint16() const
{
#if defined(_IDIS_S2_USE_BOOST)
    try {
        return boost::lexical_cast<uint16>(_wstring);
    }
    catch (boost::bad_lexical_cast&) {
        throw S2BadCastException(L"S2String::toUint16()");
    }
#else
    unsigned long value = 0;
    try {
        value = std::stoul(_wstring);
        if (value > 65535) {
            throw S2BadCastException(L"S2String::toUInt16()");
        }
    }
    catch (std::invalid_argument&) {
        throw S2BadCastException(L"S2String::toUInt16()");
    }
    catch (std::out_of_range&) {
        throw S2BadCastException(L"S2String::toInt16()");
    }
    return value;
#endif
}

int32 S2String::toInt32() const
{
#if defined(_IDIS_S2_USE_BOOST)
    try {
        return boost::lexical_cast<int32>(_wstring);
    }
    catch (boost::bad_lexical_cast&) {
        throw S2BadCastException(L"S2String::toInt32()");
    }
#else
    int value = 0;
    try {
        value = std::stoi(_wstring);
    }
    catch (std::invalid_argument&) {
        throw S2BadCastException(L"S2String::toInt32()");
    }
    catch (std::out_of_range&) {
        throw S2BadCastException(L"S2String::toInt32()");
    }
    return value;
#endif
}

uint32 S2String::toUint32() const
{
#if defined(_IDIS_S2_USE_BOOST)
    try {
        return boost::lexical_cast<uint32>(_wstring);
    }
    catch (boost::bad_lexical_cast&) {
        throw S2BadCastException(L"S2String::toUint32()");
    }
#else
    unsigned long value = 0;
    try {
        value = std::stoul(_wstring);
    }
    catch (std::invalid_argument&) {
        throw S2BadCastException(L"S2String::toUInt32()");
    }
    catch (std::out_of_range&) {
        throw S2BadCastException(L"S2String::toUInt32()");
    }
    return value;
#endif
}

int64 S2String::toInt64() const
{
#if defined(_IDIS_S2_USE_BOOST)
    try {
        return boost::lexical_cast<int64>(_wstring);
    }
    catch (boost::bad_lexical_cast&) {
        throw S2BadCastException(L"S2String::toInt64()");
    }
#else
    int64 value = 0;
    try {
        value = std::stoll(_wstring);
    }
    catch (std::invalid_argument&) {
        throw S2BadCastException(L"S2String::toInt64()");
    }
    catch (std::out_of_range&) {
        throw S2BadCastException(L"S2String::toInt64()");
    }
    return value;
#endif
}

uint64 S2String::toUint64() const
{
#if defined(_IDIS_S2_USE_BOOST)
    try {
        return boost::lexical_cast<uint64>(_wstring);
    }
    catch (boost::bad_lexical_cast&) {
        throw S2BadCastException(L"S2String::toUint64()");
    }
#else
    uint64 value = 0;
    try {
        value = std::stoull(_wstring);
    }
    catch (std::invalid_argument&) {
        throw S2BadCastException(L"S2String::toUInt64()");
    }
    catch (std::out_of_range&) {
        throw S2BadCastException(L"S2String::toUInt64()");
    }
    return value;
#endif
}

float32 S2String::toFloat32() const
{
#if defined(_IDIS_S2_USE_BOOST)
    try {
        return boost::lexical_cast<float32>(_wstring);
    }
    catch (boost::bad_lexical_cast&) {
        throw S2BadCastException(L"S2String::toFloat32()");
    }
#else
    float32 value = 0;
    try {
        value = std::stof(_wstring);
    }
    catch (std::invalid_argument&) {
        throw S2BadCastException(L"S2String::toFloat32()");
    }
    catch (std::out_of_range&) {
        throw S2BadCastException(L"S2String::toFloat32()");
    }
    return value;
#endif
}

float64 S2String::toFloat64() const
{
#if defined(_IDIS_S2_USE_BOOST)
    try {
        return boost::lexical_cast<float64>(_wstring);
    }
    catch (boost::bad_lexical_cast&) {
        throw S2BadCastException(L"S2String::toFloat64()");
    }
#else
    float64 value = 0;
    try {
        value = std::stod(_wstring);
    }
    catch (std::invalid_argument&) {
        throw S2BadCastException(L"S2String::toFloat64()");
    }
    catch (std::out_of_range&) {
        throw S2BadCastException(L"S2String::toFloat64()");
    }
    return value;
#endif
}

const uint8 ARCHIVE_ID_LENGTH   = 1;
const uint8 ARCHIVE_ID_STRING   = 2;

void S2String::archiveTo(S2Archive* archive) const
{
    if (S2TreedArchive* treedArchive = dynamic_cast<S2TreedArchive*>(archive)) {
        treedArchive->writeS2String(ARCHIVE_ID_STRING, *this);
    }
    else {
        std::string s = to_string(STD_LOCALE_UTF8);
        uint16 length = static_cast<uint16>(s.length());
        archive->writeUint16(ARCHIVE_ID_LENGTH, length);
        if (0 < length) {
            archive->writeBinary(ARCHIVE_ID_STRING, s.c_str(), length * sizeof(char));
        }
    }
}

void S2String::unarchiveFrom(S2Archive* archive)
{
    if (S2TreedArchive* treedArchive = dynamic_cast<S2TreedArchive*>(archive)) {
        from_wstring(treedArchive->readS2String().to_wstring());
    }
    else {
        uint16 length;
        archive->readUint16(ARCHIVE_ID_LENGTH, &length);
        _wstring.clear();
        if (0 < length) {
            std::vector<char> v(length + 1, '\0');
            archive->readBinary(ARCHIVE_ID_STRING, &v[0], length * sizeof(char));

            from_string(&v[0], STD_LOCALE_UTF8);
        }
    }
}

void S2String::serializeTo(S2Stream* stream) const
{
    std::string s = to_string(STD_LOCALE_UTF8);
    uint16 length = static_cast<uint16>(s.length());
    stream->writeUint16(length);
    if (0 < length) {
        stream->write(s.c_str(), length * sizeof(char));
    }
}

void S2String::deserializeFrom(S2Stream* stream)
{
    uint16 length;
    stream->readUint16(&length);
    _wstring.clear();
    if (0 < length) {
        std::vector<char> v(length + 1, '\0');
        stream->read(&v[0], length * sizeof(char));

        from_string(&v[0], STD_LOCALE_UTF8);
    }
}

