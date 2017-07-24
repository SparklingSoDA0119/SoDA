#ifndef _SODA_S2_ARCHIVE_H
#define _SODA_S2_ARCHIVE_H

#include <SoDA_S2Object.h>
#include <SoDA_S2Stream.h>
#include <SoDA_S2Exception.h>

#include <vector>
#include <cassert>

namespace SoDA {
class S2ArchiveDelegate;

class S2Archive : public S2Object {
public :
    S2Archive();
    S2Archive(const S2Archive& archive);
    virtual ~S2Archive() { close(); }

private :
    uint32 _streamBasePosition;
    uint8  _version;

    enum Type {
        TYPE_PATH = 0,
        TYPE_BOOL,      
        TYPE_INT8,       
        TYPE_UINT8,       
        TYPE_INT16,       
        TYPE_UINT16,       
        TYPE_INT32,       
        TYPE_UINT32,       
        TYPE_INT64,       
        TYPE_UINT64,       
        TYPE_FLOAT32,       
        TYPE_FLOAT64,       
        TYPE_BINARY,
    };

    enum Constant {
        MAX_OBJECT_ARRAY_ITEM = 255 * 254,
    };

protected :
    S2Stream* _stream;

protected :
    virtual void init(S2Stream* stream, bool create);

public :
    const S2Stream* stream() const { return _stream; }

    uint32 streamBasePosition() const { return _streamBasePosition; }
    void   create(S2Stream* stream)   { init(stream, true); _created = true; }
    void   open(S2Stream* stream)     { init(stream, false); }

    virtual void close() { _created = false; }

public :
    static const uint8 VERSION_1            = 1;
    static const uint8 CURRENT_VERSION      = VERSION_1;

protected :
#pragma pack(1)
    struct Item {
        uint32 position;
        uint8  key;
        uint8  type;
        uint32 next;
        uint32 size;

        bool isValid() const { return (key != INVALID_KEY); }
        void makeInvalid()   { key = INVALID_KEY; }

        void readFrom(S2Stream* stream);
        void writeTo (S2Stream* stream);

        static uint32 savedSize();

        virtual void init(uint8 key, uint8 type, uint32 size);
    }; // struct Item

#pragma pack()
protected :
    uint32 rootItemPosistion() const;
    void   readHeader(Item* rootItem);
    void   writeHeader(Item* rootItem);

public :
    static uint32 dataInfoSize();
    static uint32 arrayInfoSize();

private :
    virtual void _writeArrayCount(uint8 key, uint32 count) { writeUint32(key, cout); }
    virtual bool _readArrayCountObjectArray(uint8 key, uint32* count) const { return readUint32(KEY_ARRAY_COUNT, count); }
    virtual void _preProcessReadObjects(uint8 key) const {}
    virtual void _preProcessWriteObjects(uint8 key);
    virtual void _addPathForWriteObjects(uint8 key);
    virtual void _postProcessWriteObjects(uint8 key, const S2Stream& name = L"") {}

protected :
    mutable std::vector<Item> _pathItem;
    mutable Item _lastSubItem;

private :
    virtual void _pathInto(uint8 key) const;
    virtual void _pathOut() const;

public :
    std::vector<uint8> path() const;
    
    virtual void setPath(const std::vector<uint8>& path) const;
    virtual void setPath(uint8 key, ...) const;
    void pathInfo(uint8 key) const;
    void pathOut() const;

protected :
    static const uint8 INVALID_KEY          = 0;
    static const uint8 KEY_ARRAY_COUNT      = 1;
    static const uint8 KEY_ARRAY_DATA       = 2;

private :
    virtual void _addPath(uint8 key);

protected :
    virtual bool findItem(uint8 key, Item* item) const;
    virtual bool findItem(uint8 key, uint8 type, uint32 size) const;
    virtual void itemAdded(const Item& item);

public :
    virtual void   addPath(uint8 key);
    virtual bool   findItem(uint8 key, uint8* type = 0, uint32* size = 0) const;
    virtual uint32 readDataSize(uint8 key) const;
    virtual uint32 readArrayCount(uint8 key) const;
    virtual uint32 readArrayDataPosition(uint8 key) const;

public :
#define DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS(T, Name, ArrayName)                     \
    virtual bool read##Name(uint8 key, T* data) const;                              \
    virtual bool read##Array(uint8 key, T* array, uint32 i = 0, uint32 n = 0) const;\
    virtual void write##Name(uint8 key, T data, const S2Stream& name = L"");        \
    virtual void write##ArrayName(uint8 key, const T* array, uint32 count, const S2String& name = L"");

#define DEECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS_WITH_MANIPULATE(T, Name, ArrayName)    \
    DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS(T, Name, ArrayName)                         \
    virtual void reserve##ArrayName(uint8 key, uint32 count);                       \
    virtual void update##ArrayName(uint8 key, const T* array, uint32 count, uint32 offset);


    DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS_WITH_MANIPULATE(  bool,    Bool,    BoolArray)
    DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS_WITH_MANIPULATE(  int8,    Int8,    Int8Array)
    DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS_WITH_MANIPULATE( uint8,   Uint8,   Uint8Array)
    DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS(                  int16,   Int16,   Int16Array)
    DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS(                 uint16,  Uint16,  Uint16Array)
    DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS(                  int32,   Int32,   Int32Array)
    DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS(                 uint32,  Uint32,  Uint32Array)
    DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS(                  int64,   Int64,   Int64Array)
    DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS(                 uint64,  Uint64,  Uint64Array)
    DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS(                float32, Float32, Float32Array)
    DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS(                float64, Float64, Float64Array)

    virtual bool readBinary(uint8 key, void* data, uint32 size) const;
    virtual void writeBinary(uint8 key, const void* data, uint32 size, const S2String& name = L"");

    virtual bool readObject(uint8 key, S2Object* object) const;
    virtual void writeObject(uint8 key, const S2Object& object, const S2String& name = L"");

    template<typename T>
    bool readObjectArray(uint8 key, T* array, uint32 i = 0, uint32 n = 0) const
    { return readObjects(key, &array[0], i, n); }

    template<typename T>
    void writeObjectArray(uint8 key, const T* array, uint32 count, const S2String& name = L"")
    { writeObjects(key, &array[0], &array[count], name); }

    template<typename T>
    bool readObjectPtrArray(uint8 key, T* array, uint32 i = 0, uint32 n = 0) const
    { return readObjectsPtr(key, &array[0], i, n); }

    template<typename T>
    void writeObjectPtrArray(uint8 key, const T* array, uint32 count, const S2String& name = L"")
    { writeObjectsPtr(key, &array[0], &array[count], name); }

    template<typename OutputIterator>
    bool readObjects(uint8 key, OutputIterator itr, uint32 i = 0, uint32 n = 0) const;

    template<typename InputIterator>
    void writeObjects(uint8 key, InputIterator begin, InputIterator end, const S2String& name = L"");
    
    template<typename OutputIterator>
    bool readObjectsPtr(uint8 key, OutputIterator itr, uint32 i = 0, uint32 n = 0) const;
    
    template<typename InputIterator>
    void writeObjectsPtr(uint8 key, InputIterator begin, InputIterator end, const S2String& name = L"");

protected :
    void copyItemOnly(const S2Archive& archive, const Item& item);
    void copyItemAndSubItems(const S2Archive& archive, const Item& item);

public :
    virtual void copyFrom(const S2Archive& archive, uint8 key, bool copyPath = true);
    S2Archive& operator=(const S2Archive& archive);

protected :
    S2ArchiveDelegate* _delegate;

public :
    S2ArchiveDelegate* delegate() const { return _delegate; }
    void SetDelegate(S2ArchiveDelegate* delegate) { _delegate = delegate; }

private :
    bool _created;

public :
    bool created() const { return _created; }

    enum {
        EXCEPTION_INVALID_MAGIC = 1,
        EXCEPTION_INVALID_VERSION,
        EXCEPTION_INVALID_ARCHIVE,
        EXCEPTION_PATH_NOT_FOUND,
        EXCEPTION_ITEM_NOT_FOUND,
        EXCEPTION_TYPE_MISMATCH,
        EXCEPTION_SIZE_MISMATCH,
        EXCEPTION_PARSER_ERROR,
    };

public :
    class Scanner {
    public :
        Scanner(uint8* buffer = 0, uint32 bufferSize = 0) 
            : _depth(0), _buffer(buffer), _bufferSize(bufferSize) {}

        virtual ~Scanner() {}

    private :
        uint32 _depth;
        uint8* _buffer;
        uint32 _bufferSize;
        friend class S2Archive;
        friend class S2TrredArchive;

    public :
        uint32 depth() const { return _depth; }
        const uint8* buffer() const { return _buffer; }
        uint32 bufferSize() const { return _bufferSize; }
        virtual void begin() {}
        virtual void headerScanned(const S2Archive& archive) {}
        virtual void itemScanned(uint8 key, uint8 type, void* data) {}

        virtual void end() {}
    }; // class Scanner

    protected:
        void scanPath(Scanner* scanner) const;

    public:
        void scan(Scanner* scanner) const;
        virtual S2String toString() const;
}; // class S2Archive

template<typename OutputIterator>
bool S2Archive::readObjects(uint8 key, OutputIterator itr, uint32 i, uint32 n) const
{
    assert(key != INVALID_KEY);
    if (!findItem(key, TYPE_PATH, 0)) {
        return false;
    }

    uint32 count = 0;
    _pathInto(key);
    _readArrayCountObjectsArray(key, &count);

    if (n == 0) {       // 개수가 0이면,
        n = count - i;   // i부터 전체를 다 읽는다.
    }
    else {
        if (count <= i) {
            S2RangeException::Info info(0, count - 1, i);
            throw S2RangeException(L"S2Archive::readObjects", info);
        }
        if (count < i + n) {
            S2RangeException::Info info(1, count - i, n);
            throw S2RangeException(L"S2Archive::readObjects", info);
        }
    }

    if (count <= 256 - 2) {   // 0(INVALID_KEY)과 1(KEY_ARRAY_COUNT)은 제외.
        // i 앞은 건너뛰고 그 이후부터 n개의 객체를 읽는다.
        uint8 objKey = KEY_ARRAY_DATA + i;
        for (; 0 < n; n--, objKey++, ++itr) {
            readObject(objKey, &*itr);
        }
    }
    else {
        _preProcessReadObjects(key);

        // i 앞은 건너뛰고 그 이후부터 n개의 객체를 읽는다.
        uint8 objKey = 1 + i;
        for (uint8 subKey = KEY_ARRAY_DATA; 0 < n; subKey++, objKey = 1) {
            _pathInto(subKey);
            for (; 0 < n && 0 < objKey; n--, objKey++, ++itr) {
                readObject(objKey, &*itr);
            }
            _pathOut();
        }
    }
    _pathOut();
    return true;
}

template<typename InputIterator>
void S2Archive::writeObjects(uint8 key, InputIterator begin, InputIterator end, const S2String& name)
{
    assert(key != INVALID_KEY);
    if (!findItem(key, TYPE_PATH, 0)) {
        _preProcessWriteObjects(key);
    }
    _pathInto(key);

    uint32 count = static_cast<uint32>(std::distance(begin, end));
    _writeArrayCount(KEY_ARRAY_COUNT, count);

    if (count <= 256 - 2) {   // 0(INVALID_KEY)과 1(KEY_ARRAY_COUNT)은 제외.
        uint8 key = KEY_ARRAY_DATA;
        for (InputIterator itr = begin; itr != end; ++itr) {
            writeObject(key++, *itr, name);
        }
    }
    else {
        InputIterator itr = begin;
        for (uint8 subKey = KEY_ARRAY_DATA; itr != end; subKey++) {
            _addPathForWriteObjects(subKey);
            _pathInto(subKey);
            for (uint8 objKey = 1; 0 < objKey && itr != end; objKey++, ++itr) {
                writeObject(objKey, *itr, name);
            }
            _pathOut();
        }
    }
    _postProcessWriteObjects(key, name);
    _pathOut();
}

template<typename OutputIterator>
bool S2Archive::readObjectsPtr(uint8 key, OutputIterator itr, uint32 i, uint32 n) const
{
    assert(key != INVALID_KEY);
    if (!findItem(key, TYPE_PATH, 0)) {
        return false;
    }

    uint32 count = 0;
    _pathInto(key);
    _readArrayCountObjectsArray(key, &count);

    if (n == 0) {       // 개수가 0이면 
        n = count - i;  // i부터 전체를 다 읽는다.
    }
    else {
        if (count <= i) {
            S2RangeException::Info info(0, count - 1, i);
            throw S2RangeException(L"S2Archive::readObjects", info);
        }
        if (count < i + n) {
            S2RangeException::Info info(1, count - i, n);
            throw S2RangeException(L"S2Archive::readObjects", info);
        }
    }

    if (count <= 256 - 2) {   // 0(INVALID_KEY)과 1(KEY_ARRAY_COUNT)은 제외.
        // i 앞은 건너뛰고 그 이후부터 n개의 객체를 읽는다.
        uint8 objKey = KEY_ARRAY_DATA + i;
        for (; 0 < n; n--, objKey++, ++itr) {
            readObject(objKey, &**itr);
        }
    }
    else {
        _preProcessReadObjects(key);

        // i 앞은 건너뛰고 그 이후부터 n개의 객체를 읽는다.
        uint8 objKey = 1 + i;
        for (uint8 subKey = KEY_ARRAY_DATA; 0 < n; subKey++, objKey = 1) {
            _pathInto(subKey);
            for (; 0 < n && 0 < objKey; n--, objKey++, ++itr) {
                readObject(objKey, &**itr);
            }
            _pathOut();
        }
    }

    _pathOut();
    return true;
}

template<typename InputIterator>
void S2Archive::writeObjectsPtr(uint8 key, InputIterator begin, InputIterator end, const S2String& name)
{
    assert(key != INVALID_KEY);
    if (!findItem(key, TYPE_PATH, 0)) {
        _preProcessWriteObjects(key);
    }
    _pathInto(key);

    uint32 count = static_cast<uint32>(std::distance(begin, end));
    _writeArrayCount(KEY_ARRAY_COUNT, count);

    if (count <= 256 - 2) {   // 0(INVALID_KEY)과 1(KEY_ARRAY_COUNT)은 제외.
        uint8 key = KEY_ARRAY_DATA;
        for (InputIterator itr = begin; itr != end; ++itr) {
            writeObject(key++, **itr, name);
        }
    }
    else {
        InputIterator itr = begin;
        for (uint8 subKey = KEY_ARRAY_DATA; itr != end; subKey++) {
            _addPathForWriteObjects(subKey);
            _pathInto(subKey);
            for (uint8 objKey = 1; 0 < objKey && itr != end; objKey++, ++itr) {
                writeObject(objKey, **itr, name);
            }
            _pathOut();
        }
    }
    _postProcessWriteObjects(key);
    _pathOut();
}
} // namespace SoDA
#endif // _SODA_S2_ARCHIVE_H

