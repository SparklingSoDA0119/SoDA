#ifndef _SODA_OBJECT_H
#define _SODA_OBJECT_H

#include <iostream>

#include <SoDA_Type.h>

#if defined(_WIN32_WCE)
    // WinCE do not support std::abort() function. So that is defined here.
    void abort(void);
#endif

namespace SoDA {

class S2Archive;
class S2Stream;
class S2Invocation;
class S2MemoryStream;
class S2Port;
class S2Exception;
class S2String;

class S2Object {
public :
    S2Object() {}
    virtual ~S2Object() = 0;

    virtual S2Object* clone() const { return NULL; }
    
    virtual void archiveTo      (S2Archive* archive) const {}
    virtual void unarchiveFrom  (S2Archive* archive)       {}
    virtual void serializeTo    (S2Stream*  stream)        {}
    virtual void deserializeFrom(S2Stream*  stream)        {}

protected :
    virtual void exceptionCaught        (S2Exception&      e);
    virtual void objectExceptionCaught  (S2Object&         e);
    virtual void stdExceptionCaught     (std::exception& e);
    virtual void unknownExceptionCaught ();
    virtual bool performWithoutException(void (S2Object::*func)());
#define performWidthoutException(func)     performWithoutException(static_cast<void (Object::*)()>(func))
    
protected :
    virtual bool invoke(const S2Invocation& invocation, S2MemoryStream* result) { return true; }
    
    void invokeAndRespond(uint32 invocationId, const S2Invocation& invocation,
                          S2MemoryStream* result, S2Port* responsePort);

public :
    virtual void performInvocation(uint32 invocationId, 
                                   const S2Invocation& invocation,
                                   S2Port* responsePort);

public :
    virtual size_t hash() const { return 0; }

    virtual S2String toString() const;
    
};

inline S2Object::~S2Object() {}
}       // namespace SoDA

namespace boost {
}       // namespace boost

std::wostream& operator<<(std::wostream& stream, const SoDA::S2Object& object);
std::ostream&  operator<<(std::ostream&  stream, const SoDA::S2Object& object);

#endif      // _SODA_OBJECT_H
