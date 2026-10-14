//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
#ifndef __NPC_BREEDER_H__
#define __NPC_BREEDER_H__

namespace npcBreeder
{
int CalcRecoveryZen(BYTE type, wchar_t (&Text)[100]);
void RecoverPet(BYTE type);
} // namespace npcBreeder

using namespace npcBreeder;

#endif // __NPC_BREEDER_H__