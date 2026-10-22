#pragma once

#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/ItemData/SharedItemModel.h"

#include <span>
#include <string_view>
#include <vector>

namespace Data::Items
{
// In-memory table of the item models (Data/Items/Models), indexed by item
// type like the item database, and the shared models they name. Built at
// startup, before the models are opened; the item editor moves models with
// their items (Swap) and changes them (Set, Remove, SetSharedModel).
class ItemModelDatabase
{
public:
    static ItemModelDatabase& GetInstance();

    ItemModelDatabase();

    // Replaces all models. Each model goes to its (group, number); models
    // with invalid ids are skipped (validation reports them). The glow colors
    // of the models are looked up in `glowColors`.
    void Build(std::span<const ItemModelDefinition> models, const Effects::GlowColorList& glowColors);

    // Returns nullptr for invalid ids and for items without a model.
    const ItemModelDefinition* Find(int itemType) const
    {
        if (!IsValidItemType(itemType))
        {
            return nullptr;
        }

        const ItemModelDefinition& model = m_models[itemType];
        return model.Exists() ? &model : nullptr;
    }

    // The glow colors of the model; nullptr for invalid ids and for items
    // without a model.
    const ItemGlowColors* FindGlowColors(int itemType) const
    {
        return Find(itemType) != nullptr ? &m_glowColors[itemType] : nullptr;
    }

    // Swaps two slots and updates the ids of the moved models, for the item
    // editor, which moves an item's model with the item.
    void Swap(int firstItemType, int secondItemType);

    // Puts the model at its (group, number), with its glow colors looked up
    // in `glowColors`; an entry without a file and a shared model empties
    // the slot. Invalid ids are ignored. For the item editor.
    void Set(const ItemModelDefinition& model, const Effects::GlowColorList& glowColors);
    void Remove(int itemType);
#ifdef _EDITOR
    // Looks the glow colors of every model up in `glowColors` again, after
    // the item editor changed the list.
    void ResolveGlowColors(const Effects::GlowColorList& glowColors);
#endif

    // The shared models (SharedModels.json), as loaded; the model entries
    // that name one have its file, texture folders and none-blend meshes
    // filled in.
    void SetSharedModels(std::span<const SharedItemModel> sharedModels);
    std::span<const SharedItemModel> GetSharedModels() const
    {
        return m_sharedModels;
    }
    // Returns nullptr for names that are not shared models.
    const SharedItemModel* FindSharedModel(std::string_view name) const;
    // Adds the shared model or replaces the one of its name; the model
    // entries that name it are not changed. For the item editor.
    void SetSharedModel(const SharedItemModel& sharedModel);
    void RemoveSharedModel(std::string_view name);

    // All MAX_ITEM slots, indexed by item type, including empty ones.
    std::span<const ItemModelDefinition> GetAllSlots() const
    {
        return m_models;
    }

    int GetModelCount() const
    {
        return m_modelCount;
    }

    // Counts the changes (builds, swaps, sets, removes and shared model
    // changes); tables taken from the models (e.g. the render styles) take
    // them again when it changes.
    int GetVersion() const
    {
        return m_version;
    }

private:
    std::vector<ItemModelDefinition> m_models;
    std::vector<ItemGlowColors> m_glowColors;
    std::vector<SharedItemModel> m_sharedModels;
    int m_modelCount = 0;
    int m_version = 0;
};
} // namespace Data::Items

#define g_ItemModelDatabase Data::Items::ItemModelDatabase::GetInstance()
