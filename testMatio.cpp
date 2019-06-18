#include "matio.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	mat_t *mat = Mat_CreateVer("test.mat", "MATLAB 5.0 MAT file. Created by The Captury.", MAT_FT_MAT5);

	size_t dims[2] = {3,1};
	matvar_t *keypoints = Mat_VarCreate("keypoints", MAT_C_CELL, MAT_T_CELL, 1, dims, NULL, 0);

	dims[0] = 32;
	dims[1] = 4;
	double data[32*4*3];
	for (int i = 0; i < 32*4*3; ++i)
		data[i] = i;

	for (int i = 0; i < 3; ++i) {
		matvar_t  *positions = Mat_VarCreate(NULL, MAT_C_DOUBLE, MAT_T_DOUBLE, 2, dims, data+32*4*i, 0);
		matvar_t  *thingy = Mat_VarSetCell(keypoints, i, positions);
	}

	Mat_VarWrite(mat, keypoints, MAT_COMPRESSION_NONE);

	Mat_Close(mat);
}
