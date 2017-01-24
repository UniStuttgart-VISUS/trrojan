    /*OFFSET*/ 
	int gIdX = get_group_id(0);
	int gIdY = get_group_id(1);
	
	// invoke one tile only with 8x8 threads
	idX += offset.x;
	idY += offset.y;
	
	// reject based on global Id
	// if (idX < offsetX || idX >= offsetX + 8 || idY < offsetY || idY >= offsetY + 8)
    // {
        // return;
    // }
	// reject based on group Id
	// if (gIdX < offsetX / 8 || gIdX >= offsetX / 8 + 8 || gIdY < offsetY / 8 || gIdY >= offsetY / 8 + 8)
    // {
        // return;
    // }
	// reject based on global Id and group Id
	// if (idX / get_local_size(0) != gIdX || idY / get_local_size(1) != gIdY )
	// {
		// return;
	// }

