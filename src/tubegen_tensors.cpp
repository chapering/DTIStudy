#include <sys/time.h>

#include <gg/args.h>
#include <streamtube/streamtube.h>

#include <iostream>

using namespace std;


//  MISSING_ROUND is defined by the Makefile for gcc 2.95.
//
#ifdef MISSING_ROUND
int round(double x)
{   return  (int)floor(x + 0.5);
}
#endif

double radius = 1.0;
int csize = 20;
double clinear = 0.30;
double cdist = 2.0;
double sspace[3]={10,10,5};
double al = 0.1;
double cr = 0.3;
char  dt[256]="/u/sz/diffusion/data/susumu/mri/Diff";
char  chi0[256]="/u/sz/diffusion/data/susumu/mri/Diff";
char  chi1[256]="/u/sz/diffusion/data/susumu/mri/Diff";
char  chi2[256]="/u/sz/diffusion/data/susumu/mri/Diff";
char  t2[256]="/u/sz/diffusion/data/susumu/mri/T2";
char  dt1[256]="/u/sz/diffusion/data/susumu/mri/Diff";
char  dt2[256]="/u/sz/diffusion/data/susumu/mri/Diff";
char  pv[256]="/u/sz/diffusion/data/susumu/mri/Diff";
double pv1thresh = 0.5;
char  pvwm[256]="/u/sz/diffusion/data/susumu/mri/Diff";
char outfname[256] = "/map/gfx0/common0/diffusion/Interface/bin/tubegen/1.sm";
int savesm = 1;
double roipoint[3]={0,0,0};
double roilength[3]={0,0,0};
double cutoffroi[6] = {0,0,0,0,0,0};
int useroi = 0;
int useseedsfile = 0;
int usecutoffroi = 0;
int odemethod = 0;
int flatdir = -1;
double dthresh = 0.5;
double step = 1;
double t2thresh = -1;
double chi0thresh = 2.0;
double chi1thresh = 2.0;
double chi2thresh = 2.0;
int Colseed = 0;
char Colmask[256]="nomask";
double ColmaskVx[3];
double ColmaskVy[3];
double ColmaskCtr[3];
double ColDTICtr[3];
double Colmaskvsize[3] = {0.859376,0.859376,3.0000};
double ColDTIvsize[3];
int ColS = 4;
int SS = 0;
int memtest = 0;
char seedsfile[256]="NoSeedsFile";
char outseedsfile[256]="NoSeedsFile";
int gmcon = 0;
int crossmidbrain = 0;
int midbrain[3] = {0,45,45};//midbrain definiation, 0 is x direction, 1 is y, 2 is z; last two ints are slice numbers, starting from 0.
char segfname[256] = "NoSegFile";
int FASAMPLING = 0;
char fafname[256] = "./d2mem/FA";
int numptr= 6;
int meshnovsize = 0;
double randlen = 1.0;
double randseed = 0;
int MC = 0;
int longshort = 0;
int outFA = 0;

/* ---------------------- added by hcai 05/14/2011 --------------------- */
/* file to hold the tensor info */
char g_fnout[256] = {'\0'};
/* if output the tensor matrix - 6 distinct elements */
int g_bTensorElements = 0;
/* if output FA value for each voxel */
int g_bFAvalue = 1;
/* if contain eigen values of each voxel in the output */
int g_bEigenvalues = 1;
/* if contain eigen vector of each voxel in the output */
int g_bEigenvectors = 1;
/* ----------------------------------------------------------------------*/

void writeOutROI(tube_collection_generator* tg, double* Point, double* Length, char* filename);

args_spec args[] =
{
	{A_HELPMSG,(char *)"Generate a set of streamtubes."},
	{(char *)"out",(char *)"Output .sm file", {A_STRING, A_PTR &outfname}, A_REQUIRED},
	{A_HELPMSG, (char *)"    ======== Standard Options ========"},
	{A_HELPMSG, (char *)"        ---- Input ----"},
	{(char *)"dt",(char *)"Diffusion tensor image path", {A_STRING, A_PTR &dt}},
	{(char *)"t2",(char *)"T2-weighted image path", {A_STRING, A_PTR &t2}},
	{(char *)"FA",(char *)"Fractional anisotropy image path", {{A_STRING, A_PTR &fafname}}},
	{A_HELPMSG, (char *)"        ---- Output ----"},
	{(char *)"cr",(char *)"Compression ratio; 0 means no compression", {{A_DOUBLE, A_PTR &cr}}},
	{(char *)"OUTSF",(char *)"Output seed-points file path?", {{A_STRING, A_PTR &outseedsfile}}},
	{(char *)"SAVESM", (char *)"Save .sm file?", {{A_INT, A_PTR &savesm}}},
	{A_HELPMSG, (char *)"        ---- Seeding ----"},
	{(char *)"seed",(char *)"Block size in which to place one seed each.  Measured in voxels.",
		{{A_DOUBLE, A_PTR &(sspace[0])},
			{A_DOUBLE, A_PTR &(sspace[1])},
			{A_DOUBLE, A_PTR &(sspace[2])}}
	},
	{(char *)"randlen",(char *)"Random jittering diameter, coupled with seed",{{A_DOUBLE, A_PTR &randlen}}},
	{(char *)"FASAMPLING",(char *)"Sample according to FA?", {{A_INT, A_PTR &FASAMPLING}}},
	{(char *)"USESF",(char *)"Use seed point file?", {{A_INT, A_PTR &useseedsfile}}},
	{(char *)"SF",(char *)"Seed point file path", {{A_STRING, A_PTR &seedsfile}}},
	{A_HELPMSG, (char *)"        ---- Integration ----"},
	{(char *)"ODE",(char *)"ODE method. 0=rk2, 1=rk4",{{A_INT, A_PTR &odemethod}}},
	{(char *)"stepsize",(char *)"Stepsize in mm", {{A_DOUBLE, A_PTR &step}}},
	{(char *)"Longshort",(char *)"For integration, start from (0) short one, (1) long one, (2) longest of the two, or (3) mean of the two",{{A_INT, A_PTR &longshort}}},
	{(char *)"t2thresh",(char *)"Low threshold for T2 to terminate integration", {{A_DOUBLE, A_PTR &t2thresh}}},
	{(char *)"al",(char *)"Low threshold for linear anisotropy to terminate integration",{{A_DOUBLE, A_PTR &al}}},
	{A_HELPMSG, (char *)"        ---- Culling ----"},
	{(char *)"cs",(char *)"Min tube length in mm",{{A_INT, A_PTR &csize}}},
	{(char *)"cl",(char *)"Min average linear anisotropy",{{A_DOUBLE, A_PTR &clinear}}},
	{(char *)"cd",(char *)"Min distance between tubes in mm",{{A_DOUBLE, A_PTR &cdist}}},
	{(char *)"dth",(char *)"Low threshold on distances in mm",{{A_DOUBLE, A_PTR &dthresh}}},
	{A_HELPMSG, (char *)"        ---- Streamtube Rendering ----"},
	{(char *)"radius",(char *)"Streamtube radius in mm",{{A_DOUBLE, A_PTR &radius}}},
	{(char *)"NUMPTR", (char *)"Number of points on a ring", {{A_INT, A_PTR &numptr}}},
	{(char *)"MESHNOVSIZE", (char *)"Mesh coords not affected by vsize?", {{A_INT, A_PTR &meshnovsize}}},
	{(char *)"OUTFA", (char *)"Output FA as red color?", {A_INT, A_PTR &outFA}},
	{A_HELPMSG, (char *)"    ======== ROI Options ========"},
	{(char *)"USROI",(char *)"Use region of interest for seed point generation?",{{A_INT, A_PTR &useroi}}},
	{(char *)"ROI",(char *)"Region of interest for seed point generation",
		{{A_DOUBLE, A_PTR &(roipoint[0]), (char *)"<center_x"},
			{A_DOUBLE, A_PTR &(roipoint[1]), (char *)"center_y"},
			{A_DOUBLE, A_PTR &(roipoint[2]), (char *)"center_z>"},
			{A_DOUBLE, A_PTR &(roilength[0]), (char *)"<edgelen_x"},
			{A_DOUBLE, A_PTR &(roilength[1]), (char *)"edgelen_y"},
			{A_DOUBLE, A_PTR &(roilength[2]), (char *)"edgelen_z>"}}
	},
	{(char *)"USECUTOFFROI", (char *)"Use cutoff ROI?", {{A_INT, A_PTR & usecutoffroi}}},
	{(char *)"ROICUTOFF",(char *)"Cut off ROI. Only streamtubes running through this ROI are kept.",
		{{A_DOUBLE, A_PTR &(cutoffroi[0]), (char *)"<center_x"},
			{A_DOUBLE, A_PTR &(cutoffroi[1]), (char *)"center_y"},
			{A_DOUBLE, A_PTR &(cutoffroi[2]), (char *)"center_z>"},
			{A_DOUBLE, A_PTR &(cutoffroi[3]), (char *)"<edgelen_x"},
			{A_DOUBLE, A_PTR &(cutoffroi[4]), (char *)"edgelen_y"},
			{A_DOUBLE, A_PTR &(cutoffroi[5]), (char *)"edgelen_z>"}}
	},
	{A_HELPMSG, (char *)"    ======== Multi-Compartment Options ========"},
	{(char *)"MC",(char *)"Use the multi-compartment model?",{{A_INT, A_PTR &MC}}},
	{(char *)"dt1",(char *)"1nd diffusion tensor image path in multi-compartemnt model",{A_STRING, A_PTR &dt1}},
	{(char *)"dt2",(char *)"2nd diffusion tensor image path in multi-compartment model",{A_STRING, A_PTR &dt2}},
	{A_HELPMSG, (char *)"    ======== Partial Volume Options ========"},
	{(char *)"pv",(char *)"Partial volume between iso and wm image path",{A_STRING, A_PTR &pv}},
	{(char *)"pv1thresh",(char *)"partial volume threshold for tracking", {{A_DOUBLE, A_PTR &pv1thresh}}},
	{(char *)"pvwm",(char *)"Partial volume between wm image path",{A_STRING, A_PTR &pvwm}},
	{A_HELPMSG, (char *)"    ======== Statistical Options ========"},
	{(char *)"chi0",(char *)"chisqr for isotropic model",{A_STRING, A_PTR &chi0}},
	{(char *)"chi1",(char *)"chisqr for single-compartemnt model",{A_STRING, A_PTR &chi1}},
	{(char *)"chi2",(char *)"chisqr multi-compartemnt model",{A_STRING, A_PTR &chi2}},
	{(char *)"chi0thresh",(char *)"Isotropy model Chisqr threshold", {{A_DOUBLE, A_PTR &chi0thresh}}},
	{(char *)"chi1thresh",(char *)"Single-compartment model Chisqr threshold", {{A_DOUBLE, A_PTR &chi1thresh}}},
	{(char *)"chi2thresh",(char *)"Multi-compartment model Chisqr threshold", {{A_DOUBLE, A_PTR &chi2thresh}}},
	{A_HELPMSG, (char *)"    ======== ColA Options ========"},
	{(char *)"ColSeed",(char *)"seed ColA data mask for seeds",{{A_INT, A_PTR &Colseed}}},
	{(char *)"Colmask",(char *)"Col mask image path",{A_STRING, A_PTR &Colmask}},
	{(char *)"ColmaskVx", (char *)"Vx of the mask Coord",
		{{A_DOUBLE, A_PTR &(ColmaskVx[0])},
			{A_DOUBLE, A_PTR &(ColmaskVx[1])},
			{A_DOUBLE, A_PTR &(ColmaskVx[2])}}
	},
	{(char *)"ColmaskVy", (char *)"Vx of the mask Coord",
		{{A_DOUBLE, A_PTR &(ColmaskVy[0])},
			{A_DOUBLE, A_PTR &(ColmaskVy[1])},
			{A_DOUBLE, A_PTR &(ColmaskVy[2])}}
	},
	{(char *)"ColmaskCtr", (char *)"Ctr of the mask Coord",
		{{A_DOUBLE, A_PTR &(ColmaskCtr[0])},
			{A_DOUBLE, A_PTR &(ColmaskCtr[1])},
			{A_DOUBLE, A_PTR &(ColmaskCtr[2])}}
	},
	{(char *)"ColDTICtr", (char *)"Ctr of the DTI Coord",
		{{A_DOUBLE, A_PTR &(ColDTICtr[0])},
			{A_DOUBLE, A_PTR &(ColDTICtr[1])},
			{A_DOUBLE, A_PTR &(ColDTICtr[2])}}
	},
	{(char *)"ColDTIvsize", (char *)"vsize of the DTI Coord",
		{{A_DOUBLE, A_PTR &(ColDTIvsize[0])},
			{A_DOUBLE, A_PTR &(ColDTIvsize[1])},
			{A_DOUBLE, A_PTR &(ColDTIvsize[2])}}
	},
	{(char *)"Colmaskvsize", (char *)"vsize of the mask Coord",
		{{A_DOUBLE, A_PTR &(Colmaskvsize[0])},
			{A_DOUBLE, A_PTR &(Colmaskvsize[1])},
			{A_DOUBLE, A_PTR &(Colmaskvsize[2])}}
	},
	{(char *)"ColS",(char *)"Number of random points used in a voxel in Colseeding",{{A_INT, A_PTR &ColS}}},
	{A_HELPMSG, (char *)"    ======== Miscellaneous Options ========"},
	{(char *)"SS",(char *)"Using the tubes generated for the seed points of the Second Start",{{A_INT, A_PTR &SS}}},
	{(char *)"flatdir",(char *)"trace on a plane perpendicular to flatdir. 0: 1 0 0; 1: 0 1 0; 2: 0 0 1",{{A_INT, A_PTR &flatdir}}},
	{(char *)"SEG",(char *)"Segmentation image file path", {{A_STRING, A_PTR &segfname}}},
	{(char *)"GMCON",(char *)"GM constraint extension step, 0 means no constraint", {{A_INT, A_PTR &gmcon}}},
	{(char *)"CROSSMIDBRAIN",(char *)"Col lesion test: keep only the tubes across midbrain?", {{A_INT, A_PTR &crossmidbrain}}},
	{(char *)"MIDBRAIN", (char *)"Definition of midbrain (in voxel units)",
		{{A_INT, A_PTR &(midbrain[0])},
			{A_INT, A_PTR &(midbrain[1])},
			{A_INT, A_PTR &(midbrain[2])}}
	},
	{(char *)"randseed",(char *)"srand? 0: no srand, -1: srand(time), other: srand(other) ",{{A_DOUBLE, A_PTR &randseed}}},

	/* ---------------------- added by hcai 05/14/2011 --------------------- */
	{(char *)"tensorinfo",(char *)"file to save Tensor data associated with each streamline point", {A_STRING, A_PTR &g_fnout}},
	{(char *)"te", (char *)"contain tensor elements", {A_INT, A_PTR &g_bTensorElements}},
	{(char *)"fa", (char *)"contain FA value", {A_INT, A_PTR &g_bFAvalue}},
	{(char *)"eval", (char *)"contain eigenvalues", {A_INT, A_PTR &g_bEigenvalues}},
	{(char *)"evec", (char *)"contain eigenvectors", {A_INT, A_PTR &g_bEigenvectors}},
	/* ----------------------------------------------------------------------*/

	{A_END}
};


int main(int argc, char **argv)
{
	int i1,i2,i3;
	// magic character to indicate "do not write the seed file"
	outseedsfile[5] = 15;

	args_scanx(args,argc,argv);

	tube_collection_generator * tg;

	if(MC == 0) {
		tg = new tube_collection_generator(dt, t2);
	} else if(MC == 1) {
		tg = new tube_collection_generator(chi0, chi1, chi2, dt, dt1, dt2, t2, pv, pvwm);
	}

	cout << "Spacing: " <<
		tg->getDtData()->getTensors()->getSpacing(0) << " " <<
		tg->getDtData()->getTensors()->getSpacing(1) << " " <<
		tg->getDtData()->getTensors()->getSpacing(2) << endl;

	// convert units for culling distance and distance threshold
	// BUGBUG -- what are the units on cdist and dthresh?
	cdist = cdist/tg->getDtData()->getTensors()->getSpacing(0);
	dthresh = dthresh/tg->getDtData()->getTensors()->getSpacing(0);

	cout << "CullingDist: " << cdist << " | MinCullingDist: " << dthresh << endl;

	int i,j,k;

	if (fabs(randseed-0)<0.0000000001)
	{
		;
	}
	else if(fabs(randseed+1)<0.000000001)
	{
		struct timeval t;
		gettimeofday(&t,0);
		srand48(t.tv_sec);
		cout<<"rand: "<<t.tv_sec<<endl;
	}
	else
	{
		srand48((long int)randseed);
	}

	/*
	   double min=10000, max=-10000;

	   for(i=0;i<tg->getDtData()->getTensors()->getSize(0);i++)
	   for(j=0;j<tg->getDtData()->getTensors()->getSize(1);j++)
	   for(k=0;k<tg->getDtData()->getTensors()->getSize(2);k++){

	   double loc[3];
	   loc[0] = i;loc[1]=j;loc[2]=k;
	   MagDiffTens tens = (MagDiffTens)(tg->getDtData()->getTensorInterp())->interp_m(tg->getDtData()->getTensors(), loc);
	   double number[6];
	   tens->getSymmetric(number);

	   for(int lp=0;lp<6;lp++){
	   if(number[lp]>max)
	   max = number[lp];
	   if(number[lp]<min)
	   min = number[lp];
	   }

	   }

	   cout<<"minmax"<<min<<" "<<max<<endl;
	   exit(0);
	   */

	cout << "Mean I0 = " << tg->getDtData()->getI0Mean() << endl;
	cout << "DTI X size = " << tg->getDtData()->getTensors()->getSize(0) << endl;

	vector<double> seeds;

	float randlen = 0.5;
	if(sspace[0]<1)
		randlen = sspace[0]*0.5;


	if((!useroi)&&(!Colseed)&&(!useseedsfile)) {
		/*
		   for(i=0;i<tg->getDtData()->getTensors()->getSize(0)/sspace[0];i++){
		   for(j=0;j<tg->getDtData()->getTensors()->getSize(1)/sspace[1];j++)
		   for(k=0;k<tg->getDtData()->getTensors()->getSize(2)/sspace[2];k++){

		   if(memtest){

		   seeds.push_back(105);
		   seeds.push_back(97);
		   seeds.push_back(83);

		   }
		   else{
		   seeds.push_back(i*sspace[0]+(drand48()-0.5)*randlen);
		   seeds.push_back(j*sspace[1]+(drand48()-0.5)*randlen);
		   seeds.push_back(k*sspace[2]+(drand48()-0.5)*randlen);
		   }
		   }
		   }
		   */
	} else if(useroi) {
		for(i=(int)ceil(roipoint[0]-roilength[0]/2.0);i<(int)floor(roipoint[0]+roilength[0]/2.0);i++) {
			for(j=(int)ceil(roipoint[1]-roilength[1]/2.0);j<(int)floor(roipoint[1]+roilength[1]/2.0);j++) {
				for(k=(int)ceil(roipoint[2]-roilength[2]/2.0);k<(int)floor(roipoint[2]+roilength[2]/2.0);k++) {
					cout << "<ijk> = <" << i << " " << j << " " << k << ">" << endl;
					for(i1=0;i1<1.0/sspace[0];i1++){
						for(i2 = 0;i2<1.0/sspace[1];i2++){
							for(i3 = 0;i3<1.0/sspace[2];i3++){
								double dtmp;
								double xmin = roipoint[0]-roilength[0]/2.0,
									   xmax = roipoint[0]+roilength[0]/2.0,
									   ymin = roipoint[1]-roilength[1]/2.0,
									   ymax = roipoint[1]+roilength[1]/2.0,
									   zmin = roipoint[2]-roilength[2]/2.0,
									   zmax = roipoint[2]+roilength[2]/2.0;
								dtmp = i+i1*sspace[0] + (drand48()-0.5)*sspace[0]*0.5;
								while((dtmp<xmin)||(dtmp>xmax))
									dtmp = i+i1*sspace[0] + (drand48()-0.5)*sspace[0]*0.5;
								//    cout<<dtmp<<" "<<i+i1*sspace[0]<<" ";
								seeds.push_back(dtmp);
								dtmp = j+i2*sspace[1] + (drand48()-0.5)*sspace[1]*0.5;
								while((dtmp<ymin)||(dtmp>ymax))
									dtmp = j+i2*sspace[1] + (drand48()-0.5)*sspace[1]*0.5;
								//    cout<<dtmp<<" "<<j+i2*sspace[0]<<" ";
								seeds.push_back(dtmp);
								dtmp = k+i3*sspace[2] + (drand48()-0.5)*sspace[2]*0.5;
								while((dtmp<zmin)||(dtmp>zmax))
									dtmp = k+i3*sspace[2] + (drand48()-0.5)*sspace[2]*0.5;
								//    cout<<dtmp<<" "<<k+i3*sspace[2]<<endl;;
								seeds.push_back(dtmp);
							} } }
				} } }
		writeOutROI(tg, roipoint, roilength, (char *)"./models/ROI.sm");
	} else if(Colseed) {
		cout<<"before open mask"<<endl;

		/*
		   MRIimage* img = new MRIimage("/map/gfx0/common0/diffusion/Interface/datasets/Colorado_10_27_03_A/bin/mask");
		   cout<<"open mask"<<endl;
		   Vec vx(0.999846,0,-0.0175481);
		   Vec vy(-0.00225841,0.991701,-0.128544);
		   Vec vz(0.01740,0.12855,0.99155);
		   Pt ctr(-112.753,-116.432,-60.2749);
		   Pt ctr2(-140,-140,-18.4);
		   */


		//old colseed, used for Colorado_10_27_03_AB, new ones Colorado_3... changed to registered mask
		/*
		   MRIimage* img = new MRIimage(Colmask);
		   cout<<"open mask"<<endl;
		   Vec vx(ColmaskVx[0],ColmaskVx[1],ColmaskVx[2]);
		   Vec vy(ColmaskVy[0],ColmaskVy[1],ColmaskVy[2]);
		   Vec vz =  (cross(vx,vy)).normalize();
		   if(vz.z()<0)
		   vz = vz*(-1.0);
		   Pt ctr(ColmaskCtr[0], ColmaskCtr[1], ColmaskCtr[2]);
		   Pt ctr2(ColDTICtr[0], ColDTICtr[1], ColDTICtr[2]);

		   for(k=0;k<img->zres;k++){
		   for(j=0;j<img->yres;j++){
		   for(i=0;i<img->xres;i++){
		   if(fabs(img->lookup_nocheck_s(i,j,k,0)-1)<0.00001){
		   for(int nums=0;nums<ColS;nums++){
		   double r[3] = {drand48()-0.5, drand48()-0.5,drand48()-0.5};
		   Vec preal = ctr-ctr2+(i+r[0])*Colmaskvsize[0]*vx+(j+r[1])*Colmaskvsize[1]*vy+(k+r[2])*Colmaskvsize[2]*vz;
		   seeds.push_back(preal.x()/ColDTIvsize[0]);
		   seeds.push_back(preal.y()/ColDTIvsize[1]);
		   seeds.push_back(preal.z()/ColDTIvsize[2]);
		   }
		   }
		   }
		   }
		   }
		   delete img;
		   */

		MRIimage* img = new MRIimage(Colmask);

		cout<<"file name : "<<Colmask<<endl;
		cout<<"seeds size: "<<seeds.size()<<endl;

		for(k=0;k<img->zres;k++){
			for(j=0;j<img->yres;j++){
				for(i=0;i<img->xres;i++){
					if(fabs(img->lookup_nocheck_s(i,j,k,0)-1)<0.00001){
						cout<<"ijk "<<i<<" "<<j<<" "<<k<<endl;
						for(int nums=0;nums<ColS;nums++){
							double r[3] = {drand48()-0.5, drand48()-0.5,drand48()-0.5};
							seeds.push_back(i+r[0]*0.5);
							seeds.push_back(j+r[1]*0.5);
							seeds.push_back(k+r[2]*0.5);
						}
					}
				} } }
		delete img;
	} else if(useseedsfile) {
		//fstream fseeds(seedsfile);
		fstream fseeds(seedsfile, ios::in);
		int nseeds;
		double x,y,z;
		fseeds>>nseeds;

		//this ensures that precison sets 
		//the number of digits after the decimal 
		//point, -cad, Wed Mar 29 11:07:37 EST 2006
		//fseeds.setf(ios_base::fixed, ios_base::floatfield);
		fseeds.setf(ios::fixed, ios::floatfield);
		fseeds.precision(5);

		//std::cout<<"reading seed points..."<<std::endl;
		for(i=0;i<nseeds;i++){
			fseeds>>x>>y>>z;
			seeds.push_back(x);
			seeds.push_back(y);
			seeds.push_back(z);
		}
	}

	cout << "# seeds: " << seeds.size()/3.0 << endl;

	// set the tube generator parameters from command-line arguments
	tg->outfname = outfname;
	tg->setSeeds(seeds);
	tg->setCullingSize(csize);
	tg->setCullingDistance(cdist);
	tg->setCullingLinearanisotropy(clinear);
	tg->setDistThreshold(dthresh);
	tg->_randlen = randlen;
	if(cr>0){
		tg->setCompressRatio(cr);
		tg->setCompresstube(1);
	} else {
		tg->setCompresstube(0);
	}
	tg->setStepSize(step);
	tg->setTAnisLow(al);
	tg->setODEMETHOD(odemethod);
	tg->setFlatDir(flatdir);
	tg->setT2Thresh(t2thresh);
	tg->setPV1Thresh(pv1thresh);
	tg->setLongshort(longshort);
	tg->_chi0_thresh = chi0thresh;
	tg->_chi1_thresh = chi1thresh;
	tg->_chi2_thresh = chi2thresh;
	tg->_outFA = outFA;
	if(usecutoffroi == 1){
		tg->setCutOffROI(cutoffroi);
	}
	if(gmcon){
		tg->_seg = new MRIimage(segfname);
		cout << "segfile name " << segfname << " " << tg->_seg->yres << endl;
		tg->_GMCON = gmcon;
	}
	if(crossmidbrain){
		tg->_CROSSMIDBRAIN = crossmidbrain;
		tg->_MIDBRAIN[0] = midbrain[0];
		tg->_MIDBRAIN[1] = midbrain[1];
		tg->_MIDBRAIN[2] = midbrain[2];
	}
	tg->_FASAMPLING = FASAMPLING;
	if(FASAMPLING== 1 || outFA == 1 ){
		tg->_FA = new MRIimage(fafname);
	}
	tg->_Outseedsfile = outseedsfile;

	cout << "begining to generate tubes" << endl;
	if((!useroi)&&(!Colseed)&&(!useseedsfile)) {
		tg->generateSpacing(sspace, MC);
	} else {
		tg->generate(MC);
	}
	//  tg->generate2();
	cout << "finished generating tubes" << endl;
	cout << "culling size = " << tg->getCullingSize() << endl;
	cout << "culling LA = " << tg->getCullingLinearanisotropy() << endl;
	cout << "culling distance " << tg->getCullingDistance() << endl;
	cout << "tube list size" << tg->getTubeList().size()<<endl;

	/* ---------------------- added by hcai 05/14/2011 --------------------- */
	/* dump tensor data associated with each streamline point */
	/* ---------------      OUTPUT DATA FORMAT  ------------------------- 
	 * line structure where each line contains
	 *
	 * voxel_coordinate tensor_matrix FA eigenvalues eigenvectors 
	 *
	 * Vx Vy Vz t(0,0) t(0,1) t(0,2) t(1,1) t(1,2) t(2,2) FA Lamda1 Lamda2 Lamda3 \
	 * Vec1x Vec1y Vec1z Vec2x Vec2y Vec2z Vec3x Vec3y Vec3z
	 *
	 * added by hcai, 05/14/2011
	 */
	if ( strlen( g_fnout ) >= 1 ) {
		ofstream ofs ( g_fnout );

		vector<streamtube*> tl = tg->getTubeList();

		/* ------- if wanna write in binary format, release all the annonated as
		 * follows
		int space = 0;
		for(int i=0;i<tl.size();i++){
			space+=tl[i]->getSize();
		}
		int szElement = 3;
		if ( g_bFAvalue ) {
			szElement += 1;
		}
		if ( g_bEigenvectors ) {
			szElement += 9;
		}
		if ( g_bEigenvalues ) {
			szElement += 3;
		}
		if ( g_bTensorElements ) {
			szElement += 6;
		}

		float *data = (float*)malloc(sizeof(float)*(space*szElement));
		*/

		//int index = 0;
		ofs << tl.size() << endl;
		for(int i=0;i<tl.size();i++){
			//data[index++] = sizeof(float)*tl[i]->getSize()*szElement;
			ofs << tl[i]->getSize() << endl;
			cerr << "In processing with streamline No. " << (i+1) << endl;
			for(int j=0;j<tl[i]->getSize();j++){
				double loc[3];
				double dataloc[3];
				for(int k=0;k<3;k++){
					dataloc[k] = tl[i]->getTrajectoryPoints()[j*3+k];
					loc[k] = tl[i]->getTrajectoryPoints()[j*3+k]*tg->getDtData()->getTensors()->getSpacing(k);
					//data[index++] = (float)loc[k];
					ofs << (float)loc[k] << " ";
				}
				MagDiffTens tens = (MagDiffTens)(tg->getDtData()->getTensorInterp())->interp_m(tg->getDtData()->getTensors(), dataloc);
				if ( g_bFAvalue ) {
					ofs << tens->fracAnisotropy() << " ";
					//data[index++] = tens->fracAnisotropy();
				}

				if ( g_bEigenvalues ) {
					double e[3];
					tens->eigenvalues(e);
					for(int k=0;k<3;k++) {
						//data[index++] = e[k];
						ofs << e[k] << " ";
					}
				}

				if ( g_bEigenvectors ) {
					double e[3];
					for(int ei = 0;ei<3; ei++){
						tens->eigenvector(ei, e);
						for(int k=0;k<3;k++) {
							//data[index++] = e[k];
							ofs << e[k] << " ";
						}
					}
				}

				if ( g_bTensorElements ) {
					for ( int e = 0; e < 6; e++) {
						ofs << tens->getDiff(e) << " ";
						//data[index++] = tens->getDiff(e);
					}
				}

				ofs << endl;
			}
		}
		//ofs.write((unsigned char*)data, sizeof(float)*(space*15+tl.size()));
		ofs.close();
		// free (data);
		cerr << "Finished with tensor info saved into " << g_fnout << endl;
	}
	/* ----------------------------------------------------------------------*/

	jot_tube_mesh_generator * mg = new jot_tube_mesh_generator(tg->getTubeList());
	mg->setRadius(radius);
	mg->setNumptr(numptr);

	// generate the streamTUBES (curved cylindrical volumes around the
	// streamlines) and write them to a file
	if(savesm){
		mg->generate();
		mg->writeMesh(outfname,meshnovsize);
		string s = outfname;
		s.replace(s.find(".sm"),3,".size");
		ofstream sizef(s.c_str());
		sizef<<tg->getTubeList().size()<<endl;
		sizef.close();
	}

	string str2 = outfname;
	str2.replace(str2.find(".sm"),3,".data");
	mg->writeTrajCross((char*)str2.c_str());

	if(SS){

		tube_collection_generator * tgss = new tube_collection_generator(dt, t2);

		cout << "spacing " <<
			tgss->getDtData()->getTensors()->getSpacing(0) << " " <<
			tgss->getDtData()->getTensors()->getSpacing(1) << " " <<
			tgss->getDtData()->getTensors()->getSpacing(2) << endl;

		vector<double> sspert;
		vector<double> sseeds = mg->meshG->getTrajPoints();
		cout<<"SS seed number : "<<sseeds.size()/3.0<<endl;
		for(i=0;i<sseeds.size()/3.0;i++){
			for(j=0;j<10;j++){
				sspert.push_back(sseeds[i*3]/(tgss->getDtData()->getTensors()->getSpacing(0))+drand48()*2-1);
				sspert.push_back(sseeds[i*3+1]/(tgss->getDtData()->getTensors()->getSpacing(1))+drand48()*2-1);
				sspert.push_back(sseeds[i*3+2]/(tgss->getDtData()->getTensors()->getSpacing(2))+drand48()*2-1);
			}
		}


		tgss->setSeeds(sspert);

		tgss->setCullingSize(csize);
		tgss->setCullingDistance(cdist);
		tgss->setCullingLinearanisotropy(clinear);
		tgss->setDistThreshold(dthresh);
		if(cr>0){
			tgss->setCompressRatio(cr);
			tgss->setCompresstube(1);
		}
		else
			tgss->setCompresstube(0);

		tgss->setStepSize(step);
		tgss->setTAnisLow(al);
		tgss->setODEMETHOD(odemethod);
		tgss->setFlatDir(flatdir);
		tgss->setT2Thresh(t2thresh);

		cout<<"out1"<<endl;
		tgss->generate(MC);

		jot_tube_mesh_generator * mgss = new jot_tube_mesh_generator(tgss->getTubeList());
		mgss->setRadius(radius);
		mgss->generate();
		string sss = outfname;
		sss.replace(sss.find(".sm"),3,"ss.sm");
		mgss->writeMesh((char*)sss.c_str(),0);

		string str1 = outfname;
		str1.replace(str1.find(".sm"),3,"ss.data");
		mgss->writeTrajCross((char*)str1.c_str());


	}

#if cullthing
	cout<<"cusize"<<tg->_culllist.size()<<endl;

	for(int i=0;i<tg->_culllist.size();i++){
		cout<<tg->_culllist[i].size()<<endl;
		mg = new jot_tube_mesh_generator(tg->_culllist[i]);
		cout<<tg->_culllist[i][0]->getSize()<<endl;
		cout<<"hhh"<<endl;;
		mg->generate();
		cout<<"hh"<<endl;

		char fn[30] = "./models/list/aaaa.sm";
		fn[14] = i/1000 + '0';
		fn[15] = i/100-(i/1000)*10+'0';
		fn[16] = i/10 -(i/100*10)+'0';
		fn[17] = i-(i/10)*10+'0';
		cout<<fn<<endl;
		mg->writeMesh(fn,0);
	}
#endif

}

void writeOutROI(tube_collection_generator* tg, double* Point, double* Length, char* filename){

	ofstream ofs(filename);
	double P[8][3];
	int index = 0;
	int i,j,k;
	for(k=0;k<2;k++)
		for(j=0;j<2;j++)
			for(i=0;i<2;i++){
				P[index][0] = (Point[0]-Length[0]/2.0+Length[0]*i)*tg->getDtData()->getTensors()->getSpacing(0);
				P[index][1] = (Point[1]-Length[1]/2.0+Length[1]*j)*tg->getDtData()->getTensors()->getSpacing(1);
				P[index][2] = (Point[2]-Length[2]/2.0+Length[2]*k)*tg->getDtData()->getTensors()->getSpacing(2);
				index++;
			}

	ofs<<8<<endl;
	for(i=0;i<8;i++)
		ofs<<P[i][0]<<" "<<P[i][1]<<" "<<P[i][2]<<endl;

	ofs<<12<<endl;

	ofs<<0<<" "<<2<<" "<<1<<endl;
	ofs<<1<<" "<<3<<" "<<2<<endl;
	ofs<<4<<" "<<6<<" "<<5<<endl;
	ofs<<5<<" "<<7<<" "<<6<<endl;
	ofs<<0<<" "<<6<<" "<<4<<endl;
	ofs<<0<<" "<<6<<" "<<2<<endl;
	ofs<<0<<" "<<5<<" "<<4<<endl;
	ofs<<0<<" "<<5<<" "<<1<<endl;
	ofs<<1<<" "<<3<<" "<<5<<endl;
	ofs<<3<<" "<<7<<" "<<5<<endl;
	ofs<<2<<" "<<7<<" "<<3<<endl;
	ofs<<2<<" "<<7<<" "<<6<<endl;

	ofs.close();

}
