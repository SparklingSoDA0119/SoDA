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
}

