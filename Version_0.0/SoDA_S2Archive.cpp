#include <S2Archive.h>
#include <S2TreedArchive.h>
#include <S2ArchiveDelegate.h>
#include <S2Endian.h>
#include <S2Exception.h>

#define S2TRACE_NAME    L"[S2Archive]"

#include <S2Debug.h>

#include <boost/foreach.hpp>
#include <boost/scoped_array.hpp>

#include <functional>
#include <algorithm>

#include <cstdarg>
#include <cassert>

using namespace SoDA;

S2Archive::S2Archive()
    : _stream(0), _streamBasePosition(0), _version(CURRENT_VERSION), _delegate(0), _created(false)
{
}

S2Archive::S2Archive(const S2Archive& archive)
    : _stream(0), _streamBasePosition(0), _version(CURRENT_VERSION), _delegate(0), _created(false)
{
    operator=(archive);
}

void S2Archive::init(S2Stream* stream, bool create)
{
    _stream = stream;
    _streamBasePosition = _stream->position();

    Item rootItem;
    if (create) {
        writeHeader(&rootItem);
    }
    else {
        readHeader(&rootItem);
    }

    _pathItems.clear();
    _pathItems.push_back(rootItem);
    _lastSubItem.makeInvalid();
}

void S2Archive::Item::init(uint8 key, uint8 type, uint32 size)
{
    this->key = key;
    this->type = type;
    this->next = 0;
    this->size = size;
}

void S2Archive::Item::readFrom(S2Stream* stream)
{
    position = stream->position();

    if (stream->byteOrder() != S2Endian::systemByteOrder()) {
        stream->readUint8(&key);
        stream->readUint8(&type);
        stream->readUint32(&next);
        stream->readUint32(&size);
    }
    else {
        stream->read(&key, savedSize());
    }
}

void S2Archive::Item::writeTo(S2Stream* stream)
{
    position = stream->position();
    if (stream->byteOrder() != S2Endian::systemByteOrder()) {
        stream->writeUint8(key);
        stream->writeUint8(type);
        stream->writeUint32(next);
        stream->writeUint32(size);
    }
    else {
        stream->write(&key, savedSize());
    }
}

uint32 S2Archive::Item::savedSize()
{
    return  sizeof(uint8) +     // key
            sizeof(uint8) +     // type
            sizeof(uint32) +    // next
            sizeof(uint32);     // size
}

////////////////////////////////////////////////////////////////////////////////
// 아카이브 구조 
//
// VERSION_1
//   uint32 magic               // 항상 S2_ARCHIVE_MAGIC
//   uint8  _version            // 항상 VERSION_1
//   uint8  reserved[15]
//   Item   root-item           // 루트 항목은 데이터가 없다.
//   Item   first-item
//   ...... first-item-data
//   Item   second-item
//   ...... second-item-data
//   <...repeat...>
//
//   each Item
//     uint8  key               // 루트 항목은 항상 INVALID_KEY.
//     uint8  type              // 루트 항목은 항상 TYPE_PATH.
//     uint32 next
//     uint32 size
//     ...... data              // 데이터의 크기는 타입에 따라 다르다.
//                              // 표준 타입: sizeof(TYPE)
//                              // S2Object: 재귀적으로 정의됨.

const uint32 S2_ARCHIVE_MAGIC = 0x63569147;
const uint32 S2_ARCHIVE_V1_RESERVED_SIZE = 15;

struct IsZero : std::unary_function<uint8, bool> {
    bool operator()(const uint8& i) const { return i == 0; }
};

uint32 S2Archive::rootItemPosition() const
{
    return streamBasePosition() +
           sizeof(uint32) +             // magic
           sizeof(uint8) +              // version
           S2_ARCHIVE_V1_RESERVED_SIZE; // reserved
}

void S2Archive::readHeader(Item* rootItem)
{
    // magic
    uint32 magic;
    _stream->readUint32(&magic);
    if (magic != S2_ARCHIVE_MAGIC) {
        throw S2Exception(L"S2Archive::readHeader()", EXCEPTION_INVALID_MAGIC);
    }

    // version
    _stream->readUint8(&_version);
    if (CURRENT_VERSION < _version) {
        throw S2Exception(L"S2Archive::readHeader()", EXCEPTION_INVALID_VERSION);
    }

    // reserved
    uint8 reserved[S2_ARCHIVE_V1_RESERVED_SIZE];
    _stream->read(reserved, sizeof(reserved));
    uint8* end = reserved + sizeof(reserved);
    if (end != std::find_if(reserved, end, std::not1(IsZero()))) {
        throw S2Exception(L"S2Archive::readHeader()", EXCEPTION_INVALID_ARCHIVE);
    }

    // root-item
    rootItem->readFrom(_stream);
    if (rootItem->key != INVALID_KEY || rootItem->type != TYPE_PATH) {
        throw S2Exception(L"S2Archive::readHeader()", EXCEPTION_INVALID_ARCHIVE);
    }
}

void S2Archive::writeHeader(Item* rootItem)
{
    // magic
    _stream->writeUint32(S2_ARCHIVE_MAGIC);

    // _version
    _stream->writeUint8(CURRENT_VERSION);

    // reserved
    uint8 reserved[S2_ARCHIVE_V1_RESERVED_SIZE] = { 0, };
    _stream->write(reserved, sizeof(reserved));

    // root item
    rootItem->init(INVALID_KEY, TYPE_PATH, 0);
    rootItem->writeTo(_stream);
}

////////////////////////////////////////////////////////////////////////////////

void S2Archive::_pathInto(uint8 key) const
{
    assert(key != INVALID_KEY);
    assert(!_pathItems.empty());

    // 현재 경로에 key를 추가한다. key는 반드시 존재해야 한다.
    Item item = _pathItems.back();
    if (item.size == 0) {       // 마지막 항목에 하위 항목이 없으면 잘못 지정된 경로.
        throw S2Exception(L"S2Archive::pathInto", EXCEPTION_PATH_NOT_FOUND);
    }

    _stream->moveTo(item.size); // 하위 항목의 위치로 이동.
    item.readFrom(_stream);
    uint32 maxTry = UINT_MAX;
    while (item.key != key) {
        if (item.next == 0 || 0 == --maxTry) {   // 더 이상 하위 항목이 없다.
            throw S2Exception(L"S2Archive::pathInto", EXCEPTION_PATH_NOT_FOUND);
        }
        _stream->moveTo(item.next);
        item.readFrom(_stream);
    }

    _pathItems.push_back(item);
    _lastSubItem.makeInvalid();
}

void S2Archive::_pathOut() const
{
    if (1 < _pathItems.size()) {
        _pathItems.pop_back();
        _lastSubItem.makeInvalid();
    }
}

std::vector<uint8> S2Archive::path() const
{
    std::vector<uint8> path;
    std::vector<Item>::const_iterator i = _pathItems.begin();
    while (++i != _pathItems.end()) {
        path.push_back(i->key);
    }
    return path;
}

void S2Archive::setPath(const std::vector<uint8>& path) const
{
    _pathItems.erase(_pathItems.begin() + 1, _pathItems.end());
    _lastSubItem.makeInvalid(); 

    std::for_each(path.begin(), path.end(),
                  std::bind1st(std::mem_fun(&S2Archive::_pathInto), this));
}

void S2Archive::setPath(uint8 key, ...) const
{
    _pathItems.erase(_pathItems.begin() + 1, _pathItems.end());
    _lastSubItem.makeInvalid();

    va_list args;
    va_start(args, key);
    uint32 maxTry = UINT_MAX;
    while (key != INVALID_KEY && --maxTry) {
        _pathInto(key);
        key = va_arg(args, int32);
    }
    va_end(args);
}

void S2Archive::pathInto(uint8 key) const
{
    _pathInto(key);
}

void S2Archive::pathOut() const
{
    _pathOut();
}

bool S2Archive::findItem(uint8 key, Item* item) const
{
    uint32 position = _pathItems.back().size;   // 하위 항목의 위치.
    uint32 maxTry = UINT_MAX;
    while (position != 0 && --maxTry) {
        _stream->moveTo(position);
        item->readFrom(_stream);
        if (item->key == key) {
            return true;
        }
        position = item->next;
    }
    return false;
}

bool S2Archive::findItem(uint8 key, uint8 type, uint32 size) const
{
    Item item;
    if (findItem(key, &item)) {
        if (item.type != type) {
            throw S2Exception(L"S2Archive::findItem()", EXCEPTION_TYPE_MISMATCH);
        }
        if (item.type != TYPE_PATH && item.size != size) {
            throw S2Exception(L"S2Archive::findItem()", EXCEPTION_SIZE_MISMATCH);
        }
        return true;
    }
    return false;
}

void S2Archive::itemAdded(const Item& item)
{
    if (_pathItems.back().size == 0) {
        Item& parent = _pathItems[_pathItems.size() - 1];
        parent.size = item.position;
        _stream->moveTo(parent.position);
        parent.writeTo(_stream);
    }
    else {
        if (!_lastSubItem.isValid()) {
            uint32 position = _pathItems.back().size;
            uint32 maxTry = UINT_MAX;
            while (position != 0 && --maxTry) {
                _stream->moveTo(position);
                _lastSubItem.readFrom(_stream);
                position = _lastSubItem.next;
            }
        }
        _lastSubItem.next = item.position;
        _stream->moveTo(_lastSubItem.position);
        _lastSubItem.writeTo(_stream);
    }

    _lastSubItem = item;
}

////////////////////////////////////////////////////////////////////////////////

bool S2Archive::findItem(uint8 key, uint8* type, uint32* size) const
{
    Item item;
    if (findItem(key, &item)) {
        if (type) {
            *type = item.type;
        }
        if (size) {
            *size = item.size;
        }
        return true;
    }
    return false;
}

uint32 S2Archive::readDataSize(uint8 key) const
{
    Item item;
    if (!findItem(key, &item)) {
        throw S2Exception(L"readDataSize", EXCEPTION_ITEM_NOT_FOUND);
    }
    return item.size;
}

uint32 S2Archive::readDataPosition(uint8 key) const
{
    Item item;
    if (!findItem(key, &item)) {
        throw S2Exception(L"readDataPosition", EXCEPTION_ITEM_NOT_FOUND);
    }
    assert(item.type != TYPE_PATH);
    return item.position + S2Archive::dataInfoSize() - streamBasePosition();
}

uint32 S2Archive::readArrayCount(uint8 key) const
{
    assert(key != INVALID_KEY);
    if (!findItem(key, TYPE_PATH, 0)) {
        throw S2Exception(L"readArrayCount", EXCEPTION_ITEM_NOT_FOUND);
    }
    _pathInto(key);
    uint32 count = 0;
    if (!readUint32(KEY_ARRAY_COUNT, &count)) {
        _pathOut();
        throw S2Exception(L"readArrayCount", EXCEPTION_ITEM_NOT_FOUND);
    }
    _pathOut();
    return count;
}

uint32 S2Archive::readArrayDataPosition(uint8 key) const
{
    Item item;
    if (!findItem(key, &item)) {
        throw S2Exception(L"readArrayDataPosition", EXCEPTION_ITEM_NOT_FOUND);
    }
    assert(item.type == TYPE_PATH);
    return item.position + S2Archive::arrayInfoSize() - streamBasePosition();
}

////////////////////////////////////////////////////////////////////////////////

void S2Archive::_addPath(uint8 key)
{
    assert(key != INVALID_KEY);

    _stream->moveTo(_stream->size());

    Item item;
    item.init(key, TYPE_PATH, 0);
    item.writeTo(_stream);
    itemAdded(item);
}

void S2Archive::addPath(uint8 key)
{
    _addPath(key);

    if (_delegate) {
        _delegate->archivePathAdded(this);
    }
}

#define DEFINE_SODA_S2_ARCHIVE_TYPE_MEMFUNS(T, Name, ArrayName, Type)           \
    bool S2Archive::read##Name(uint8 key, T* data) const                        \
    {                                                                           \
        assert(key != INVALID_KEY);                                             \
        assert(data != 0);                                                      \
        if (!findItem(key, Type, sizeof(T))) {                                  \
            return false;                                                       \
        }                                                                       \
        _stream->read##Name(data);                                              \
        return true;                                                            \
    }                                                                           \
    void S2Archive::write##Name(uint8 key, T data, const S2String& name)        \
    {                                                                           \
        assert(key != INVALID_KEY);                                             \
        if (!findItem(key, Type, sizeof(T))) {                                  \
            _stream->moveTo(_stream->size());                                   \
            Item item;                                                          \
            item.init(key, Type, sizeof(T));                                    \
            item.writeTo(_stream);                                              \
            _stream->write##Name(data);                                         \
            itemAdded(item);                                                    \
            if (_delegate) {                                                    \
                _delegate->archiveItemAdded(this, key);                         \
            }                                                                   \
        }                                                                       \
        else {                                                                  \
            _stream->write##Name(data);                                         \
            if (_delegate) {                                                    \
                _delegate->archiveItemUpdated(this, key);                       \
            }                                                                   \
        }                                                                       \
    }                                                                           \
    bool S2Archive::read##ArrayName(uint8 key, T* array, uint32 i, uint32 n) const \
    {                                                                           \
        assert(key != INVALID_KEY);                                             \
        assert(array != 0);                                                     \
        if (!findItem(key, TYPE_PATH, 0)) {                                     \
            return false;                                                       \
        }                                                                       \
        _pathInto(key);                                                         \
        uint32 count;                                                           \
        readUint32(KEY_ARRAY_COUNT, &count);                                    \
        if (n == 0) {                                                           \
            n = count - i;                                                      \
        }                                                                       \
        else {                                                                  \
            if (count <= i) {                                                   \
                S2RangeException::Info info(0, count - 1, i);                   \
                throw S2RangeException(L"read...Array", info);                  \
            }                                                                   \
            if (count < i + n) {                                                \
                S2RangeException::Info info(1, count - i, n);                   \
                throw S2RangeException(L"read...Array", info);                  \
            }                                                                   \
        }                                                                       \
        findItem(KEY_ARRAY_DATA, TYPE_BINARY, sizeof(T) * count);               \
        _stream->moveTo(_stream->position() + i * sizeof(T));                   \
        _stream->read##ArrayName(array, n);                                     \
        _pathOut();                                                             \
        return true;                                                            \
    }                                                                           \
    void S2Archive::write##ArrayName(uint8 key, const T* array, uint32 count, const S2String& name)   \
    {                                                                           \
        assert(key != INVALID_KEY);                                             \
        assert(array != 0);                                                     \
        if (!findItem(key, TYPE_PATH, 0)) {                                     \
            _stream->moveTo(_stream->size());                                   \
            _addPath(key);                                                      \
        }                                                                       \
        _pathInto(key);                                                         \
        writeUint32(KEY_ARRAY_COUNT, count);                                    \
        if (!findItem(KEY_ARRAY_DATA, TYPE_BINARY, sizeof(T) * count)) {        \
            _stream->moveTo(_stream->size());                                   \
            Item item;                                                          \
            item.init(KEY_ARRAY_DATA, TYPE_BINARY, sizeof(T) * count);          \
            item.writeTo(_stream);                                              \
            _stream->write##ArrayName(array, count);                            \
            itemAdded(item);                                                    \
            if (_delegate) {                                                    \
                _delegate->archiveItemAdded(this, key);                         \
            }                                                                   \
        }                                                                       \
        else {                                                                  \
            _stream->write##ArrayName(array, count);                            \
            if (_delegate) {                                                    \
                _delegate->archiveItemUpdated(this, key);                       \
            }                                                                   \
        }                                                                       \
        _pathOut();                                                             \
    }

#define DEFINE_SODA_S2_ARCHIVE_TYPE_MEMFUNS_WITH_MANIPULATE(T, Name, ArrayName, Type) \
    DEFINE_SODA_S2_ARCHIVE_TYPE_MEMFUNS(T, Name, ArrayName, Type)               \
    void S2Archive::reserve##ArrayName(uint8 key, uint32 count)                 \
    {                                                                           \
        assert(key != INVALID_KEY);                                             \
        if (!findItem(key, TYPE_PATH, 0)) {                                     \
            _stream->moveTo(_stream->size());                                   \
            _addPath(key);                                                      \
        }                                                                       \
        _pathInto(key);                                                         \
        writeUint32(KEY_ARRAY_COUNT, count);                                    \
        if (!findItem(KEY_ARRAY_DATA, TYPE_BINARY, sizeof(T) * count)) {        \
            _stream->moveTo(_stream->size());                                   \
            Item item;                                                          \
            item.init(KEY_ARRAY_DATA, TYPE_BINARY, sizeof(T) * count);          \
            item.writeTo(_stream);                                              \
            _stream->moveTo(_stream->position() + (sizeof(T) * count) - 1);     \
            _stream->writeUint8(0);                                             \
            itemAdded(item);                                                    \
            if (_delegate) {                                                    \
                _delegate->archiveItemAdded(this, key);                         \
            }                                                                   \
        }                                                                       \
        _pathOut();                                                             \
    }                                                                           \
    void S2Archive::update##ArrayName(uint8 key, const T* array, uint32 count, uint32 offset) \
    {                                                                           \
        assert(key != INVALID_KEY);                                             \
        assert(array != 0);                                                     \
        if (!findItem(key, TYPE_PATH, 0)) {                                     \
            throw S2Exception(L"updateArray", EXCEPTION_ITEM_NOT_FOUND);        \
        }                                                                       \
        _pathInto(key);                                                         \
        uint8 type = 0;                                                         \
        uint32 size = 0;                                                        \
        if (!findItem(KEY_ARRAY_DATA, &type, &size)) {                          \
            throw S2Exception(L"updateArray", EXCEPTION_ITEM_NOT_FOUND);        \
        }                                                                       \
        if (TYPE_BINARY != type) {                                              \
            throw S2Exception(L"updateArray", EXCEPTION_TYPE_MISMATCH);         \
        }                                                                       \
        else if (size < (sizeof(T) * (offset + count))) {                       \
            throw S2Exception(L"updateArray", EXCEPTION_SIZE_MISMATCH);         \
        }                                                                       \
        else {                                                                  \
            _stream->moveTo(_stream->position() + (sizeof(T) * offset));        \
            _stream->write##ArrayName(array, count);                            \
            if (_delegate) {                                                    \
                _delegate->archiveItemUpdated(this, key);                       \
            }                                                                   \
        }                                                                       \
        _pathOut();                                                             \
    }

DEFINE_SODA_S2_ARCHIVE_TYPE_MEMFUNS_WITH_MANIPULATE(  bool,    Bool,    BoolArray,  TYPE_BOOL)
DEFINE_SODA_S2_ARCHIVE_TYPE_MEMFUNS_WITH_MANIPULATE(  int8,    Int8,    Int8Array,  TYPE_INT8)
DEFINE_SODA_S2_ARCHIVE_TYPE_MEMFUNS_WITH_MANIPULATE( uint8,   Uint8,   Uint8Array,  TYPE_UINT8)
DEFINE_SODA_S2_ARCHIVE_TYPE_MEMFUNS(                  int16,   Int16,   Int16Array, TYPE_INT16)
DEFINE_SODA_S2_ARCHIVE_TYPE_MEMFUNS(                 uint16,  Uint16,  Uint16Array, TYPE_UINT16)
DEFINE_SODA_S2_ARCHIVE_TYPE_MEMFUNS(                  int32,   Int32,   Int32Array, TYPE_INT32)
DEFINE_SODA_S2_ARCHIVE_TYPE_MEMFUNS(                 uint32,  Uint32,  Uint32Array, TYPE_UINT32)
DEFINE_SODA_S2_ARCHIVE_TYPE_MEMFUNS(                  int64,   Int64,   Int64Array, TYPE_INT64)
DEFINE_SODA_S2_ARCHIVE_TYPE_MEMFUNS(                 uint64,  Uint64,  Uint64Array, TYPE_UINT64)
DEFINE_SODA_S2_ARCHIVE_TYPE_MEMFUNS(                float32, Float32, Float32Array, TYPE_FLOAT32)
DEFINE_SODA_S2_ARCHIVE_TYPE_MEMFUNS(                float64, Float64, Float64Array, TYPE_FLOAT64)

bool S2Archive::readBinary(uint8 key, void* data, uint32 size) const
{
    if (!findItem(key, TYPE_BINARY, size)) {
        return false;
    }
    _stream->read(data, size);
    return true;
}

void S2Archive::writeBinary(uint8 key, const void* data, uint32 size, const S2String& name)
{
    if (!findItem(key, TYPE_BINARY, size)) {
        _stream->moveTo(_stream->size());
        Item item;
        item.init(key, TYPE_BINARY, size);
        item.writeTo(_stream);
        _stream->write(data, size);
        itemAdded(item);
        if (_delegate) {
            _delegate->archiveItemAdded(this, key);
        }
    }
    else {
        _stream->write(data, size);
        if (_delegate) {
            _delegate->archiveItemUpdated(this, key);
        }
    }
}

bool S2Archive::readObject(uint8 key, S2Object* object) const
{
    if (!findItem(key, TYPE_PATH, 0)) {
        return false;
    }
    _pathInto(key);
    object->unarchiveFrom(const_cast<S2Archive*>(this));
    _pathOut();
    return true;
}

void S2Archive::writeObject(uint8 key, const S2Object& object, const S2String& name)
{
    bool add = !findItem(key, TYPE_PATH, 0);
    if (add) {
        _stream->moveTo(_stream->size());
        _addPath(key);
    }
    _pathInto(key);
    object.archiveTo(const_cast<S2Archive*>(this));
    _pathOut();

    if (_delegate) {
        if (add) {
            _delegate->archiveItemAdded(this, key);
        }
        else {
            _delegate->archiveItemUpdated(this, key);
        }
    }
}

uint32 S2Archive::dataInfoSize()
{
    return Item::savedSize();
}

uint32 S2Archive::arrayInfoSize()
{
    return dataInfoSize() +     // 항목 자체.
           Item::savedSize() +  // 개수 항목.
           sizeof(uint32) +     // 개수 데이터.
           Item::savedSize();   // 배열 데이터 항목.
}

////////////////////////////////////////////////////////////////////////////////

void S2Archive::copyItemOnly(const S2Archive& archive, const Item& item)
{
    if (findItem(item.key, item.type, item.size)) {
        if (item.type != TYPE_PATH) {
            boost::scoped_array<uint8> data(new uint8[item.size]);
            archive._stream->read(data.get(), item.size);
            _stream->write(data.get(), item.size);
            if (_delegate) {
                _delegate->archiveItemUpdated(this, item.key);
            }
        }
    }
    else {
        if (item.type == TYPE_PATH) {
            _addPath(item.key);
        }
        else {
            _stream->moveTo(_stream->size());
            Item dstItem;
            dstItem.init(item.key, item.type, item.size);
            dstItem.writeTo(_stream);
            boost::scoped_array<uint8> data(new uint8[item.size]);
            archive._stream->read(data.get(), item.size);
            _stream->write(data.get(), item.size);
            itemAdded(dstItem);
        }
        if (_delegate) {
            _delegate->archiveItemAdded(this, item.key);
        }
    }
}

void S2Archive::copyItemAndSubItems(const S2Archive& archive, const Item& item)
{
    copyItemOnly(archive, item);

    if (item.type == TYPE_PATH && item.size != 0) {
        _pathInto(item.key);
        archive._stream->moveTo(item.size);

        Item subItem;
        while (true) {
            subItem.readFrom(archive._stream);
            copyItemAndSubItems(archive, subItem);
            if (subItem.next == 0) {
                break;
            }
            archive._stream->moveTo(subItem.next);
        }
        _pathOut();
    }
}

#define BOTH_S2_ARCHIVE_TYPE(a, b) \
    (!dynamic_cast<const S2TreedArchive*>(a) && !dynamic_cast<const S2TreedArchive*>(&b))

void S2Archive::copyFrom(const S2Archive& archive, uint8 key, bool copyPath)
{
    if (BOTH_S2_ARCHIVE_TYPE(this, archive)) {
        std::vector<uint8> orgPath = path();
        uint32 orgPosition = archive._stream->position();

        if (key == 0) {
            archive._stream->moveTo(archive.rootItemPosition());
            Item rootItem;
            rootItem.readFrom(archive._stream);
            if (rootItem.size != 0) {
                archive._stream->moveTo(rootItem.size);

                Item item;
                while (true) {
                    item.readFrom(archive._stream);
                    copyItemAndSubItems(archive, item);
                    if (item.next == 0) {
                        break;
                    }
                    archive._stream->moveTo(item.next);
                }
            }
        }
        else {
            Item item;
            if (!archive.findItem(key, &item)) {
                throw S2Exception(L"copyFrom", EXCEPTION_ITEM_NOT_FOUND);
            }

            if (copyPath) {
                Item pathItem;
                BOOST_FOREACH(uint8 pathKey, archive.path()) {
                    if (findItem(pathKey, &pathItem)) {
                        if (pathItem.type != TYPE_PATH) {
                            throw S2Exception(L"S2Archive::copyFrom", EXCEPTION_PATH_NOT_FOUND);
                        }
                        _pathInto(pathKey);
                    }
                    else {
                        _addPath(pathKey);
                    }
                }
            }

            copyItemAndSubItems(archive, item);
        }

        setPath(orgPath);
        archive._stream->moveTo(orgPosition);
    }
    else {
        S2ASSERT(false, L"Different data format");
    }
}

S2Archive& S2Archive::operator=(const S2Archive& archive)
{
    if (BOTH_S2_ARCHIVE_TYPE(this, archive)) {
        _stream->moveTo(rootItemPosition());
        Item rootItem;
        rootItem.readFrom(archive._stream);
        if (rootItem.size != 0) {
            archive._stream->moveTo(rootItem.size);
        }

        copyFrom(archive, INVALID_KEY);
    }
    else {
        S2ASSERT(false, L"Different data format");
    }

    return *this;
}

#undef BOTH_S2_ARCHIVE_TYPE

////////////////////////////////////////////////////////////////////////////////

void S2Archive::scanPath(S2Archive::Scanner* scanner) const
{
    Item item;
    while (true) {
        item.readFrom(_stream);
        if (item.type == TYPE_PATH) {
            scanner->itemScanned(item.key, item.type, 0);
            if (item.size != 0) {
                _stream->moveTo(item.size);
                scanner->_depth++;
                scanPath(scanner);
                scanner->_depth--;
            }
        }
        else {
            if (scanner->_buffer) {
                #define CASE_TYPE_READ(Type, Name, type)    \
                    case TYPE_##Type :                      \
                        _stream->read##Name(                \
                            reinterpret_cast<type*>(        \
                                scanner->_buffer));         \
                        break
                switch (item.type) {
                    case TYPE_PATH : break; // 경고 제거용.
                    CASE_TYPE_READ( BOOL,     Bool,   bool);
                    CASE_TYPE_READ( INT8,     Int8,   int8);
                    CASE_TYPE_READ(UINT8,    Uint8,  uint8);
                    CASE_TYPE_READ( INT16,    Int16,  int16);
                    CASE_TYPE_READ(UINT16,   Uint16, uint16);
                    CASE_TYPE_READ( INT32,    Int32,  int32);
                    CASE_TYPE_READ(UINT32,   Uint32, uint32);
                    CASE_TYPE_READ( INT64,    Int64,  int64);
                    CASE_TYPE_READ(UINT64,   Uint64, uint64);
                    CASE_TYPE_READ(FLOAT32, Float32, float32);
                    CASE_TYPE_READ(FLOAT64, Float64, float64);
                    case TYPE_BINARY :
                        _stream->moveTo(_stream->position() + item.size);
                        break;
                }
            }
            scanner->itemScanned(item.key, item.type, scanner->_buffer);
        }
        if (item.next == 0) {
            break;
        }
        _stream->moveTo(item.next);
    }
}

void S2Archive::scan(S2Archive::Scanner* scanner) const
{
    try {
        uint32 position = _stream->position();
        scanner->begin();

        _stream->moveTo(streamBasePosition());
        Item rootItem;
        const_cast<S2Archive*>(this)->readHeader(&rootItem);
        scanner->headerScanned(*this);

        if (rootItem.size != 0) {
            _stream->moveTo(rootItem.size);
            scanner->_depth = 0;
            scanPath(scanner);
        }

        scanner->end();
        _stream->moveTo(position);
    }
    catch (S2Exception&) {
        throw;
    }
}

S2String S2Archive::toString() const
{
    class SScanner : public Scanner {
    public:
        uint8 _buffer[256];
        S2String _string;
        SScanner() : Scanner(_buffer, sizeof(_buffer)) {}
        virtual void begin() { _string = L"{"; }
        virtual void end() { _string += L"\n}"; }
        virtual void itemScanned(uint8 key, uint8 type, void* data)
        {
            _string += L"\n";
            for (uint32 i = 0; i <= depth(); ++i) {
                _string += L"  ";
            }
            #define CASE_TYPE_PRINT(Type, format, type)                 \
                case TYPE_##Type :                                      \
                    if (data) {                                         \
                        S2String s(#Type);                              \
                        _string.appendf(L"[%u:%@]=" format, key, &s,    \
                                *reinterpret_cast<type*>(data));        \
                    }                                                   \
                    else {                                              \
                        S2String s(#Type);                              \
                        _string.appendf(L"[%u:%@]", key, &s);           \
                    }                                                   \
                    break
            switch (type) {
                case TYPE_PATH :
                    _string.appendf(L"[%u:PATH]", key);
                    break;
                case TYPE_BOOL :
                    _string.appendf(L"[%u:BOOL]=%s", key,
                        *reinterpret_cast<bool*>(data) ? "true" : "false");
                    break;
                CASE_TYPE_PRINT(  INT8,  L"%d",    int8);
                CASE_TYPE_PRINT( UINT8,  L"%u",   uint8);
                CASE_TYPE_PRINT(  INT16, L"%d",    int16);
                CASE_TYPE_PRINT( UINT16, L"%u",   uint16);
                CASE_TYPE_PRINT(  INT32, L"%d",    int32);
                CASE_TYPE_PRINT( UINT32, L"%u",   uint32);
                CASE_TYPE_PRINT(  INT64, L"%lld",  int64);
                CASE_TYPE_PRINT( UINT64, L"%llu", uint64);
                CASE_TYPE_PRINT(FLOAT32, L"%f",  float32);
                CASE_TYPE_PRINT(FLOAT64, L"%f",  float64);
                case TYPE_BINARY : {
                    _string.appendf(L"[%u:BINARY]=...", key);
                    break;
                }
                default:
                    _string.appendf(L"[%u:UNKNOWN(%u)]", key, type);
                    break;
            }
        }
    };

    SScanner scanner;
    const_cast<S2Archive*>(this)->scan(&scanner);
    return scanner._string;
}

void S2Archive::_preProcessWriteObjects(uint8 key)
{
    _stream->moveTo(_stream->size());
    Item item;
    item.init(key, TYPE_PATH, 0);
    item.writeTo(_stream);
    itemAdded(item);
}

void S2Archive::_addPathForWriteObjects(uint8 key)
{
    _stream->moveTo(_stream->size());
    Item item;
    item.init(key, TYPE_PATH, 0);
    item.writeTo(_stream);
    itemAdded(item);
}
