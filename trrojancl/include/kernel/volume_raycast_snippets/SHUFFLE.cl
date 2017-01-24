/*SHUFFLE*/
    if (gId >= IMG_RES*IMG_RES)
    {
        return;
    }
    gId = shuffledIds[gId];
    idX = (int)(gId % get_global_size(0));
    idY = (int)(gId / get_global_size(0));
	if (idX > IMG_RES || idY > IMG_RES)
	{
		return;
	}
