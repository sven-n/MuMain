#include "stdafx.h"
#include "App/Control/ControlTargets.h"

#include "Core/Text/Utf8.h"
#include "Engine/Object/ZzzCharacter.h"

#include <cwchar>

namespace App::Control::Targets
{
TargetLookup ResolveTarget(const Request& request, int& key)
{
    key = -1;
    if (!request.Has("target"))
    {
        return TargetLookup::Missing;
    }

    int id = 0;
    if (request.GetInt("target", id))
    {
        if (FindCharacterIndex(id) >= MAX_CHARACTERS_CLIENT)
        {
            return TargetLookup::NotInView;
        }
        key = id;
        return TargetLookup::Found;
    }

    std::string name;
    if (!request.GetString("target", name) || name.empty())
    {
        return TargetLookup::Malformed;
    }

    const std::wstring wanted = Core::Text::FromUtf8(name);
    for (int index = 0; index < MAX_CHARACTERS_CLIENT; ++index)
    {
        const CHARACTER& character = CharactersClient[index];
        if (character.Object.Live && wcscmp(character.ID, wanted.c_str()) == 0)
        {
            key = character.Key;
            return TargetLookup::Found;
        }
    }
    return TargetLookup::NotInView;
}

std::string TargetError(const Request& request, TargetLookup lookup, std::string_view command, bool required)
{
    switch (lookup)
    {
    case TargetLookup::Found:
        return {};
    case TargetLookup::Missing:
        return required ? App::Control::EncodeError(request.EncodedId(), App::Control::ErrorCode::BadRequest,
                                                    "`" + std::string(command) + "` needs a `target`")
                        : std::string{};
    case TargetLookup::Malformed:
        return App::Control::EncodeError(request.EncodedId(), App::Control::ErrorCode::BadRequest,
                                         "`target` is an object id or a character name");
    case TargetLookup::NotInView:
        break;
    }
    return App::Control::EncodeError(request.EncodedId(), App::Control::ErrorCode::NotInView,
                                     "the client does not see that object");
}
} // namespace App::Control::Targets
