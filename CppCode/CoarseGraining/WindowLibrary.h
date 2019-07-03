

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
} ;

class LibLucy3D : public LibBase {
public :
    LibLucy3D(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; }
    double Lucy (double r) {static double cst=105./(16*M_PI*w*w*w) ; if (r>=w) return 0 ; else {double f=r/w ; return (cst*(-3*f*f*f*f + 8*f*f*f - 6*f*f +1)) ; }}
    double window(double r) {return (Lucy(r)) ;}
    double window_int(double r1, double r2) {return window_avg(r1, r2) ; }
    double window_avg (double r1, double r2) {return (0.5*(Lucy(r1)+Lucy(r2))) ; }
};

class LibRect3D : public LibBase {
public:
    const double cst = 8/(w*w*w) ;
    double window (double r) {if (r>=w) return 0 ; else return cst ;}
};

class LibRectND : public LibBase {
public:
  LibRectND(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; }
  double window (double r) {if (r>=w) return 0 ; else {double a =1 ; for (int b=0 ; b<d ; b++,a*=w) ; return a ;}}
};

class LibLucy1DPBC : public LibBase {
public:
    double scale ; // integral over all the non lucy dimensions
    double Lucy (double r) {static double cst=2*w/5. ; if (r>=w) return 0 ; else {double f=r/w ; return (cst*(-3*f*f*f*f + 8*f*f*f - 6*f*f +1)) ; }}
    double windows (double r) {return (Lucy(r)*scale) ; }
    double distance (int id, v1d loc) {return(fabs(loc[0]-data->pos[0][id])) ;}
};
