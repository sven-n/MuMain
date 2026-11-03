void RenderThing(const OBJECT* o, const ITEM& item)
{
    OBJECT copy = *o;
    OBJECT* turned = &copy;
    turned->Angle[2] += 90.f;
    copy.BoneTransform = o->BoneTransform;
    const CHARACTER* c = &CharactersClient[0];
    const auto* effect = &Effects[0];
    // OBJECT* stale = &Items[0];
    /* CHARACTER* hero = Hero; */
    const wchar_t* text = L"OBJECT* o = &Effects[0];";
    const int weapon = o->Owner->Weapon;
}

void MoveThing(OBJECT* o)
{
    CHARACTER* c = &CharactersClient[0];
}

void RenderCloth(const CHARACTER* c)
{
    if (auto* cloth = (CPhysicsCloth*)c->Object.m_pCloth)
    {
    }
    auto vertex = [&](float u) { return u; };
}

void RenderSplit(const
                 OBJECT* o)
{
    // OBJECT* oldObject;
    const char* text = "}";
    CHARACTER copy = *Hero;
    OBJECT* body = &copy.Object;
    body->Alpha = 0.5f;
}
