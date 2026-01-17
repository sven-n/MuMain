// SideHair.h: interface for the new SideHair
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIDEHAIR_H__8C7DEDBA_0577_4B98_BD53_900F41EAC8AD__INCLUDED_)
#define AFX_SIDEHAIR_H__8C7DEDBA_0577_4B98_BD53_900F41EAC8AD__INCLUDED_

#pragma once

#include "ShadowVolume.h"

class CSideHair : CShadowVolume
{
public:
    CSideHair();
    virtual ~CSideHair();

    virtual void Create(vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], BMD* b, OBJECT* o, bool SkipTga = true);
    virtual void Destroy(void);
    void Render(vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], vec3_t ppLightTransformed[MAX_MESH][MAX_VERTICES]);
protected:
    void RenderLine(vec3_t v1, vec3_t v2, vec3_t c1, vec3_t c2);
};

#endif // !defined(AFX_SIDEHAIR_H__8C7DEDBA_0577_4B98_BD53_900F41EAC8AD__INCLUDED_)
