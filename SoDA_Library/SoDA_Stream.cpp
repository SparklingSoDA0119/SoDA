#include "SoDA_Stream.h"

#define SODA_TRACE_NAME L"[SoDA_Stream]"
#include "SoDA_Debug.h"

#include <algorithm>

using namespace SoDA;

SoDA_Stream::SoDA_Stream()
    : _byteOrder(SODA_LITTLE_ENDIAN), _needsSwapEndian(false), _mark(0)
{
    setByteOrder(SODA_LITTLE_ENDIAN);
}

void SoDA_Stream::setByteOrder(SoDA_ByteOrder byteOrder)
{
    _byteOrder       = byteOrder;
    _needsSwapEndian = (_byteOrder != SoDA_Endian::systemByteOrder());
}

#define DEFINE_SODA_STREAM_TYPE_MEMFUNS(T, Name, ArrayName)                         \
    void SoDA_Stream::read##Name(T* data) const { read(data, sizeof(T)); }          \
    void SoDA_Stream::read##ArrayName(T* array, uint32 cont) const                  \
    { read(array, sizeof(T) * count); }                                             \
    void SoDA_Stream::write##Name(T data) { write(&data, sizeof(T)); }              \
    void SoDA_Stream::write##ArrayName(const T* array, uint32 count)                \
    { write(array, sizeof(T) * count); }

#define DEFINE_SODA_STREAM_TYPE_MEMFUNS_ENDIAN(T, Name, ArrayName)                  \
    void SoDA_Stream::read##Name(T* data) const                                     \
    {                                                                               \
        read(data, sizeof(T));                                                      \
        if (_needsSwapEndian) {                                                     \
            *data = SoDA_Endian::swap##Name(*data);                                 \
        }                                                                           \
    }                                                                               \
                                                                                    \
    void SoDA_Stream::read##ArrayName(T* array, uint32 count) const                 \
    {                                                                               \
        read(array, sizeof(T) * count);                                             \
        if (_needsSwapEndian) {                                                     \
            std::transform(array, array + count, array, &SoDA_Endian::swap##Name);  \
        }                                                                           \
    }                                                                               \
                                                                                    \
    void SoDA_Stream::write##Name(T data)                                           \
    {                                                                               \
        if (_needsSwapEndian) {                                                     \
            data = SoDA_Endian::swap##Name(data);                                   \
        }                                                                           \
        write(&data, sizeof(T));                                                    \
    }                                                                               \
                                                                                    \
    void SoDA_Stream::write##ArrayName(const T* array, uint32 count)                \
    {                                                                               \
        if (_needsSwapEndian) {                                                     \
            T data;                                                                 \
            for (uint32 i = 0; i < count; i++) {                                    \
                data = G2Endian::swap##Name(array[i]);                              \
                write(&data, sizeof(T));                                            \
            }                                                                       \
        }                                                                           \
        else {                                                                      \
            write(array, sizeof(T) * count);                                        \
        }                                                                           \
    }

DEFINE_SODA_STREAM_TYPE_MEMFUNS(         bool,    Bool,    BoolArray)
DEFINE_SODA_STREAM_TYPE_MEMFUNS(         int8,    Int8,    Int8Array)
DEFINE_SODA_STREAM_TYPE_MEMFUNS(        uint8,   Uint8,   Uint8Array)
DEFINE_SODA_STREAM_TYPE_MEMFUNS_ENDIAN(  int16,   Int16,   Int16Array)
DEFINE_SODA_STREAM_TYPE_MEMFUNS_ENDIAN( uint16,  Uint16,  Uint16Array)
DEFINE_SODA_STREAM_TYPE_MEMFUNS_ENDIAN(  int32,   Int32,   Int32Array)
DEFINE_SODA_STREAM_TYPE_MEMFUNS_ENDIAN( uint32,  Uint32,  Uint32Array)
DEFINE_SODA_STREAM_TYPE_MEMFUNS_ENDIAN(  int64,   Int64,   Int64Array)
DEFINE_SODA_STREAM_TYPE_MEMFUNS_ENDIAN( uint64,  Uint64,  Uint64Array)
DEFINE_SODA_STREAM_TYPE_MEMFUNS_ENDIAN(float32, Float32, Float32Array)
DEFINE_SODA_STREAM_TYPE_MEMFUNS_ENDIAN(float64, Float64, Float64Array)

void SoDA_Stream::swap(SoDA_Stream& stream)
{
    std::swap(_byteOrder, stream._byteOrder);
    std::swap(_needsSwapEndian, stream._needSwapEndian);
    std::swap(_mark, stream._mark);
}
