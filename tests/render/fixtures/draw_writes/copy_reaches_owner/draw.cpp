void RenderThing(const OBJECT* o)
{
    OBJECT copy = *o;
    copy.Owner->Live = false;
}
