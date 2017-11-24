/*SHUFFLE*/
    if (gId >= get_global_size(0)*get_global_size(1))
    {
        return;
    }
    gId = shuffledIds[gId];
    globalId.x = (int)(gId % get_global_size(0));
    globalId.y = (int)(gId / get_global_size(0));
    if (globalId.x > get_global_size(0) || globalId.y > get_global_size(1))
    {
        return;
    }
