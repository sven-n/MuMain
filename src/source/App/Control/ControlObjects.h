// What the control socket knows about the objects around the character.
//
// This is the one place that reads the engine's object tables for the control
// layer, so the event ring and the protocol stay free of game globals.
#pragma once

#include "App/Control/ControlEvents.h"

namespace App::Control
{
// Describes a character, monster, NPC or pet by its server-assigned key.
// An object the client does not know is reported with `known == false`,
// its id and kind `unknown`.
[[nodiscard]] Events::ObjectDescription DescribeGameObject(int key);
} // namespace App::Control
