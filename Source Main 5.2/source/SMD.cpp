#include "stdafx.h"
#include <ctype.h>
#include "ZzzBMD.h"
#include "SMD.h"
#include "ReadScript.h"


NodeGroup_t     NodeGroup;
SkeletonGroup_t SkeletonGroup;
TriangleGroup_t TriangleGroup;
SMDMeshGroup_t  MeshGroup;

void ParseNodes()
{
    SMDToken Token;
    NodeGroup_t* ng = &NodeGroup;
    ng->NodeNum = 0;

    while (true)
    {
        Token = (*GetToken)();
        if (Token == END) break;
        if (Token == NAME && strcmp("nodes", TokenString) == NULL) break;
    }
    while (true)
    {
        Token = (*GetToken)();
        if (Token == END) break;
        if (Token == NAME && strcmp("end", TokenString) == NULL) break;
        if (Token == NUMBER)
        {
            Node_t* n = &ng->Node[ng->NodeNum];
            Token = (*GetToken)(); strcpy(n->Name, TokenString);
            Token = (*GetToken)(); n->Parent = (short)TokenNumber;
        }
        ng->NodeNum++;
    }
    while (true)
    {
        Token = (*GetToken)();
        if (Token == END) break;
        if (Token == NAME && strcmp("skeleton", TokenString) == NULL) break;
    }
    while (true)
    {
        Token = (*GetToken)();
        if (Token == END) break;
        if (Token == NAME)
        {
            if (strcmp("end", TokenString) == NULL) break;
            if (strcmp("time", TokenString) == NULL)
            {
                Token = (*GetToken)();
                Skeleton_t* s = &ng->Skeleton;
                for (int i = 0; i < ng->NodeNum; i++)
                {
                    Token = (*GetToken)();
                    Token = (*GetToken)(); s->Position[i][0] = TokenNumber;
                    Token = (*GetToken)(); s->Position[i][1] = TokenNumber;
                    Token = (*GetToken)(); s->Position[i][2] = TokenNumber;
                    Token = (*GetToken)(); s->Rotation[i][0] = TokenNumber;
                    Token = (*GetToken)(); s->Rotation[i][1] = TokenNumber;
                    Token = (*GetToken)(); s->Rotation[i][2] = TokenNumber;
                }
            }
        }
    }
}

void ParseSkeleton()
{
    SMDToken Token;
    while (true)
    {
        Token = (*GetToken)();
        if (Token == END) break;
        if (Token == NAME && strcmp("skeleton", TokenString) == NULL) break;
    }

    SkeletonGroup_t* sg = &SkeletonGroup;
    sg->TimeNum = 0;
    while (true)
    {
        Token = (*GetToken)();
        if (Token == END) break;
        if (Token == NAME)
        {
            if (strcmp("end", TokenString) == NULL) break;
            if (strcmp("time", TokenString) == NULL)
            {
                Token = (*GetToken)();
                int TimeNum = (int)TokenNumber;
                Skeleton_t* s = &sg->Skeleton[TimeNum];
                for (int i = 0; i < NodeGroup.NodeNum; i++)
                {
                    Token = (*GetToken)();
                    Token = (*GetToken)(); s->Position[i][0] = TokenNumber;
                    Token = (*GetToken)(); s->Position[i][1] = TokenNumber;
                    Token = (*GetToken)(); s->Position[i][2] = TokenNumber;
                    Token = (*GetToken)(); s->Rotation[i][0] = TokenNumber;
                    Token = (*GetToken)(); s->Rotation[i][1] = TokenNumber;
                    Token = (*GetToken)(); s->Rotation[i][2] = TokenNumber;
                }
                sg->TimeNum++;
            }
        }
    }
}

void ParseTriangles(bool Flip)
{
    SMDToken Token;
    while (true)
    {
        Token = (*GetToken)();
        if (Token == END) break;
        if (Token == NAME && strcmp("triangles", TokenString) == NULL) break;
    }

    TriangleGroup_t* tg = &TriangleGroup;
    tg->TriangleNum = 0;
    while (true)
    {
        Token = (*GetToken)();
        if (Token == END) break;
        if (Token == NAME)
        {
            if (strcmp("end", TokenString) == NULL) break;
            strcpy(tg->TextureName[tg->TriangleNum], TokenString);
            if (!Flip)
            {
                for (int i = 0; i < 3; i++)
                {
                    SMDVertex_t* v = &tg->Vertex[tg->TriangleNum][i];
                    Token = (*GetToken)(); v->Node = (short)TokenNumber;
                    Token = (*GetToken)(); v->Position[0] = TokenNumber;
                    Token = (*GetToken)(); v->Position[1] = TokenNumber;
                    Token = (*GetToken)(); v->Position[2] = TokenNumber;
                    Token = (*GetToken)(); v->Normal[0] = TokenNumber;
                    Token = (*GetToken)(); v->Normal[1] = TokenNumber;
                    Token = (*GetToken)(); v->Normal[2] = TokenNumber;
                    Token = (*GetToken)(); v->TexCoordU = TokenNumber;
                    Token = (*GetToken)(); v->TexCoordV = 1.f - TokenNumber;
                }
            }
            else
            {
                for (int i = 2; i >= 0; i--)
                {
                    SMDVertex_t* v = &tg->Vertex[tg->TriangleNum][i];
                    Token = (*GetToken)(); v->Node = (short)TokenNumber;
                    Token = (*GetToken)(); v->Position[0] = TokenNumber;
                    Token = (*GetToken)(); v->Position[1] = TokenNumber;
                    Token = (*GetToken)(); v->Position[2] = TokenNumber;
                    Token = (*GetToken)(); v->Normal[0] = TokenNumber;
                    Token = (*GetToken)(); v->Normal[1] = TokenNumber;
                    Token = (*GetToken)(); v->Normal[2] = TokenNumber;
                    Token = (*GetToken)(); v->TexCoordU = TokenNumber;
                    Token = (*GetToken)(); v->TexCoordV = 1.f - TokenNumber;
                }
            }
            tg->TriangleNum++;
        }
    }
}

bool OpenSMDFile(wchar_t* FileName, int Type, bool Flip)
{
    if (FileName == NULL) return false;
    if ((SMDFile = _wfopen(FileName, L"rb")) == NULL)
    {
#ifdef _DEBUG
        extern HWND g_hWnd;
        wchar_t Text[1024];
        swprintf(Text, L"%s - File not exist..\r\n", FileName);
        g_ErrorReport.Write(Text);
        MessageBox(g_hWnd, Text, NULL, MB_OK);
#endif
        return false;
    }

    (*GetToken)();
    (*GetToken)();

    if (Type == REFERENCE_FRAME)
    {
        ParseNodes();
        ParseTriangles(Flip);
    }
    if (Type == SKELETAL_ANIMATION)
    {
        ParseSkeleton();
    }

    fclose(SMDFile);
    return true;
}

void FixupSMD();
void Triangle2Strip();
void SMD2BMDModel(int ID, int Actions);
void SMD2BMDAnimation(int ID, bool LockPosition);

bool OpenSMDModel(int ID, wchar_t* FileName1, int Actions, bool Flip)
{
    if (Models[ID].NumMeshs > 0) return false;

    if (OpenSMDFile(FileName1, REFERENCE_FRAME, Flip))
    {
        WideCharToMultiByte(CP_UTF8, 0, FileName1, wcslen(FileName1), Models[ID].Name, 32, 0, 0);
        Models[ID].Version = 10;
        FixupSMD();
        SMD2BMDModel(ID, Actions);
        return true;
    }
    return false;
}

bool OpenSMDAnimation(int ID, wchar_t* FileName2, bool LockPosition)
{
    if (Models[ID].NumBones > 0)
    {
        if (OpenSMDFile(FileName2, SKELETAL_ANIMATION, false))
        {
            SMD2BMDAnimation(ID, LockPosition);
            return true;
        }
    }
    return false;
}