void RenderThing(const OBJECT* source)
{
    OBJECT copy = *source;
    auto& alias = copy;
    alias.Owner->Live = false;
}
