#ifndef _SODA_S2_TREED_ARCHIVE_H
#define _SODA_S2_TREED_ARCHIVE_H

#include <S2Object.h>
#include <S2String.h>
#include <S2Archive.h>
#include <S2TreedArchiveParser.h>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace SoDA {
    class S2TreedArchiveElement;

    class S2TreedArchive : public S2Archive
                         , public S2TreedArchiveParserDelegate {
    friend class S2String;
    protected:
        S2TreedArchive(S2TreedArchiveParser* parser, bool needFree = false);

    public:
        virtual ~S2TreedArchive();
        virtual void close();

        DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS(  bool,    Bool,    BoolArray)
        DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS(  int8,    Int8,    Int8Array)
        DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS( uint8,   Uint8,   Uint8Array)
        DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS(  int16,   Int16,   Int16Array)
        DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS( uint16,  Uint16,  Uint16Array)
        DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS(  int32,   Int32,   Int32Array)
        DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS( uint32,  Uint32,  Uint32Array)
        DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS(  int64,   Int64,   Int64Array)
        DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS( uint64,  Uint64,  Uint64Array)
        DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS(float32, Float32, Float32Array)
        DECLARE_SODA_S2ARCHIVE_TYPE_MEMFUNS(float64, Float64, Float64Array)

        virtual bool readObject(uint8 key, S2Object* object) const;
        virtual void writeObject(uint8 key, const S2Object& object, const S2String& name = L"");

        virtual bool readBinary(uint8 key, void* data, uint32 size) const;
        virtual void writeBinary(uint8 key, const void* data, uint32 size, const S2String& name = L"");

        virtual const S2String& readS2String() const;
        virtual void writeS2String(uint8 key, const S2String& toWrite);

        virtual uint32 readArrayCount(uint8 key) const;
        virtual uint32 readArrayDataPosition(uint8 key) const;

        virtual bool findItem(uint8 key, uint8* type = 0, uint32* size = 0) const;
        virtual bool removeItem(uint8 key);
        virtual void setPath(const std::vector<uint8>& path) const;
        virtual void setPath(uint8 key, ...) const;

        virtual uint32 readDataSize(uint8 key) const;
        virtual uint32 readDataPosition(uint8 key) const;

        virtual void writeEmptyArray(uint8 key, const S2String& name);

        uint8 lastInsertedKey() const { return _lastInsertedKey; }

        void setRootElement(S2TreedArchiveElement* element) { _rootElement = element; }
        S2TreedArchiveElement* rootElement() const      { return _rootElement;    }
        S2TreedArchiveElement* currentElement() const   { return _currentElement; }

        S2TreedArchive& operator= (const S2TreedArchive& archive);
        bool  compare (const S2TreedArchive& archive, bool compareOrder = false) const;
        void copyFrom(const S2TreedArchive& archive, uint8 key, bool copyPath = true);

        const S2String& lastParseError() const { return _parser->lastError(); }

        void setAddRootOnLoad(bool addRootOnLoad)       { _addRootOnLoad    = addRootOnLoad;    }
        void setRemoveRootOnSave(bool removeRootOnSave) { _removeRootOnSave = removeRootOnSave; }

    protected:
        virtual void writeS2String(uint8 key, const S2String& toWrite, uint8 dataType, uint32 dataSize);
        virtual void init(S2Stream* stream, bool create);
        virtual bool findItem(uint8 key, Item* item) const;
        virtual bool findItem(uint8 key, uint8 type, uint32 size) const;

        S2TreedArchiveElement* findElement(uint8 key) const;

        virtual void scanPath(Scanner* scanner) const;

    private:
        virtual void _writeArrayCount(uint8 key, uint32 count);

        virtual bool _readArrayCountObjectsArray(uint8 key, uint32* count) const;

        virtual void _preProcessReadObjects(uint8 key) const;

        virtual void _preProcessWriteObjects(uint8 key);

        virtual void _addPathForWriteObjects(uint8 key) { _addPath(key); }

        virtual void _postProcessWriteObjects(uint8 key, const S2String& name = L"");

    private:
        virtual void _addPath(uint8 key);
        virtual void _addPath(uint8 key, bool isArray);
        virtual void _pathInto(uint8 key) const;
        virtual void _pathOut() const;

        virtual void _copyFrom(const S2TreedArchiveElement* peerNode);

    protected:
        S2TreedArchiveParser* _parser;
        bool  _needFree;
        uint8 _lastInsertedKey;
        mutable S2TreedArchiveElement* _rootElement;    // for toString
        mutable S2TreedArchiveElement* _currentElement;

        bool _addRootOnLoad;
        bool _removeRootOnSave;

    public:
        virtual void elementParsed(S2TreedArchiveElement* parentElement, S2TreedArchiveElement* parsed);
        virtual void elementSaved(const S2TreedArchiveElement* parentElement, const S2TreedArchiveElement* saved) { } // NOOP

    public:
        virtual S2String toString() const = 0;
        virtual void scan(Scanner* scanner) const;
    };
} // namespace SoDA

#endif //_SODA_S2_TREED_ARCHIVE_H

