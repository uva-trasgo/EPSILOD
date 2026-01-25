if ( old.data[0] != FLAG_KEEP_VELOCITY ) {
	// Load cell
	for( int i = 0; i < Q; i++) 
		new.data[i] = neigh(ext_params.offsets[i].x, ext_params.offsets[i].y, ext_params.offsets[i].z ).data[i];

	if ( old.data[0] == FLAG_OBSTACLE ) {
		// Collision reorder
		cell_t cell_aux;
		for ( int i = 1; i<Q; i++ ) cell_aux.data[i] = new.data[ext_params.opposite[i]];
		new = cell_aux;
	}
	else {
		// Compute 
		float p  = 0;
		vec3f vp = {0};
		for (int i = 0; i < Q; i++) {
			p += new.data[i];
			VEC3_ADD_SCALED2( vp, ext_params.cellwidth, ext_params.offsets[i], new.data[i] );
		}
		/*	
			vec3f scaled;
			VEC3_SCALE(scaled, ext_params.offsets[i], ext_params.cellwidth)
			VEC3_SCALE(scaled, scaled, new.data[i])
			VEC3_ADD(vp, scaled)
		*/
		vec3f v;
		if (p == 0) v = vp;
		else VEC3_SCALE(v, vp, (1 / p))

		for (size_t i = 0; i < Q; i++) {
			new.data[i] += ext_params.deltaT / ext_params.tau * (feq(i,p,v) - new.data[i]);
		}		
	}
}
// Implicito
// hit( tileMat, i, j, k ) = new;