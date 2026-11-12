#include "stdafx.h"
#include <ctype.h>
#include "ZzzBMD.h"
#include "SMD.h"
#include "Core/Utilities/ReadScript.h"


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
        if (Token == NAME && strcmp("nodes", TokenString) == 0) break;
    }
    while (true)
    {
        Token = (*GetToken)();
        if (Token == END) break;
        if (Token == NAME && strcmp("end", TokenString) == 0) break;
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
        if (Token == NAME && strcmp("skeleton", TokenString) == 0) break;
    }
    while (true)
    {
        Token = (*GetToken)();
        if (Token == END) break;
        if (Token == NAME)
        {
            if (strcmp("end", TokenString) == 0) break;
            if (strcmp("time", TokenString) == 0)
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
        if (Token == NAME && strcmp("skeleton", TokenString) == 0) break;
    }

    SkeletonGroup_t* sg = &SkeletonGroup;
    sg->TimeNum = 0;
    while (true)
    {
        Token = (*GetToken)();
        if (Token == END) break;
        if (Token == NAME)
        {
            if (strcmp("end", TokenString) == 0) break;
            if (strcmp("time", TokenString) == 0)
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
        if (Token == NAME && strcmp("triangles", TokenString) == 0) break;
    }

    TriangleGroup_t* tg = &TriangleGroup;
    tg->TriangleNum = 0;
    while (true)
    {
        Token = (*GetToken)();
        if (Token == END) break;
        if (Token == NAME)
        {
            if (strcmp("end", TokenString) == 0) break;
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
