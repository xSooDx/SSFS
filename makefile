ssfuse: ssfuse.c ssaux.h 
	gcc ssfuse.c  `pkg-config fuse --libs` -D_FILE_OFFSET_BITS=64 -o ssfs

