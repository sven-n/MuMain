#include "stdafx.h"

#include "LoadState.h"

namespace Data::ServerList
{
LoadState GetLoadState(int percent)
{
    if (percent >= OverloadedLoadPercent)
    {
        return LoadState::Overloaded;
    }
    return percent >= FullLoadPercent ? LoadState::Full : LoadState::Open;
}
} // namespace Data::ServerList
