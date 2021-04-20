
enum class Windows {Rect3D, Rect3DIntersect, Lucy3D, Hann3D, RectND, LucyND, LucyND_Periodic} ;

/** \brief A window base class that needs to be specialised to a specific CG window
 */
class LibBase {
public:
    LibBase(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; }
    LibBase() {}

    struct Data * data ;
    double w, d ;

    virtual double window (double) = 0 ;  //Purely virtual, need to be defined in derived classes
    virtual double window_int(double r1, double r2) {return window_avg(r1, r2) ; }
    virtual double window_avg (double r1, double r2) {return (0.5*(window(r1)+window(r2))) ; }
    virtual double distance (int id, v1d loc) {double res=0 ; for (int i=0 ; i<d ; i++) res+=(loc[i]-data->pos[i][id])*(loc[i]-data->pos[i][id]) ; return sqrt(res) ; } ///< function for mixed particle id / vector informations.
    virtual double cutoff (void) {return 2.5*w ;} 
} ;
//---------------------
class LibLucy3D : public LibBase {
public :
    LibLucy3D(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; }
    double Lucy (double r) {static double cst=105./(16*M_PI*w*w*w) ; if (r>=w) return 0 ; else {double f=r/w ; return (cst*(-3*f*f*f*f + 8*f*f*f - 6*f*f +1)) ; }}
    double window(double r) {return (Lucy(r)) ;}
    double window_int(double r1, double r2) {return window_avg(r1, r2) ; }
    double window_avg (double r1, double r2) {return (0.5*(Lucy(r1)+Lucy(r2))) ; }
};
//---------------------
class LibRect3D : public LibBase {
public:
    LibRect3D(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; cst= 1./(4./3.*M_PI*w*w*w) ; }
    double cst ;
    double window (double r) {if (r>=w) return 0 ; else return cst ;}
};
//---------------------
class LibRectND : public LibBase {
public:
  LibRectND(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; }
  double window (double r) {if (r>=w) return 0 ; else {double a =1 ; for (int b=0 ; b<d ; b++,a*=w) ; return 1/a ;}}
};
//---------------------
class LibRect3DIntersect : public LibBase {
public:
    LibRect3DIntersect(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; cst=1./(4./3.*M_PI*w*w*w) ; }
    double cst ; 
    double result ;
    double distance(int id, v1d loc) 
    {
        double dst=0 ; 
        for (int i=0 ; i<d ; i++) 
            dst+=(loc[i]-data->pos[i][id])*(loc[i]-data->pos[i][id]) ; 
        dst= sqrt(dst) ;
        double r = data->radius[id] ; 
        if (dst>w+r) return 0 ; 
        else if (dst<=fabs(w-r)) 
        {
            if (w>r)
                return cst ;
            else 
                return (1/(4./3. * M_PI * r*r*r)) ; 
        }
        else
        { 
            double vol = M_PI * (w+r-dst) * (w+r-dst) * (dst*dst + 2*dst*r - 3*r*r + 2*dst*w + 6*r*w - 3*w*w) / (12.*dst) ;
            return (vol * cst / (4./3.*M_PI*r*r*r)) ; 
        }
        
    }
    double cutoff (void) {
        double maxr=0 ;
        for (int i=0 ; i<data -> N ; i++)
            if (maxr<data->radius[i]) 
                maxr=data->radius[i] ; 
        return maxr+w ; 
    }
    double window (double r) {return r ; } // The value calculated by the distance measurement is the one returned, a bit of a hack there ...
};
//---------------------
class LibHann3D : public LibBase {
public:
    LibHann3D(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; cst= 3*M_PI/(2.*(M_PI*M_PI-6)*w*w*w) ; }
    double cst ;
    double window (double r) {if (r>=w) return 0 ; else return cst*pow(cos(M_PI*r/(2.*w)),2) ;}
} ; 
//---------------------
class LibLucyND : public LibBase {
public:
  LibLucyND(struct Data * D, double ww, double dd)
    {
        data=D; w=ww ; d=dd ;
        double Vol=pow(M_PI,d/2.)/(tgamma(d/2.+1)) ; // N-ball volume
        scale = Vol * d * (-3./(d+4) + 8./(d+3) - 6./(d+2) + 1./d) ;
        scale = 1/scale ;
        scale = scale / (pow(w, d)) ;
        printf("Lucy function scaling : %f \n", scale) ;
    }
  double scale ;
  double Lucy (double r) {if (r>=w) return 0 ; else {double f=r/w ; return (scale*(-3*f*f*f*f + 8*f*f*f - 6*f*f +1)) ; }}
  double window(double r) {return (Lucy(r)) ;}
} ;
//---------------------
class LibLucyND_Periodic : public LibLucyND
{
  public :
  LibLucyND_Periodic(struct Data * D, double ww, double dd, int periodic, vector<int>boxes, vector<double> deltas)
    : LibLucyND(D,ww,dd-__builtin_popcount(periodic))
  {
    maskperiodic = 0 ;
    for (int i =0 ; i<boxes.size() ; i++)
      if ((periodic&(1<<i)) && boxes[i]==1)
      {
        maskperiodic |= (1<<i) ;
        scale /= deltas[i] ;
      }
    printf("NB: do not use periodic_atoms with LibLucyND_Periodic!\n") ;
  }

  double distance (int id, v1d loc)
  {
    double res=0 ;
    for (int i=0 ; i<d ; i++)
      if ((maskperiodic&(1<<i))==0)
        res+=(loc[i]-data->pos[i][id])*(loc[i]-data->pos[i][id]) ;
    return sqrt(res) ;
  }

  int maskperiodic ;
};
