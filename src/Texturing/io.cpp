#include "io.h"

using namespace std ;

//----------------------
int write_img (char path[], int w, int h, uint8_t * px)
{
	try {

		std::ofstream out(path, std::ios::binary);
		TinyPngOut pngout(static_cast<uint32_t>(w), static_cast<uint32_t>(h), out);
		pngout.write(px, static_cast<size_t>(w * h));
		return EXIT_SUCCESS;

	} catch (const char *msg) {
		std::cerr << msg << std::endl;
		return EXIT_FAILURE;
	}
}
//---------
int csvread_A (const char path[], v2d & result, int d)
{
 FILE *in ; int n=0 ; double tmp ;
 in=fopen(path, "r") ; if (in==NULL) {printf("Cannot open input file %s\n", path) ; return 1 ; }
 int r ;

 r=fscanf(in,"%*[^\n]%*c") ; // Skipping header

 while (!feof(in))
 {
     r=fscanf(in, "%lg%*c", &tmp) ;
     if (feof(in)) break ;

     result.push_back(v1d (d*d,0)) ;
     result[n][0]=tmp ;
     for (int i=1 ; i<d*d ; i++)
         r=fscanf(in, "%lg%*c", &result[n][i]) ;

     //dispvector(result[n]) ;
     r=r ;
     n++ ;
 }

 fclose(in) ;
 return 0 ;
}
//-------
int csvread_XR (const char path[], v2d & result, v1d &R, int d)
{
 FILE *in ; int n=0 ; double tmp ;
 in=fopen(path, "r") ; if (in==NULL) {printf("Cannot open input file %s\n", path) ; return 1 ; }
 int r ;

 r=fscanf(in,"%*[^\n]%*c") ; // Skipping header

 while (!feof(in))
 {
     r=fscanf(in, "%lg%*c", &tmp) ;
     if (feof(in)) break ;

     result.push_back(v1d (d,0)) ;
     R.push_back(0) ;
     result[n][0]=tmp ;
     for (int i=1 ; i<d ; i++)
         r=fscanf(in, "%lg%*c", &result[n][i]) ;
     r=fscanf(in, "%lg%*c", &R[n]) ;
     //printf("%g %g %g %g\n", result[n][0], result[n][1], result[n][2], R[n]) ;
     r=fscanf(in, "%*s%*c") ;
     r=r ;
     n++ ;
 }

 fclose(in) ;
 return 0 ;
}
//--------------
int write_colormap_vtk(int d, vector<vector<float>> & colors)
{

int nvalues=40 ;
vector<double> p(3), ptmp ;
vector <uint8_t> a ;
a.resize(3,0) ;

if (d!=4) printf("ERROR: cannot export the colormap for d!=4 currently\n") ;

FILE *vtkout ;
vtkout=fopen("Colormap.vtk", "w") ;
fprintf(vtkout,"# vtk DataFile Version 2.0\nTexture for ND DEM\nASCII\nDATASET STRUCTURED_POINTS\nDIMENSIONS %d %d %d\nORIGIN %g %g %g\nSPACING %g %g %g\n\nPOINT_DATA %d\nCOLOR_SCALARS Color 3\n", nvalues, nvalues, nvalues, 0 +M_PI/2/nvalues, 0 + M_PI/2/nvalues, 0+M_PI/nvalues, M_PI/nvalues, M_PI/nvalues, 2*M_PI/nvalues, nvalues*nvalues*nvalues) ;

p[2] = 0 + M_PI/nvalues ;
printf("%g ", p[2]) ;
for (int i=0 ; i<nvalues ; i++)
{
  p[1] = 0 +M_PI/2/nvalues ;
  for (int j=0 ; j<nvalues ; j++)
  {
    p[0] = 0+M_PI/2/nvalues ;
    for (int k=0 ; k<nvalues ; k++)
    {
      ptmp=p ;
      phi2color(a.begin(), ptmp, d, colors) ;
      //printf("%g %g %g\n", p[0], p[1], p[2]) ;
      fprintf(vtkout, "%g %g %g\n", a[0]/256., a[1]/256., a[2]/256.) ;
      p[0] += M_PI/nvalues ;
    }
    p[1] += M_PI/nvalues ;
  }
  p[2] += 2*M_PI/nvalues ;
}

fclose(vtkout) ;
return 0 ;
}
//-------------------------------------------
int write_NrrdIO (string path, int d, vector<vector<float>> & colors)
{
#ifdef NRRDIO
    int npoints=32 ;
    vector<double> p(d-1,0) ;

    Nrrd *nval;
    auto nio = nrrdIoStateNew();
    nrrdIoStateEncodingSet(nio, nrrdEncodingAscii) ; //Change to nrrdEncodingRaw for binary encoding
    nval = nrrdNew();

    // Header infos
    vector <size_t> dimensions (d+1, npoints) ;
    dimensions[0] = 3 ;
		dimensions[1] = 1 ;
		dimensions[2]= npoints*2 ;

    vector <int> nrrdkind (d, nrrdKindSpace) ;
    nrrdkind[0]=nrrdKindVector ;
    nrrdAxisInfoSet_nva(nval, nrrdAxisInfoKind, nrrdkind.data() );

    vector <double> nrrdmin(d,0+M_PI/npoints/2), nrrdmax(d,M_PI-M_PI/npoints/2), nrrdspacing(d,M_PI/npoints) ;
    nrrdmin[0]=nrrdmax[0]=nrrdspacing[0]=AIR_NAN ;
    nrrdmin[d-1]=0+M_PI/npoints ; nrrdmax[d-1]=2*M_PI+M_PI/npoints ;

    char ** labels;
    labels=(char **) malloc(sizeof(char *) * (d+3)) ;
    labels[0]=(char *) malloc(sizeof(char) * (4)) ;
    sprintf(labels[0], "rgb") ;
    for (int dd=1 ; dd<d ; dd++)
    {
        labels[dd]=(char *) malloc(sizeof(char) * (3+d/10+1+1)) ;
        sprintf(labels[dd], "phi%d", dd) ;
    }

    nrrdAxisInfoSet_nva(nval, nrrdAxisInfoLabel, labels);
    nrrdAxisInfoSet_nva(nval, nrrdAxisInfoMin, nrrdmin.data());
    nrrdAxisInfoSet_nva(nval, nrrdAxisInfoMax, nrrdmax.data());
    nrrdAxisInfoSet_nva(nval, nrrdAxisInfoSpacing, nrrdspacing.data());

    int allpoint=pow(npoints,d-1)*2 ;
    unsigned char * outdata, * pout ;
    outdata=(unsigned char*) malloc(sizeof(unsigned char)*allpoint*3) ;
  //---------------------------------------------------------------------
    std::function <void(int,vector<double>)> lbdrecurse ;
    pout=outdata ;
    lbdrecurse = [&,d](int lvl, vector<double> p)
    {
      if (lvl<d-2)
      {
        p[lvl]=0+M_PI/npoints/2 ;
        for (int i=0 ; i<npoints ; i++)
        {
          lbdrecurse(lvl+1, p) ;
          p[lvl]+=M_PI/npoints ;
        }
      }
      else
      {
        p[lvl]=0+M_PI/npoints ;
        for (int i=0 ; i<2*npoints ; i++)
        {
          auto ptmp=p ; vector <uint8_t> a(3,0) ;
          phi2color(a.begin(), ptmp, d, colors) ;
          *pout=a[0] ; pout++ ;
          *pout=a[1] ; pout++ ;
          *pout=a[2] ; pout++ ;
					//printf("%d %d %d | %d %d %d #", a[0], a[1], a[2], *(pout-3), *(pout-2), *(pout-1)) ;
					p[lvl]+=M_PI/npoints ;
        }
      }
    } ;
    lbdrecurse(0,p) ;
    //---------------------------------------------------------------------
    nrrdWrap_nva(nval, outdata, nrrdTypeUChar, d+1, dimensions.data());
    string fullpath ;
    fullpath = path ;
    nrrdSave(fullpath.c_str(), nval, nio);
    free(outdata) ;
    printf("%s ", fullpath.c_str()) ;
#endif
return 0 ;
}
