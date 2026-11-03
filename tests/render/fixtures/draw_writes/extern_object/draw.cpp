void RenderThing()
{
    OBJECT live;
    {
        extern OBJECT live;
        OBJECT* pointer = &live;
        pointer->Alpha = 0.f;
    }
}
