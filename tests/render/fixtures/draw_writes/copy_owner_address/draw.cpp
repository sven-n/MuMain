void RenderThing(const OBJECT* source)
{
    OBJECT copy = *source;
    OBJECT* owner = &copy.Owner[0];
    owner->Alpha = 0.f;
}
