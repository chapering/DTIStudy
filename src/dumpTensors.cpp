#include <sys/time.h>
#include <gg/args.h>
#include <streamtube/streamtube.h>
#include <iostream>

using namespace std;

// global data structures for command line handling
/* where to load the tensor data */
char g_dtpath[256] = "./tensors";
/* where to dump the tensor info */
char g_fnout[256] = "tensorinfo";

/* if output FA value for each voxel */
int g_bFAvalue = 1;
/* if contain eigen values of each voxel in the output */
int g_bEigenvalues = 1;
/* if contain eigen vector of each voxel in the output */
int g_bEigenvectors = 1;

/* output file stream */
ostream* g_ofs = &std::cout;

args_spec args[] =
{
	{A_HELPMSG,(char *)"Dump tensor data in plain text from the given tensor field."},
	{A_HELPMSG, (char *)"----------------------------------------------------------"}, 

	{(char *)"dtpath",(char *)"Diffusion tensor image path", {A_STRING, A_PTR &g_dtpath}},

	{(char *)"fnout",(char *)"Output tensor info file", {A_STRING, A_PTR &g_fnout}},

	{(char *)"fa", (char *)"contain FA value", {A_INT, A_PTR &g_bFAvalue}},
	{(char *)"eval", (char *)"contain eigenvalues", {A_INT, A_PTR &g_bEigenvalues}},
	{(char *)"evec", (char *)"contain eigenvectors", {A_INT, A_PTR &g_bEigenvectors}},

	{A_END}
};

int main(int argc, char **argv)
{
	if ( argc < 2 ) {
		cerr << "too few arguments, type " << argv[0] << " -- for usage." << endl;
		return -1;
	}
	args_scanx(args,argc,argv);

	if ( strlen( g_fnout ) >= 1 ) {
		g_ofs = new ofstream(g_fnout);
	}

	tube_collection_generator* tg = new tube_collection_generator(g_dtpath);

	int xDim, yDim, zDim, i, j, k;
	xDim = tg->getDtData()->getTensors()->getSize(0);
	yDim = tg->getDtData()->getTensors()->getSize(1);
	zDim = tg->getDtData()->getTensors()->getSize(2);

	cerr << "DTI scale: " << xDim << " x " << yDim << " x " << zDim << endl;

	// traverse through the tensor field volume
	MagVolDiffTens pVolDiffTens = tg->getDtData()->getTensors();
	double evals[3], evec[3];
	for (i = 0; i < xDim; i ++) {
		for (j = 0; j < yDim; j ++) {
			for (k = 0; k < zDim; k ++) {
				cerr << "in processing " << i << " " << j << " " << k << endl;
				(*g_ofs) << i << " " << j << " " << k;
				const MagDiffTens pDiffTens = pVolDiffTens->getDiffTens(i,j,k);
				for ( int e = 0; e < 6; e++) {
					(*g_ofs) << " " << pDiffTens->getDiff(e);
				}

				if ( g_bFAvalue ) {
					(*g_ofs) << " " << pDiffTens->fracAnisotropy();
				}

				if ( g_bEigenvalues ) {
					pDiffTens->eigenvalues(evals);
					for ( int c = 0; c < 3; c++ ) {
						(*g_ofs) << " " << evals[c];
					}
				}

				if ( g_bEigenvectors ) {
					for ( int d = 0; d < 3; d++ ) {
						pDiffTens->eigenvector(d, evec);
						for ( int c = 0; c < 3; c++ ) {
							(*g_ofs) << " " << evec[c];
						}
					}
				}

				// other tensor properties extended here if needed
				// ....
				(*g_ofs) << endl;
			}
		}
	}

	if ( strlen( g_fnout ) >= 1 ) {
		((ofstream*)g_ofs)->close();
		delete g_ofs;
	}

	delete tg;

	return 0;
}

