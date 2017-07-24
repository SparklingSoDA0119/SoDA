#include <S2TreedArchive.h>
#include <S2TreedArchiveTypes.h>
#include <S2ArchiveDelegate.h>
#include <S2EncodeUtil.h>
#include <std_locale_utf8.h>

#define S2TRACE_NAME L"S2TreedArchive"
#include <S2Debug.h>

#include <boost/foreach.hpp>
#include <boost/scoped_array.hpp>

#include <deque>
#include <cmath>
#include <string.h>

using namespace SoDA;

namespace {
    const S2String ARRAY_STRING(L"Array");
}

S2TreedArchive::S2TreedArchive(S2TreedArchiveParser* parser, bool needFree)
: _parser(parser)
, _needFree(needFree)
, _lastInsertedKey(0)
, _addRootOnLoad(false)
, _removeRootOnSave(false)
{
    _rootElement = new S2TreedArchiveElement();
    _rootElement->setDataType(S2TreedArchive::TYPE_PATH);

    _currentElement = _rootElement;

    _parser->setDelegate(this);
}

S2TreedArchive::~S2TreedArchive()
{
    if (_needFree && _parser) {
        delete _parser;
        _parser = NULL;
    }

    if (_rootElement) {
        _rootElement->clear();
        delete _rootElement;
        _rootElement = NULL;
    }
}

void S2TreedArchive::close()
{
    if (created()) {
        const std::vector<S2TreedArchiveElement*>& children = _rootElement->children();
        if (_removeRootOnSave && !children.empty()) {
            _rootElement = children[0];
            _rootElement->setParentElement(0);
            _rootElement->setIsVirtualRoot(true);
        }

        _parser->save(_stream, _rootElement);
    }

    _rootElement->clear();

    S2Archive::close();
}

void S2TreedArchive::init(S2Stream* stream, bool create)
{
    _rootElement->setValue(0, L"", 0, false, 0);

    Item rootItem;
    _pathItems.clear();
    _pathItems.push_back(rootItem);

    _stream = stream;

    if (!create) {
        if (_parser->parse(_stream, _rootElement)) {
            if (_addRootOnLoad) {
                S2TreedArchiveElement* elem = new S2TreedArchiveElement();
                elem->appendChildElement(_rootElement);
                _rootElement->setParentElement(elem);
                _rootElement = elem;
            }
            _currentElement = _rootElement;
        }
        else {
            S2TRACE(L"Parse Exception : %@", &_parser->lastError());
            throw S2Exception(_parser->lastError(), EXCEPTION_PARSE_ERROR);
        }
    }
}

bool S2TreedArchive::findItem(uint8 key, uint8* type, uint32* size) const
{
    const std::vector<S2TreedArchiveElement*>& children = _currentElement->children();

    BOOST_FOREACH(const S2TreedArchiveElement* each, children) {
        if (each->archiveKey() == key) {
            if (type) {
                *type = each->dataType();
            }
            if (size) {
                *size = each->dataSize();
            }

            return true;
        }
    }

    return false;
}

bool S2TreedArchive::removeItem(uint8 key)
{
    return _currentElement->removeChild(key);
}

bool S2TreedArchive::findItem(uint8 key, Item* item) const
{
    const std::vector<S2TreedArchiveElement*>& children = _currentElement->children();

    BOOST_FOREACH(const S2TreedArchiveElement* each, children) {
        if (each->archiveKey() == key) {
            item->key = key;
            item->size = each->dataSize();
            item->type = each->dataType();
            return true;
        }
    }

    return false;
}

bool S2TreedArchive::findItem(uint8 key, uint8 type, uint32 size) const
{
    const std::vector<S2TreedArchiveElement*>& children = _currentElement->children();

    BOOST_FOREACH(const S2TreedArchiveElement* each, children) {
        if (each->archiveKey() == key) {
            if (type == TYPE_PATH) {
                if (each->dataType() != TYPE_PATH) {
                    throw S2Exception(L"S2TreedArchive::findItem()", EXCEPTION_TYPE_MISMATCH);
                }
            }
            else if (each->dataType() != type) {
                throw S2Exception(L"S2TreedArchive::findItem()", EXCEPTION_TYPE_MISMATCH);
            }

            return true;
        }
    }

    return false;
}

uint32 S2TreedArchive::readDataSize(uint8 key) const
{
    S2TreedArchiveElement* found = findElement(key);
    if (found) {
        return found->dataSize();
    }

    throw S2Exception(L"S2TreedArchive::readDataSize()", EXCEPTION_ITEM_NOT_FOUND);
}

uint32 S2TreedArchive::readDataPosition(uint8 key) const
{
    throw S2Exception(L"S2TreedArchive::readDataPosition() not supported", EXCEPTION_INVALID_ARCHIVE);
}

void S2TreedArchive::writeEmptyArray(uint8 key, const S2String& name)
{
    if (!findItem(key)) {
        _addPath(key);
        _pathInto(key);
        _currentElement->setArchiveKey(key);
        _currentElement->setIsArray(true);
        _currentElement->setName(name + ARRAY_STRING);
        _pathOut();
    }
}

S2TreedArchiveElement* S2TreedArchive::findElement(uint8 key) const
{
    const std::vector<S2TreedArchiveElement*>& children = _currentElement->children();

    BOOST_FOREACH(S2TreedArchiveElement* each, children) {
        if (each->archiveKey() == key) {
            return each;
        }
    }

    return NULL;
}

void S2TreedArchive::setPath(const std::vector<uint8>& path) const
{
    _pathItems.erase(_pathItems.begin() + 1, _pathItems.end());
    _lastSubItem.makeInvalid();

    BOOST_FOREACH(uint8 key, path) {
        bool keyFound = false;
        const std::vector<S2TreedArchiveElement*>& children = _currentElement->children();
        BOOST_FOREACH(const S2TreedArchiveElement* each, children) {
            if (each->archiveKey() == key) {
                keyFound = true;
            }
        }
        if (!keyFound) {
            S2TreedArchiveElement* element(new S2TreedArchiveElement());
            element->setArchiveKey(key);
            _currentElement->appendChildElement(element);
        }
        _pathInto(key);
    }
}


void S2TreedArchive::setPath(uint8 key, ...) const
{
    _pathItems.erase(_pathItems.begin() + 1, _pathItems.end());
    _lastSubItem.makeInvalid();

    va_list args;
    va_start(args, key);
    while (key != INVALID_KEY) {
        bool keyFound = false;
        const std::vector<S2TreedArchiveElement*>& children = _currentElement->children();
        BOOST_FOREACH(const S2TreedArchiveElement* each, children) {
            if (each->archiveKey() == key) {
                keyFound = true;
            }
        }
        if (!keyFound) {
            S2TreedArchiveElement* element(new S2TreedArchiveElement());
            element->setArchiveKey(key);
            _currentElement->appendChildElement(element);
        }
        _pathInto(key);
        key = va_arg(args, int32);
    }
    va_end(args);
}

void S2TreedArchive::_pathInto(uint8 key) const
{
    const std::vector<S2TreedArchiveElement*>& children = _currentElement->children();
    BOOST_FOREACH(S2TreedArchiveElement* each, children) {
        if (each->dataType() == S2Archive::TYPE_PATH && each->archiveKey() == key) {
            _currentElement = each;
            Item item;
            item.key = key;
            _pathItems.push_back(item);
            return;
        }
    }

    throw S2Exception(L"S2TreedArchive::pathInto", EXCEPTION_PATH_NOT_FOUND);
}

void S2TreedArchive::_pathOut() const
{
    if (1 < _pathItems.size()) {
        _currentElement = _currentElement->parentElement();
        _pathItems.pop_back();
    }
}

void S2TreedArchive::_addPath(uint8 key)
{
    _addPath(key, false);
}

void S2TreedArchive::_addPath(uint8 key, bool isArray)
{
    S2TreedArchiveElement* element(new S2TreedArchiveElement());
    element->setParentElement(_currentElement);
    element->setDataType(S2TreedArchive::TYPE_PATH);
    element->setArchiveKey(key);
    element->setIsArray(isArray);

    _currentElement->appendChildElement(element);
}

uint32 S2TreedArchive::readArrayCount(uint8 key) const
{
    assert(key != INVALID_KEY);
    if (!findItem(key, TYPE_PATH, 0)) {
        throw S2Exception(L"readArrayCount", EXCEPTION_ITEM_NOT_FOUND);
    }
    _pathInto(key);
    uint32 count = static_cast<uint32>(_currentElement->arrayCount());
    _pathOut();
    return count;
}

uint32 S2TreedArchive::readArrayDataPosition(uint8 key) const
{
    S2ASSERT(false, L"S2TreedArchive::readArrayDataPosition is not supported!!");
    return 0;
}

#define DEFINE_SODA_S2_XML_ARCHIVE_TYPE_MEMFUNS(T, Name, ArrayName, Type)       \
    bool S2TreedArchive::read##Name(uint8 key, T* data) const                   \
    {                                                                           \
        assert(key != INVALID_KEY);                                             \
        assert(data != 0);                                                      \
        for (uint8 i = 0; i < _currentElement->numChildren(); ++i) {            \
            if (_currentElement->child(i)->archiveKey() == key) {               \
                if (_currentElement->child(i)->dataType() != Type) {            \
                    throw S2Exception(L"S2TreedArchive::findItem()", EXCEPTION_TYPE_MISMATCH);    \
                }                                                               \
                *data = _currentElement->child(i)->to##Name();                  \
                return true;                                                    \
            }                                                                   \
        }                                                                       \
        return false;                                                           \
    }                                                                           \
    void S2TreedArchive::write##Name(uint8 key, T data, const S2String& name)   \
    {                                                                           \
        assert(key != INVALID_KEY);                                             \
        for (uint8 i = 0; i < _currentElement->numChildren(); ++i) {            \
            if (_currentElement->child(i)->archiveKey() == key) {               \
                if (_currentElement->child(i)->dataType() != Type) {            \
                    throw S2Exception(L"S2TreedArchive::findItem()", EXCEPTION_TYPE_MISMATCH);    \
                }                                                               \
                _currentElement->child(i)->setValue(key, data, name);           \
                if (_delegate) {                                                \
                    _delegate->archiveItemUpdated(this, key);                   \
                }                                                               \
                return;                                                         \
            }                                                                   \
        }                                                                       \
        S2TreedArchiveElement* element(new S2TreedArchiveElement());            \
        element->setValue(key, data, name);                                     \
        _currentElement->appendChildElement(element);                           \
        _lastInsertedKey = key;                                                 \
        if (_delegate) {                                                        \
            _delegate->archiveItemAdded(this, key);                             \
        }                                                                       \
    }                                                                           \
    bool S2TreedArchive::read##ArrayName(uint8 key, T* array, uint32 i, uint32 n) const \
    {                                                                           \
        assert(key != INVALID_KEY);                                             \
        assert(array != 0);                                                     \
        if (!findItem(key, TYPE_PATH, 0)) {                                     \
            return false;                                                       \
        }                                                                       \
        _pathInto(key);                                                         \
        _currentElement->setIsArray(true);                                      \
        uint32 count = static_cast<uint32>(_currentElement->numChildren());     \
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
        const std::vector<S2TreedArchiveElement*>& children = _currentElement->children(); \
        for (uint32 index = i; index < n + i; ++index) {                        \
            if (children[index]->dataType() != Type) {                          \
                throw S2Exception(L"S2TreedArchive::findItem()", EXCEPTION_TYPE_MISMATCH);    \
            }                                                                   \
            array[index-i] = children[index]->to##Name();                       \
        }                                                                       \
        _pathOut();                                                             \
        return true;                                                            \
    }                                                                           \
    void S2TreedArchive::write##ArrayName(uint8 key, const T* array, uint32 count, const S2String& name) \
    {                                                                           \
        assert(key != INVALID_KEY);                                             \
        assert(array != 0);                                                     \
        if (!findItem(key, TYPE_PATH, 0)) {                                     \
            _addPath(key);                                                      \
        }                                                                       \
        _pathInto(key);                                                         \
        _currentElement->setIsArray(true);                                      \
        _currentElement->setArrayCount(count);                                  \
        _currentElement->setValue(key, L"", TYPE_PATH, false, 0, name + ARRAY_STRING); \
        uint32 prevCount = _currentElement->numChildren();                      \
        if (prevCount) {                                                        \
            if (_currentElement->child(0)->dataType() != Type) {                \
               throw S2Exception(L"S2TreedArchive::findItem()", EXCEPTION_TYPE_MISMATCH);    \
            }                                                                   \
        }                                                                       \
        _currentElement->removeChildren();                                      \
                                                                                \
        for (uint32 i = 0; i < count; ++i) {                                    \
            S2TreedArchiveElement* element(new S2TreedArchiveElement());        \
            element->setValue(key, array[i], name);                             \
            _currentElement->appendChildElement(element);                       \
        }                                                                       \
        if (_delegate) {                                                        \
            if (prevCount) {                                                    \
                _delegate->archiveItemUpdated(this, key);                       \
            }                                                                   \
            else {                                                              \
                _delegate->archiveItemAdded(this, key);                         \
            }                                                                   \
        }                                                                       \
        _pathOut();                                                             \
        _lastInsertedKey = key;                                                 \
    }

DEFINE_SODA_S2_XML_ARCHIVE_TYPE_MEMFUNS(  bool,    Bool,    BoolArray,  TYPE_BOOL)
DEFINE_SODA_S2_XML_ARCHIVE_TYPE_MEMFUNS(  int8,    Int8,    Int8Array,  TYPE_INT8)
DEFINE_SODA_S2_XML_ARCHIVE_TYPE_MEMFUNS( uint8,   Uint8,   Uint8Array,  TYPE_UINT8)
DEFINE_SODA_S2_XML_ARCHIVE_TYPE_MEMFUNS(  int16,   Int16,   Int16Array, TYPE_INT16)
DEFINE_SODA_S2_XML_ARCHIVE_TYPE_MEMFUNS( uint16,  Uint16,  Uint16Array, TYPE_UINT16)
DEFINE_SODA_S2_XML_ARCHIVE_TYPE_MEMFUNS(  int32,   Int32,   Int32Array, TYPE_INT32)
DEFINE_SODA_S2_XML_ARCHIVE_TYPE_MEMFUNS( uint32,  Uint32,  Uint32Array, TYPE_UINT32)
DEFINE_SODA_S2_XML_ARCHIVE_TYPE_MEMFUNS(  int64,   Int64,   Int64Array, TYPE_INT64)
DEFINE_SODA_S2_XML_ARCHIVE_TYPE_MEMFUNS( uint64,  Uint64,  Uint64Array, TYPE_UINT64)
DEFINE_SODA_S2_XML_ARCHIVE_TYPE_MEMFUNS(float32, Float32, Float32Array, TYPE_FLOAT32)
DEFINE_SODA_S2_XML_ARCHIVE_TYPE_MEMFUNS(float64, Float64, Float64Array, TYPE_FLOAT64)

bool S2TreedArchive::readObject(uint8 key, S2Object* object) const
{
    if (!findItem(key, TYPE_PATH, 0)) {
        return false;
    }
    _pathInto(key);
    object->unarchiveFrom(const_cast<S2TreedArchive*>(this));
    _pathOut();

    return true;
}

void S2TreedArchive::writeObject(uint8 key, const S2Object& object, const S2String& name)
{
    bool isArray = _currentElement->isArray();
    bool add = false;
    if (isArray) {
        add = true;
        S2TreedArchiveElement* element(new S2TreedArchiveElement());
        element->setParentElement(_currentElement);
        _currentElement->appendChildElement(element);
        _currentElement = element;
    }
    else {
        add = !findItem(key, TYPE_PATH, 0);

        if (add) {
            _addPath(key);
        }
        _pathInto(key);
    }

    _currentElement->setArchiveKey(key);
    _lastInsertedKey = key;

    object.archiveTo(const_cast<S2TreedArchive*>(this));

    _currentElement->setName(name);

    if (isArray) {
        _currentElement = _currentElement->parentElement();
    }
    else {
        _pathOut();
    }

    if (_delegate) {
        if (add) {
            _delegate->archiveItemAdded(this, key);
        }
        else {
            _delegate->archiveItemUpdated(this, key);
        }
    }
}

bool S2TreedArchive::readBinary(uint8 key, void* data, uint32 size) const
{
    assert(key != INVALID_KEY);
    assert(data != 0);

    S2TreedArchiveElement* found = findElement(key);
    if (found) {
        if (found->dataType() != TYPE_BINARY) {
            throw S2Exception(L"S2TreedArchive::readBinary", EXCEPTION_TYPE_MISMATCH);
        }
        if (size < found->dataSize()) {
            throw S2Exception(L"S2TreedArchive::readBinary()", EXCEPTION_SIZE_MISMATCH);
        }

        boost::scoped_array<uint8> decoded(new uint8[found->dataSize()]);
        memset(decoded.get(), 0, found->dataSize());

        std::string copied = found->toS2String().to_string();
        const char* encoded = copied.c_str();

        S2EncodeUtil::base64Decode(decoded.get(), (const uint8*)encoded);

        memcpy(data, decoded.get(), size);

        return true;
    }

    throw S2Exception(L"S2TreedArchive::readBinary()", EXCEPTION_ITEM_NOT_FOUND);
}

void S2TreedArchive::writeBinary(uint8 key, const void* data, uint32 size, const S2String& name)
{
    uint32 destSize = static_cast<uint32>(std::ceil(size / 3.0f) * 4.0f + 1.0f);

    boost::scoped_array<uint8> dest(new uint8[destSize]);
    memset(dest.get(), 0, destSize);

    S2EncodeUtil::base64Encode(dest.get(), static_cast<const uint8*>(data), size);

    _lastInsertedKey = key;
    S2String encodedString(reinterpret_cast<char*>(dest.get()), STD_LOCALE_UTF8);


    bool found = findItem(key, TYPE_BINARY, 0);

    if (found) {
        _currentElement->removeChild(key);
    }
    _addPath(key);

    _pathInto(key);
    _currentElement->setValue(key, encodedString, S2Archive::TYPE_BINARY, false, size, name);
    _pathOut();

    if (_delegate) {
        found ? _delegate->archiveItemUpdated(this, key)
              : _delegate->archiveItemAdded(this, key);
    }
}

void S2TreedArchive::_writeArrayCount(uint8 key, uint32 count)
{
    _currentElement->setArrayCount(count);
}

bool S2TreedArchive::_readArrayCountObjectsArray(uint8 key, uint32* count) const
{
    _pathOut();
    *count = readArrayCount(key);
    _pathInto(key);
    _currentElement->setArrayCount(*count);

    return true;
}

const S2String& S2TreedArchive::readS2String() const
{
    return _currentElement->toS2String();

}

void S2TreedArchive::writeS2String(uint8 key, const S2String& toWrite)
{
    writeS2String(key, toWrite, TYPE_PATH, toWrite.length());
}

void S2TreedArchive::writeS2String(uint8 key, const S2String& toWrite, uint8 dataType, uint32 dataSize)
{
    _currentElement->setValue(_currentElement->archiveKey(), toWrite, dataType, true, dataSize);
}

#define BOTH_S2_TREED_ARCHIVE_TYPE(a, b) \
    (dynamic_cast<const S2TreedArchive*>(a) && dynamic_cast<const S2TreedArchive*>(&b))

void S2TreedArchive::copyFrom(const S2TreedArchive& archive, uint8 key, bool copyPath /* = true */)
{
    if (BOTH_S2_TREED_ARCHIVE_TYPE(this, archive)) {
        S2TreedArchiveElement* currentElement = _currentElement;

        if (key == 0) {
            _copyFrom(archive.rootElement());
        }
        else {
            S2TreedArchiveElement* copyTarget;
            S2TreedArchiveElement* element = archive.findElement(key);
            if (element == NULL) {
                throw S2Exception(L"copyFrom", EXCEPTION_ITEM_NOT_FOUND);
            }

            std::deque<S2TreedArchiveElement*> path;
            for (S2TreedArchiveElement* ptr = element;
                ptr && ptr->dataType() == TYPE_PATH && ptr != archive.rootElement();
                ptr = const_cast<S2TreedArchiveElement*>(ptr->parentElement())) {

                path.push_front(ptr);
            }

            if (copyPath) {
                S2TreedArchiveElement* found;
                BOOST_FOREACH(const S2TreedArchiveElement* each, path) {
                    found = findElement(each->archiveKey());
                    if (found) {
                        if (found->dataType() != TYPE_PATH) {
                            throw S2Exception(L"S2Archive::copyFrom", EXCEPTION_PATH_NOT_FOUND);
                        }
                        _pathInto(each->archiveKey());
                    }
                    else {
                        _addPath(each->archiveKey());
                    }
                }
            }

            S2TreedArchiveElement* existing = findElement(key);
            if (existing) {
                copyTarget = existing;
            }
            else {
                copyTarget = new S2TreedArchiveElement();
            }
            _currentElement = copyTarget;

            const S2TreedArchiveElement* copyPoint = archive.findElement(key);
            _copyFrom(copyPoint);

            if (!existing) {
                currentElement->appendChildElement(copyTarget);
                copyTarget->setParentElement(currentElement);
            }
        }

        _currentElement = currentElement;
    }
    else {
        S2ASSERT(false, L"Different data format");
    }
}

void S2TreedArchive::_copyFrom(const S2TreedArchiveElement* peerNode)
{
    _currentElement->copyField(peerNode);

    if (peerNode->dataType() == TYPE_PATH && !peerNode->children().empty()) {
        BOOST_FOREACH(const S2TreedArchiveElement* each, peerNode->children()) {
            _addPath(each->archiveKey());
            _pathInto(each->archiveKey());
            _copyFrom(each);
            _pathOut();
        }
    }
}

S2TreedArchive& S2TreedArchive::operator= (const S2TreedArchive& archive)
{
    if (BOTH_S2_TREED_ARCHIVE_TYPE(this, archive)) {
        _rootElement->replace(_rootElement, archive.rootElement());
        _lastInsertedKey = 0;
        _currentElement = _rootElement;
    }
    else {
        S2TRACE(L"Different data format");
    }
    return *this;
}

bool S2TreedArchive::compare(const S2TreedArchive& archive, bool compareOrder) const
{
    return _rootElement->compare(*archive.rootElement(), compareOrder);
}

void S2TreedArchive::scan(Scanner* scanner) const
{
    try {
        S2TreedArchiveElement* currentElement = _currentElement;
        _currentElement = _rootElement;

        scanner->begin();

        if (currentElement->children().size() > 0) {
            scanner->_depth = 0;
            scanPath(scanner);
        }

        scanner->end();

        _currentElement = currentElement;
    }
    catch (S2Exception&) {
        throw;
    }
}

void S2TreedArchive::scanPath(Scanner* scanner) const
{
    BOOST_FOREACH(S2TreedArchiveElement* each, _currentElement->children()) {
        if (each->dataType() == TYPE_PATH) {
            scanner->itemScanned(each->archiveKey(), each->dataType(), 0);
            if (each->children().size() > 0) {
                S2TreedArchiveElement* org = _currentElement;
                _currentElement = each;
                scanner->_depth++;
                scanPath(scanner);
                scanner->_depth--;
                _currentElement = org;
            }
        }
        else {
            #define CASE_TYPE_READ_ELEM(Type, Name, type)       \
            case TYPE_##Type: {                                 \
                type val = each->to##Name();                    \
                memcpy(scanner->_buffer, &val, sizeof(type));   \
                break;                                          \
            }
            switch (each->dataType()) {
            CASE_TYPE_READ_ELEM(BOOL,     Bool,   bool)
            CASE_TYPE_READ_ELEM(INT8,     Int8,   int8)
            CASE_TYPE_READ_ELEM(UINT8,    Uint8,  uint8)
            CASE_TYPE_READ_ELEM(INT16,   Int16,   int16)
            CASE_TYPE_READ_ELEM(UINT16,  Uint16,  uint16)
            CASE_TYPE_READ_ELEM(INT32,   Int32,   int32)
            CASE_TYPE_READ_ELEM(UINT32,  Uint32,  uint32)
            CASE_TYPE_READ_ELEM(INT64,   Int64,   int64)
            CASE_TYPE_READ_ELEM(UINT64,  Uint64,  uint64)
            CASE_TYPE_READ_ELEM(FLOAT32, Float32, float32)
            CASE_TYPE_READ_ELEM(FLOAT64, Float64, float64)
            default:
                break;
            }

            scanner->itemScanned(each->archiveKey(), each->dataType(), scanner->_buffer);
        }
    }
}

void S2TreedArchive::elementParsed(S2TreedArchiveElement* parentElement, S2TreedArchiveElement* parsed)
{
    if (parentElement == parsed) {
        _rootElement = parsed;
    }
    else {
        parentElement->appendChildElement(parsed);
        parsed->setParentElement(parentElement);
    }
}

void S2TreedArchive::_preProcessWriteObjects(uint8 key)
{
    _addPath(key, true);
}

void S2TreedArchive::_postProcessWriteObjects(uint8 key, const S2String& name)
{
    uint32 count = _currentElement->arrayCount();

    if (count > 256 - 2) {
        S2TreedArchiveElement* curElem = _currentElement;

        std::vector<S2TreedArchiveElement*> children;

        for (uint8 subKey = KEY_ARRAY_DATA; count; ++subKey) {
            _pathInto(subKey);
            BOOST_FOREACH(S2TreedArchiveElement* each, const_cast<std::vector<S2TreedArchiveElement*>& >(_currentElement->children())) {
                each->setParentElement(curElem);

                each->setArchiveKey(0);
                children.push_back(each);
                --count;
            }
            _pathOut();
        }

        const_cast<std::vector<S2TreedArchiveElement*>& >(_currentElement->children()).swap(children);
    }

    _currentElement->setName(name + L"Array");
}

#include <stdio.h>
void S2TreedArchive::_preProcessReadObjects(uint8 key) const
{
    uint32 count = std::min<uint32>(_currentElement->numChildren(), MAX_OBJECT_ARRAY_ITEM);

    if (count > 256 - 2) {
        typedef std::vector<S2TreedArchiveElement*> Elements;

        Elements elements;
        elements.swap(const_cast<Elements&>(_currentElement->children()));

        assert(_currentElement->children().empty());

        Elements::iterator itr = elements.begin();
        for (uint8 subKey = KEY_ARRAY_DATA; itr != elements.end(); ++subKey) {
            const_cast<S2TreedArchive*>(this)->_addPath(subKey);
            _pathInto(subKey);
            for (uint8 objKey = 1; 0 < objKey && itr != elements.end(); ++objKey, ++itr) {
                (*itr)->setArchiveKey(objKey);
                (*itr)->setParentElement(_currentElement);
                _currentElement->appendChildElement(*itr);
            }
            _pathOut();
        }
    }
}
