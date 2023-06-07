#include "CallbackAPI.h"
#include "IOCPImpl.h"

CallBackAPI::RequestAPI* __cdecl CreateAPI()
{
    return new IOCPImpl();
}

