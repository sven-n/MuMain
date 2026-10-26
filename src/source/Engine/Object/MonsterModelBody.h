#pragma once

class CHARACTER;
class OBJECT;

namespace Data::Monsters
{
struct MonsterModelBody;
}

// What CreateCharacterPointer gives a character of a model that has an entry
// in the model list (Data/Monsters/Models.json, "boundingBox" and
// "linkBones").
namespace Engine::Object::MonsterBody
{
// The entry's bounding box; the default one (-60, -60, 0 to 50, 50, 150)
// where it names none.
void ApplyBoundingBox(OBJECT& object, const Data::Monsters::MonsterModelBody& body);

// The bones the weapons hang on: 33 and 42 where the entry names none. One
// bone sets only the right hand's; none leaves both as the character slot
// had them.
void ApplyLinkBones(CHARACTER& character, const Data::Monsters::MonsterModelBody& body);
} // namespace Engine::Object::MonsterBody
