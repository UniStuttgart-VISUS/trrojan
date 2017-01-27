/*SHUFFLE*/
    if (gId >= get_global_size(0)*get_global_size(1))
    {
        return;
    }
    gId = shuffledIds[gId];
    idX = (int)(gId % get_global_size(0));
    idY = (int)(gId / get_global_size(0));
    if (idX > get_global_size(0) || idY > get_global_size(1))
    {
        return;
    }
