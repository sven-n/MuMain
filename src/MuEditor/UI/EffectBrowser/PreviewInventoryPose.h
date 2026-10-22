#pragma once

#ifdef _EDITOR

namespace MuEditor::Effects
{
// The matrix (a ZzzMathLib 3x4 matrix, no translation) that puts an item posed
// as the inventory shows it into the preview's Z-up scene. The inventory views
// its items with x to the right, y up and z toward the viewer; in the scene, y
// becomes up (+Z) and z points to where the preview camera first looks from
// (EffectPreviewCamera::DefaultYawDegrees), turned `turnDegrees` around Z. So
// the item stands as it does in the inventory, facing the camera.
void InventoryViewToScene(float turnDegrees, float matrix[3][4]);
} // namespace MuEditor::Effects

#endif // _EDITOR
