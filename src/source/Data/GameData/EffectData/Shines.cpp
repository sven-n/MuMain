#include "stdafx.h"

#include "Shines.h"

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemModelJsonValues.h"
#include "Data/GameData/LookData/LookJson.h"

#include <algorithm>
#include <cmath>
#include <set>

namespace Data::Effects
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

constexpr const char* ShinesKey = "shines";
constexpr const char* NameKey = "name";
constexpr const char* LightKey = "light";
constexpr const char* PassesKey = "passes";
constexpr const char* KindKey = "kind";
constexpr const char* MotionKey = "motion";
constexpr const char* OpacityKey = "opacity";
constexpr const char* BrightnessKey = "brightness";
constexpr const char* TextureKind = "texture";
constexpr const char* ReflectionKind = "reflection";

class Reader
{
public:
    Reader(const std::string& source, std::vector<ItemDataIssue>& issues) : m_source(source), m_issues(issues) {}

    void Fatal(const std::string& field, const std::string& message) const
    {
        Items::Json::AddFileIssue(m_issues, m_source, ItemDataIssue::NoItem, field, message);
    }

    void WarnAboutUnknownKeys(const OrderedJson& json, const std::string& field,
                              std::initializer_list<std::string_view> keys) const
    {
        Items::Json::KeySet known;
        for (const std::string_view key : keys)
        {
            known.emplace(key);
        }
        Items::Json::WarnAboutUnknownKeys(json, known, field.empty() ? "" : field + ".",
                                          [this](const std::string& unknown, const std::string& message)
                                          {
                                              m_issues.push_back({ItemDataIssueSeverity::Warning, m_source,
                                                                  ItemDataIssue::NoItem, ItemDataIssue::NoItem, unknown,
                                                                  message});
                                          });
    }

    // A number of 0 or more; false (with a problem) for anything else.
    bool ReadAmount(const OrderedJson& json, const std::string& field, double& value) const
    {
        if (!json.is_number() || !std::isfinite(json.get<double>()) || json.get<double>() < 0.0)
        {
            Fatal(field, "must be a number of 0 or more");
            return false;
        }
        value = json.get<double>();
        return true;
    }

    bool ReadPass(const OrderedJson& json, const std::string& field, ShinePass& pass) const
    {
        if (!json.is_object())
        {
            Fatal(field, "must be an object");
            return false;
        }
        bool ok = true;
        const auto kind = json.find(KindKey);
        if (kind == json.end() || !kind->is_string() || (*kind != TextureKind && *kind != ReflectionKind))
        {
            Fatal(field + "." + KindKey, std::string("must be \"") + TextureKind + "\" or \"" + ReflectionKind + "\"");
            ok = false;
        }
        else
        {
            pass.reflection = *kind == ReflectionKind;
        }
        if (const auto motion = json.find(MotionKey); motion != json.end())
        {
            const std::optional<Looks::Motion> named =
                motion->is_string() ? Looks::MotionNamed(motion->get_ref<const std::string&>()) : std::nullopt;
            if (!named)
            {
                Fatal(field + "." + MotionKey, "must be one of " + Looks::MotionNameList());
                ok = false;
            }
            else if (!pass.reflection)
            {
                Fatal(field + "." + MotionKey, "is only for a reflection");
                ok = false;
            }
            else
            {
                pass.motion = *named;
            }
        }
        if (const auto opacity = json.find(OpacityKey); opacity != json.end())
        {
            ok = ReadAmount(*opacity, field + "." + OpacityKey, pass.opacity) && ok;
        }
        if (const auto brightness = json.find(BrightnessKey); brightness != json.end())
        {
            ok = ReadAmount(*brightness, field + "." + BrightnessKey, pass.brightness) && ok;
        }
        WarnAboutUnknownKeys(json, field, {KindKey, MotionKey, OpacityKey, BrightnessKey});
        return ok;
    }

    bool ReadShine(const OrderedJson& json, const std::string& field, Shine& shine) const
    {
        if (!json.is_object())
        {
            Fatal(field, "must be an object");
            return false;
        }
        bool ok = Items::Json::ReadNameField(json, NameKey, field, shine.name,
                                             [this](const std::string& at, const std::string& message)
                                             { Fatal(at, message); });
        if (const auto light = json.find(LightKey); light != json.end())
        {
            double factor = 0.0;
            if (ReadAmount(*light, field + "." + LightKey, factor))
            {
                shine.light = factor;
            }
            else
            {
                ok = false;
            }
        }
        const auto passes = json.find(PassesKey);
        if (passes == json.end() || !passes->is_array())
        {
            Fatal(field + "." + PassesKey, "missing or not a list of passes");
            ok = false;
        }
        else
        {
            for (size_t i = 0; i < passes->size(); ++i)
            {
                ShinePass pass;
                if (ReadPass((*passes)[i], field + "." + PassesKey + "[" + std::to_string(i) + "]", pass))
                {
                    shine.passes.push_back(pass);
                }
                else
                {
                    ok = false;
                }
            }
        }
        WarnAboutUnknownKeys(json, field, {NameKey, LightKey, PassesKey});
        return ok;
    }

private:
    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
};

OrderedJson WritePass(const ShinePass& pass)
{
    OrderedJson json = OrderedJson::object();
    json[KindKey] = pass.reflection ? ReflectionKind : TextureKind;
    if (pass.reflection && pass.motion != Looks::Motion::SlowDrift)
    {
        json[MotionKey] = Looks::MotionName(pass.motion);
    }
    if (pass.opacity != 1.0)
    {
        json[OpacityKey] = Items::ModelJson::WriteNumber(pass.opacity);
    }
    if (pass.brightness != 1.0)
    {
        json[BrightnessKey] = Items::ModelJson::WriteNumber(pass.brightness);
    }
    return json;
}
} // namespace

void ReadShinesJson(std::string_view text, const std::string& source, std::vector<Shine>& shines,
                    std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, ShinesFormatVersion, root, issues))
    {
        return;
    }
    const Reader reader(source, issues);
    const auto list = root.find(ShinesKey);
    if (list == root.end() || !list->is_array())
    {
        reader.Fatal(ShinesKey, "missing or not a list of shines");
        return;
    }
    std::set<std::string, std::less<>> names;
    for (size_t i = 0; i < list->size(); ++i)
    {
        const std::string field = std::string(ShinesKey) + "[" + std::to_string(i) + "]";
        Shine shine;
        if (!reader.ReadShine((*list)[i], field, shine))
        {
            continue;
        }
        if (!names.insert(shine.name).second)
        {
            reader.Fatal(field + "." + NameKey, "\"" + shine.name + "\" is in the list more than once");
            continue;
        }
        shines.push_back(std::move(shine));
    }
    reader.WarnAboutUnknownKeys(root, "", {Items::Json::Keys::FormatVersion, ShinesKey});
}

std::string WriteShinesJson(std::span<const Shine> shines)
{
    OrderedJson list = OrderedJson::array();
    for (const Shine& shine : shines)
    {
        OrderedJson json = OrderedJson::object();
        json[NameKey] = shine.name;
        if (shine.light)
        {
            json[LightKey] = Items::ModelJson::WriteNumber(*shine.light);
        }
        OrderedJson passes = OrderedJson::array();
        for (const ShinePass& pass : shine.passes)
        {
            passes.push_back(WritePass(pass));
        }
        json[PassesKey] = std::move(passes);
        list.push_back(std::move(json));
    }
    OrderedJson root;
    root[Items::Json::Keys::FormatVersion] = ShinesFormatVersion;
    root[ShinesKey] = std::move(list);
    const std::string text = root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    return Items::Json::PutListEntriesOnOneLine(text, PassesKey) + "\n";
}

ShineList& ShineList::GetInstance()
{
    static ShineList instance;
    return instance;
}

void ShineList::Build(std::span<const Shine> shines)
{
    m_shines.assign(shines.begin(), shines.end());
    ++m_version;
}

const Shine* ShineList::Find(std::string_view name) const
{
    const auto found = std::ranges::find(m_shines, name, &Shine::name);
    return found != m_shines.end() ? &*found : nullptr;
}
} // namespace Data::Effects
