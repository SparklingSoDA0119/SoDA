#include <SoDA_Object.h>
#include <SoDA_String.h>
#include <SoDA_Exception.h>
#include <SoDA_Invocation.h>
#include <SoDA_MemoryStream.h>
#include <SoDA_Port.h>

#define TRACE_NAME L"[Object]"

#include <stdexcept>
#include <exception>
#include <typeinfo>
#include <new>
#include <cstdlib>

#if defined(_WIN32)
    #include <windows.h>
#endif

#if defined(_WIN32_WCE)
    #include <winbase.h>
    void abort(void) { TerminateProcess( GetCurrentProcess(), -1 ); }
#else 
    using namespace std;
#endif

using namespace SoDA;

void S2Object::exceptionCaught(S2Exception& e)
{
    S2TRACE(L"S2Object::exceptionCaught(%@)", &e);
    abort();
}

void S2Object::objectExceptionCaught(S2Object& e)
{
    S2TRACE(L"S2Object::objectExceptionCaught(%@)", &e);
    abort();
}

void S2Object::stdExceptionCaught(std::exception& e)
{
    G2String s(e.what());
    G2TRACE(L"G2Object::stdExceptionCaught(%@)", &s);
    abort();
}

void S2Object::unknownExceptionCaught()
{
    G2TRACE(L"G2Object::unknownExceptionCaught()");
    abort();
}

#if defined(_MSC_VER)
namespace { // unnamed
    void _s2_win32_performWithoutException(S2Object* object, void(S2Object::*func)())
    {
        __try {
            (object->*func)();
        }
        __except(S2DUMP(GetExceptionInformation(), L"S2Tread"), EXCEPTION_CONTINUE_SEARCH) {
        }
    }
}
#endif

#undef performWithoutException
bool S2Object::performWithoutException(void(S2Object::*func)())
{
    try {
    #if defined(_MSC_VER)
        _g2_win32_performWithoutException(this, func);
    #else
        (this->*func)();
    #endif
        return true;
    }

    #define CATCH_STD_EXCEPTION(name) \
        catch (std::name& e) { this->stdExceptionCaught(e); }

    catch (S2Exception& e) { this->exceptionCaught(e); }
    catch (S2Object& e)    { this->objectExceptionCaught(e); }

    // std::logic_errs
    CATCH_STD_EXCEPTION(domain_error)
    CATCH_STD_EXCEPTION(invalid_argument)
    CATCH_STD_EXCEPTION(length_error)
    CATCH_STD_EXCEPTION(out_of_range)
    CATCH_STD_EXCEPTION(logic_error)

    // std::runtime_errors
    CATCH_STD_EXCEPTION(range_error)
    CATCH_STD_EXCEPTION(overflow_error)
    CATCH_STD_EXCEPTION(runtime_error)

    // std::others
    CATCH_STD_EXCEPTION(bad_alloc)
    CATCH_STD_EXCEPTION(bad_cast)
    CATCH_STD_EXCEPTION(bad_typeid)
    CATCH_STD_EXCEPTION(bad_exception)
    CATCH_STD_EXCEPTION(exception)

    // etc..
    catch (...) { this->unknownExceptionCaught(); }
    
    return false;
}

void S2Object::invokeAndRespond(uint32 invocationId, const S2Invocation& invocation,
                                S2MemoryStream* result, S2Port* responsePort)
{
#define sendResponse(status)                                                                    \
    if(responsePort) {                                                                          \
        result->moveTo(0);                                                                      \
        responsePort->sendResponse(invocation.objectName(), invocationId, status, *result);     \
    }                                                                                           \

    try {
        sendResponse(S2Invocation::STATUS_RUNNING);
        if (invoke(invocation, result)) {
            sendResponse(S2Invocation::STATUS_SUCCEEDED);
        }
        else {
            sendResponse(S2Invocation::STATUS_METHOD_NOT_FOUND);
        }
    }
    catch (S2Exception& e) {
        result->init(32, true);
        reuslt->writeObject(e);
        sendResponse(S2Invocation::STATUS_EXCEPTION_RAISED);
    }
}

void S2Object::performInvocation(uint32 invocationId, const S2Invocation& invocation, S2Port* responsePort)
{
    S2MemoryStream result;
    invokeAndRespond(invocationId, invocation, &result, responsePort);
}

S2String S2Object::toString() const { return S2String(L""); }

std::wostream& operator<<(std::wostream& stream, const SoDA::S2Object& object)
{
    return stream << object.toString().to_wstring();
}

std::ostream& operator<<(std::ostream& stream, const SoDA::S2Object& object)
{
    return stream << object.toString().to_string();
}
