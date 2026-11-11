#pragma once

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Types.h>

#include <vector>

// What the cash shop's dialogs (CMsgBoxIGSBuyPackageItem, CMsgBoxIGSBuySelectItem,
// CMsgBoxIGSSendGift) share in their documents: the row of buttons, whose clicks a dialog takes in
// its Update() and sends as its own message box event, and the developer lines (FOR_WORK) right of
// the dialog.
namespace GameShop
{
struct DialogButton
{
    Rml::String label;
    bool enabled = true;

    bool operator==(const DialogButton&) const = default;
};

// Binds `buttons`, `debug_lines` and igs_dialog_button(i) on a dialog's model, and registers the
// string list type the dialog's own lists use too. `Model` has std::vector<DialogButton> buttons and
// std::vector<Rml::String> debugLines; a click sets `pressed` to the button's index.
template <typename Model> void BindDialogCommon(Rml::DataModelConstructor& c, Model& model, int& pressed)
{
    auto button = c.RegisterStruct<DialogButton>();
    button.RegisterMember("label", &DialogButton::label);
    button.RegisterMember("enabled", &DialogButton::enabled);
    c.RegisterArray<std::vector<DialogButton>>();
    c.RegisterArray<std::vector<Rml::String>>();
    c.Bind("buttons", &model.buttons);
    c.Bind("debug_lines", &model.debugLines);
    c.BindEventCallback("igs_dialog_button",
                        [&pressed](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                        {
                            if (args.size() == 1)
                                pressed = args[0].Get<int>(-1);
                        });
}
} // namespace GameShop
