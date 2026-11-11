#include "stdafx.h"

#ifdef _EDITOR

#include "ScopedOffscreenCapture.h"

#include "Render/Renderer/MuRenderer.h"

namespace MuEditor
{
ScopedOffscreenCapture::~ScopedOffscreenCapture()
{
    mu::GetRenderer().EndOffscreenCapture();
}
} // namespace MuEditor

#endif // _EDITOR
