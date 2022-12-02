// Copyright (c) 2022 Ultimaker B.V.
// CuraEngine is released under the terms of the AGPLv3 or higher

#ifdef ARCUS

#include <Arcus/Error.h> //To process error codes.
#include "ccglobal/log.h"

#include "communication/Listener.h"

namespace cura52
{

void Listener::stateChanged(Arcus::SocketState)
{
    // Do nothing.
}

void Listener::messageReceived()
{
    // Do nothing.
}

void Listener::error(const Arcus::Error& error)
{
    if (error.getErrorCode() == Arcus::ErrorCode::Debug)
    {
        LOGD("{}", error.getErrorMessage());
    }
    else
    {
        LOGE("{}", error.getErrorMessage());
    }
}

} // namespace cura52

#endif // ARCUS