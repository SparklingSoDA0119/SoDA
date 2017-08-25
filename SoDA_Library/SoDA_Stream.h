#ifndef _SODA_STREAM_H_
#define _SODA_STREAM_H_

# pragma once

#include "SoDA_Object.h"
#include "SoDA_Endian.h"

namespace SoDA {
    class SoDA_Stream : public SoDA_Object {
    public :
        SoDA_Stream();
        virtual ~SoDA_Stream() {}

        // About Endian
    private :
        SoDA_ByteOrder _byteOrder;
        bool _needsSwapEndian;

    public :
        SoDA_ByteOrder byteOrder() const { return _byteOrder; }

    private :
        uint32 _mark;
    public :
        void mark()       { _mark = position(); }
        void moveToMark() { moveTo(_mark); }

    public :
        virtual bool   isValid()  const = 0;
        virtual uint32 remains()  const      { return size() - position(); }
        virtual uint32 size()     const = 0;
        virtual uint32 position() const = 0;

        virtual void moveTo(uint32 position)        const = 0;
        virtual void write(const void* data, uint32 size) = 0;
        virtual void fill(uint8 data, uint32 size)        = 0;
        virtual void clear()                              = 0;

    public :
#define DECLARE_SODA_STREAM_TYPE_MEMFUNS(T, Name, ArrayName) \
        T read##Name() const { T t; read##Name(&t); return t; } \
        void read##Name      (T* data)                const; \
        void read##ArrayName (T* array, uint32 count) const; \
        void write##Name     (T data); \
        void write#ArraryName(const T* array, uint32 count);

        DECLARE_SODA_STREAM_TYPE_MEMFUNS(   bool,    Bool,    BoolArray)
        DECLARE_SODA_STREAM_TYPE_MEMFUNS(   int8,    Int8,    Int8Array)
        DECLARE_SODA_STREAM_TYPE_MEMFUNS(  uint8,   Uint8,   Uint8Array)
        DECLARE_SODA_STREAM_TYPE_MEMFUNS(  int16,   Int16,   Int16Array)
        DECLARE_SODA_STREAM_TYPE_MEMFUNS( uint16,  Uint16,  Uint16Array)
        DECLARE_SODA_STREAM_TYPE_MEMFUNS(  int32,   Int32,   Int32Array)
        DECLARE_SODA_STREAM_TYPE_MEMFUNS( uint32,  Uint32,  Uint32Array)
        DECLARE_SODA_STREAM_TYPE_MEMFUNS(  int64,   int64,   Int64Array)
        DECLARE_SODA_STREAM_TYPE_MEMFUNS( uint64,  Uint32,  Uint64Array)
        DECLARE_SODA_STREAM_TYPE_MEMFUNS(float32, float32, Float32Array)
        DECLARE_SODA_STREAM_TYPE_MEMFUNS(float64, float64, Float64Array)

        void readObject (SoDA_Object* object) const;
        void writeObject(const SoDA_Object& object);

    private :
        SoDA_Stream(const SoDA_Stream&);
        SoDA_Stream operator=(const SoDA_Stream&);

    protected :
        void swap(SoDA_Stream& stream);
    };  // class SoDA_Stream

    inline void SoDA_Stream::readObject(SoDA_Object* object) const
    {
        object->deserializeFrom(const_cast<SoDA_Stream*>(this));
    }

    inline void SoDA_Steam::writeObject(const SoDA_Object& object) const
    {
        object.serializeTo(const_cast<SoDA_Object*>(this));
    }
} // namespace SoDA

#endif // _SODA_STREAM_H_
