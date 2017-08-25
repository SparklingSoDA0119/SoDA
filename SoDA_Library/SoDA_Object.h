#ifndef _SODA_OBJECT_H_
#define _SODA_OBJECT_H_

# pragma once

#include "G2Type.h"
#include <ostream>

namespace SoDA {
    
    // description : It is base class of all SoDA class
    class SoDA_Object {
    public :
        SoDA_Object () {}
        virtual ~SoDA_Object() = 0;
        virtual ~SoDA_Object* clone() const { return NULL; }

    public :
        virtual void archiveTo    (SoDA_Archive* archive) const {}
        virtual void unarchiveFrom(SoDA_Archvie* archive) {}

        virtual void serializeTo    (SoDA_Stream* stream) const {}
        virtual void deserializeFrom(SoDA_Stream* stream) {}

    protected :
        virtual void exceptionCaught      (SoDA_Exception&    e);
        virtual void objectExceptionCaught(SoDA_Object&       e);
        virtual void stdExceptionCaught   (std::exception&    e);
        virtual void unknownException     ();

    public :
        virtual void performWithoutException(void (SoDA_Object::*func)());
#define performWithoutException(func)   \
        performWithoutException(static_cast<void (SoDA_Object::*)()>(func))

    protected :
        // remote calling
        virtual bool invoke(const SoDA_Invocation& invocation, \
                            SoDA_MemoryStream* result) { return true; }
        // operate remote calling and send ack
        void invokeAndRespond(uint32 invocationId, \
                              const G2Invocation& invocation, \
                              SoDA_MemoryStream* result, \
                              SoDA_Port*         responsePort);

    public :
        // operate remot calling
        virtual void performInvocation(uint32 invocationId, \
                                       const SoDA_Invocation& invocation, \
                                       SoDA_Port* responsePort);

    public :
        // It return Hash value for "boost::unordered * container"
        virtual size_t hash() const { return 0; }

    // For Debuging
    public :
        virtual SoDA_String toString() const;
    };  // SoDA_Object

    inline SoDA_Object::~SoDA_Object() {}

}   // namespace SoDA

#endif // _SODA_OBJECT_H_
