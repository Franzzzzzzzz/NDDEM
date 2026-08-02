#include <random>
#include <optional>
enum class Windows {Rect3D, RectND, Hann3D,
                    Sphere3DIntersect, SphereNDIntersect, 
                    Lucy3D, Lucy3DFancyInt, Lucy3D_half, Lucy3D_scale, LucyND, LucyND_Periodic,
                    Dirac3D, MonteCarloIntersect3D, RVE} ;
using cquat = const Eigen::Quaternion<double> ;
/** \brief A window base class that needs to be specialised to a specific CG window
 */
class LibBase {
public:
    LibBase(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; }
    LibBase() {}
    virtual ~LibBase() {}

    struct Data * data ;
    double w, d ;
    std::optional<std::array<double,3>> mainaxis={} ; 

    virtual double window_weight (int, cv1d &, cquat &) = 0 ;  //Purely virtual, need to be defined in derived classes
    virtual std::pair<double,double> window_contact_weight (int p, int q, cv1d & loc, cquat & orient ) {
        double wpqs = window_avg (p, q, loc, orient) ;
        double wpqf = window_int (p, q, loc, orient) ;
        return (make_pair(wpqs, wpqf)) ;
    }    
    virtual double distance (int id, cv1d &loc) {double res=0 ; for (int i=0 ; i<d ; i++) res+=(loc[i]-data->pos[i][id])*(loc[i]-data->pos[i][id]) ; return sqrt(res) ; } ///< function for mixed particle id / vector informations.
    virtual double cutoff (void) {return 2.5*w ;}
    virtual double window_function(double r) {return 0 ;} ; 
    
    int export_window (std::string filename, cv1d & loc, cquat & orient)
    {
        int npts = 21 ;
        double dx=2*w/(double)(npts-1) ; 
        double radius = data->radius[0] ; 
        struct Data * backup = data ; 
        data = new struct Data ; 
        double location[3][1] ;
        data->pos.push_back(&location[0][0]) ;
        data->pos.push_back(&location[1][0]) ;
        data->pos.push_back(&location[2][0]) ;
        data->radius = &radius ; 
        
        FILE *out ; 
        out=fopen((filename+"-window.vtk").c_str(), "w") ;
        if (out==NULL) {printf("ERR: Cannot open file to write to.") ; return 1 ; }
        fprintf(out, "# vtk DataFile Version 2.0\nWindowFunction\nASCII\nDATASET STRUCTURED_POINTS\nDIMENSIONS %d %d %d\nORIGIN %g %g %g\nSPACING %g %g %g\n", npts, npts, npts, loc[0]-w, loc[1]-w, loc[2]-w, dx, dx, dx) ;
        fprintf(out, "POINT_DATA %d\n", npts*npts*npts) ;
        fprintf(out, "SCALARS windowvalue double \nLOOKUP_TABLE default \n") ;
        for (int z=0 ; z<npts ; z++)
        {
          for (int y=0 ; y<npts ; y++)
              for (int x=0 ; x<npts ; x++)
              {
                  location[0][0] = loc[0]-w + x*dx ; 
                  location[1][0] = loc[1]-w + y*dx ; 
                  location[2][0] = loc[2]-w + z*dx ; 
                  fprintf(out, "%g ", window_weight(0, loc, orient)) ; 
              }
          fprintf(out, "\n") ;
        }
        delete data ; 
        data = backup ; 
        fclose(out) ; 
        return 0 ; 
    }
    
protected: 
    virtual double window_int(int p, int q, cv1d & loc, cquat & orient) {return window_avg(p, q, loc, orient) ; }
    virtual double window_avg (int p, int q, cv1d & loc, cquat & orient) {return (0.5*(window_weight(p, loc, orient)+window_weight(q, loc, orient))) ; }
} ;
//---------------------
class LibLucy3D : public LibBase {
public :
    LibLucy3D(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; cst=105./(16*M_PI*w*w*w) ; }
    LibLucy3D() {} ;
    double Lucy (double r) {if (r>=w) return 0 ; else {double f=r/w ; return (cst*(-3*f*f*f*f + 8*f*f*f - 6*f*f +1)) ; }}
    double window_function(double r) {return Lucy(r) ; }
    double window_weight(int id, cv1d & loc, cquat & orient) { return Lucy(distance(id,loc)) ;}
    double cutoff (void) {return 2*w ;} // added by benjy to get rid of extra (hopefully useless) data
    //double window_int(double r1, double r2) {return window_avg(r1, r2) ; }
    //double window_avg (double r1, double r2) {return (0.5*(Lucy(r1)+Lucy(r2))) ; }
  protected:
    double cst ;
};
//--------------------
class LibLucy3D_half : public LibLucy3D {
public:     
    LibLucy3D_half(struct Data * D, double ww, double dd) : LibLucy3D(D, ww, dd) {mainaxis={1.,0.,0.} ; }
    double window_weight(int id, cv1d & loc, cquat & orient)
    {
        Eigen::Vector3d v(loc[0]-data->pos[0][id], loc[1]-data->pos[1][id], loc[2]-data->pos[2][id]);
        Eigen::Vector3d rotated = orient.conjugate() * v ; // Maybe is orient^-1 // .conjugate to do that
        if (rotated[0]<0) return Lucy(rotated.norm())*2 ; 
        else return 0 ;    
    }
    
} ;
//--------------------
class LibLucy3D_scale : public LibLucy3D {
public:     
    LibLucy3D_scale(struct Data * D, double ww, double dd) : LibLucy3D(D, ww, dd) {}
    double window_weight(int id, cv1d & loc, cquat & orient)
    {
        Eigen::Vector3d v(loc[0]-data->pos[0][id], loc[1]-data->pos[1][id], loc[2]-data->pos[2][id]);
        Eigen::Vector3d rotated = orient * v ; // Maybe is orient^-1 // .conjugate to do that
        rotated[0] *= scalex ; 
        rotated[1] *= scaley ; 
        rotated[2] *= scalez ; 
        return Lucy(rotated.norm()) ;
    }

private: 
    double scalex=1., scaley=1., scalez=1. ; 
} ;
//--------------------
class LibLucy3DFancyInt : public LibLucy3D {
public :
    LibLucy3DFancyInt (struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; cst=105./(16*M_PI*w*w*w) ;}
    std::pair<double,double> window_contact_weight (int p, int q, const v1d & loc, cquat & orient)
    {
      double res = 0 ;
      double prev, cur, first ;
      v1d locp (d,0) ;
      v1d lpq(d,0) ;
      for (int i=0 ; i<d ;i++)
      {
        lpq[i] = data->pos[i][q] - data->pos[i][p] ;
        locp[i] = data->pos[i][p] ;
      }

      //double length = 0 ; for (int i=0 ; i<d ;i++) length += lpq[i]*lpq[i] ; length = sqrt(length)/Nsteps ;

      first=prev=cur=Lucy(distance(p,loc)) ;
      for (int i=0 ; i<Nsteps ; i++)
      {
        for (int j=0; j<d ; j++)
            locp[j] += (1./Nsteps) * lpq[j] ;
        cur=Lucy(distancevec(locp,loc)) ;
        res += (cur+prev)/(2*Nsteps) ;
        //printf("%g | %g | %g %g %g\n", res, cur, distance(q,locp), Lucy(distance(p,loc)), Lucy(distance(q,loc))) ;
        prev=cur ;
      }
      //printf("%g %g \n", res, LibLucy3D::window_int(distance(p,loc),distance(q,loc))) ;
      return make_pair((Lucy(first)+Lucy(cur))/2.0,res) ;
    }
    double distancevec (v1d l1, v1d loc) {double res=0 ; for (int i=0 ; i<d ; i++) res+=(loc[i]-l1[i])*(loc[i]-l1[i]) ; return sqrt(res) ;}
    void set_integrationsteps (int steps) {if (steps<1) printf("Less than 1 step is not meaningful.") ; if (steps>1000) printf("You've chosen a very large number of integration steps, you may want to reconsider") ; Nsteps=steps ; }
private:
    int Nsteps = 10 ;
};



//---------------------
class LibRect3D : public LibBase {
public:
    LibRect3D(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; cst= 1./(4./3.*M_PI*w*w*w) ; }
    double cst ;
    double window_weight(int id, cv1d & loc, cquat & orient) {double r=distance(id, loc) ; return window_function(r) ; }
    double window_function(double r) {if (r>=w) return 0 ; else return cst ; }
};
//---------------------
class LibRectND : public LibBase {
public:
  LibRectND(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; }
  double window_weight(int id, cv1d & loc, cquat & orient) {double r=distance(id, loc) ; return window_function(r) ; }
    double window_function(double r) {if (r>=w) return 0 ; else {double a =1 ; for (int b=0 ; b<d ; b++,a*=w) {} return 1/a ;} }
};
//---------------------
class LibDirac3D : public LibBase {
public:
    LibDirac3D(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ;}   
    
    double window_weight (int id, cv1d &loc, cquat & orient)
    {
        double r = data->radius[id] ;
        double dst = distance(id, loc) ;
        if (dst > r)
            return 0;
        else
            return data->mass[id] / (4./3.*M_PI*r*r*r) ; 
    }
} ; 
//---------------------
class LibSphere3DIntersect : public LibBase {
public:
    LibSphere3DIntersect(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; cst=1./(4./3.*M_PI*w*w*w) ; }
    double cst ;
    double result ;
    double window_weight(int id, cv1d &loc, cquat & orient)
    {
        double dst=distance(id, loc) ; 
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
        if (data->N==0) return 2*w ;
        for (int i=0 ; i<data -> N ; i++)
            if (maxr<data->radius[i])
                maxr=data->radius[i] ;
        return maxr+w ;
    }
    virtual std::pair<double,double> window_contact_weight (int p, int q, cv1d & loc, cquat & orient)
    {
        double wpqs = window_avg (p, q, loc, orient) ;
        double wpqf = 0 ;

        v1d locp (d,0) ;
        v1d lpq(d,0) ;
        double normp=0, normlpq=0 ;
        double b = 0 ;
        for (int i=0 ; i<d ;i++)
        {
          lpq[i] = data->pos[i][q] - data->pos[i][p] ;
          locp[i] = data->pos[i][p]-loc[i] ;
          normp += locp[i]*locp[i] ;
          normlpq += lpq[i]*lpq[i] ;
          b += locp[i]*lpq[i] ;
        }
        b*=2 ;

        double Delta = b*b-4*normlpq*(normp-w*w) ;
        if (Delta<=0)
            wpqs = 0 ;
        else
        {
            double alpha1 = (-b - sqrt(Delta))/(2*normlpq) ;
            double alpha2 = (-b + sqrt(Delta))/(2*normlpq) ;

            if (alpha1<0 && alpha2>0 && alpha2<1)
                wpqf = cst * alpha2 ;
            else if (alpha1<0 && alpha2>1)
                wpqf = cst ;
            else if (alpha1>0 && alpha1<1 && alpha2>1)
                wpqf = cst * (1-alpha1) ;
            else if (alpha1>0 && alpha1<1 && alpha2>0 && alpha2<1) //this is a VERY small window ... ...
                wpqf = cst*(alpha2-alpha1) ;
            else
                wpqf = 0 ;

        }
        return (make_pair(wpqs,wpqf)) ;
    }
};
//------------------------------------------
class LibSphereNDIntersect : public LibBase {
public:
    LibSphereNDIntersect(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; cst=1./(nballvolume()* pow(w, d)) ; }
    double nballvolume () {return pow(M_PI,d/2.)/tgamma(d/2.+1) ; }
    double ncapvolume (double r, double h)
    {
      if (h<0 || h>2*r) return 0 ;
      if (h<=r)
      {
        //printf("%g %g\n", 1/2. * nballvolume() * pow(r, d) * boost::math::ibeta ((d+1)/2., 1/2., (2*r*h-h*h)/(r*r)), M_PI*h*h/3*(3*r-h)) ;
        return 1/2. * nballvolume() * pow(r, d) * boost::math::ibeta ((d+1)/2., 1/2., (2*r*h-h*h)/(r*r)) ;
      }
      else
      {
        //printf("%g %g %g %g\n", h, r, nballvolume() * pow(r, d) - 1/2. * nballvolume() * pow(r, d) * (boost::math::ibeta((d+1)/2., 1/2., (2*r*(2*r-h)-(2*r-h)*(2*r-h))/(r*r))),
        //  4/3.*M_PI*r*r*r - M_PI*(2*r-h)*(2*r-h)/3*(3*r-(2*r-h))) ;
        return nballvolume() * pow(r, d) - 1/2. * nballvolume() * pow(r, d) * boost::math::ibeta((d+1)/2., 1/2., (2*r*(2*r-h)-(2*r-h)*(2*r-h))/(r*r)) ; // Completement normalised beta function (1-I_x(a,b))
      }
    }
    double cst ;
    double result ;
    double window_weight (int id, cv1d & loc, cquat & orient)
    {
        double r=distance(id, loc) ;
        double r2 = data->radius[id] ;
        double &r1 = w ;
        if (r>r1+r2) return 0 ;
        else if (r<=fabs(r1-r2))
        {
            if (r1>r2)
                return cst ;
            else
                return 1./(nballvolume()*pow(r2, d)) ;
        }
        else
        {
            double h1 = (r2+r1-r)*(r2-r1+r)/(2*r) ;
            double h2 = (r1+r2-r)*(r1-r2+r)/(2*r) ;

            double vol = ncapvolume(r1,h1) + ncapvolume(r2,h2) ;
            return vol * cst / (nballvolume() * pow(r2,d)) ;
        }
    }
    double cutoff (void) {
        double maxr=0 ;
        if (data->N==0) return 2*w ;
        for (int i=0 ; i<data -> N ; i++)
            if (maxr<data->radius[i])
                maxr=data->radius[i] ;
        return maxr+w ;
    }
    virtual std::pair<double,double> window_contact_weight (int p, int q, cv1d & loc, cquat & orient)
    {
        double wpqs = window_avg (p, q, loc, orient) ;
        double wpqf = 0 ;

        v1d locp (d,0) ;
        v1d lpq(d,0) ;
        double normp=0, normlpq=0 ;
        double b = 0 ;
        for (int i=0 ; i<d ;i++)
        {
          lpq[i] = data->pos[i][q] - data->pos[i][p] ;
          locp[i] = data->pos[i][p]-loc[i] ;
          normp += locp[i]*locp[i] ;
          normlpq += lpq[i]*lpq[i] ;
          b += locp[i]*lpq[i] ;
        }
        b*=2 ;

        double Delta = b*b-4*normlpq*(normp-w*w) ;
        if (Delta<=0)
            wpqs = 0 ;
        else
        {
            double alpha1 = (-b - sqrt(Delta))/(2*normlpq) ;
            double alpha2 = (-b + sqrt(Delta))/(2*normlpq) ;

            if (alpha1<0 && alpha2>0 && alpha2<1)
                wpqf = cst * alpha2 ;
            else if (alpha1<0 && alpha2>1)
                wpqf = cst ;
            else if (alpha1>0 && alpha1<1 && alpha2>1)
                wpqf = cst * (1-alpha1) ;
            else if (alpha1>0 && alpha1<1 && alpha2>0 && alpha2<1) //this is a VERY small window ... ...
                wpqf = cst*(alpha2-alpha1) ;
            else
                wpqf = 0 ;

        }
        return (make_pair(wpqs,wpqf)) ;
    }
};
//---------------------------------
class LibMonteCarloIntersect3D : public LibBase {
public: 
    LibMonteCarloIntersect3D (struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; 
        gen=std::mt19937(123457); random=std::uniform_real_distribution<double>(-1.0, 1.0) ; }
    
    double window_weight (int id, cv1d & loc, cquat & orient)
    {
        if (win == nullptr) printf("ERROR: window needs to be set to use the LibMonteCarlo Intersect") ; 
        
        if (data->superquadric.size()==0 || data->superquadric[0] == nullptr)
        {
            //printf("INFO: prefer LibSphere3DIntersect for spherical particles\n") ; 
            double weight=0 ;
            double k[3] = {loc[0]-data->pos[0][id], loc[1]-data->pos[1][id], loc[2]-data->pos[2][id]} ; 
            double x, y, z ;             
            for (int i=0 ; i<Nmc ; i++)
            {
                //x=random(gen) ; y=random(gen) ; z=random(gen) ; //[-1,1]
                //if ( x*x+y*y+z*z<1)
                double r = cbrt(abs(random(gen))) ; 
                double costheta=random(gen) ; 
                double phi = random(gen)*M_PI ; 
                double sintheta = sqrt(1-costheta*costheta) ; 
            
                x = r*sintheta*cos(phi) ;
                y = r*sintheta*sin(phi) ; 
                z = r*costheta ;                 
                if ( (x*data->radius[id] - k[0])*(x*data->radius[id] - k[0]) + 
                         (y*data->radius[id] - k[1])*(y*data->radius[id] - k[1]) + 
                         (z*data->radius[id] - k[2])*(z*data->radius[id] - k[2]) < w*w)
                    {weight += win->window_function(r) ;}
            }
            return weight/(double)Nmc ; 
        }
        else
        {
            Eigen::Quaternion<double> orient(data->orient[0][id], data->orient[1][id], data->orient[2][id], data->orient[3][id]) ; 
            Eigen::Quaternion<double> k(0, loc[0]-data->pos[0][id], loc[1]-data->pos[1][id], loc[2]-data->pos[2][id]) ; 
            Eigen::Quaternion<double> kprime = orient.inverse() * k * orient ; 
            
            if ( kprime.x()*kprime.x()+kprime.z()*kprime.z()+kprime.z()*kprime.z() > pow(w+std::max(data->superquadric[0][id], std::max(data->superquadric[1][id], data->superquadric[2][id])),2))
                return 0 ; 
            
            double weight=0 ;
            double x, y, z ; 
            for (int i=0 ; i<Nmc ; i++)
            {
                double r = cbrt(abs(random(gen))) ;
                double costheta=random(gen) ;
                double phi = random(gen)*M_PI ;
                double sintheta = sqrt(1-costheta*costheta) ;

                x = r*sintheta*cos(phi) ;
                y = r*sintheta*sin(phi) ;
                z = r*costheta ;
                //printf("%g %g %g %g %g %g\n", data->superquadric[0][id], data->superquadric[1][id], data->superquadric[2][id], data->superquadric[3][id], data->superquadric[4][id], data->superquadric[5][id]) ;

                if (pow(x, data->superquadric[3][id]) + pow(y, data->superquadric[4][id]) + pow(z, data->superquadric[5][id])<1) // In superquadric? 
                 if ( (x*data->superquadric[0][id]-kprime.x())*(x*data->superquadric[0][id]-kprime.x()) + 
                      (y*data->superquadric[1][id]-kprime.y())*(y*data->superquadric[1][id]-kprime.y()) + 
                      (z*data->superquadric[2][id]-kprime.z())*(z*data->superquadric[2][id]-kprime.z()) < w*w ) // In sphere?
                      {weight += win->window_function(r) ;}
            }
            return weight/(double)Nmc ;
        }
    }    
    double cutoff (void) {
        double maxr=0 ;
        if (data->N==0) return 2*w ;
        for (int i=0 ; i<data -> N ; i++)
            if (maxr<data->radius[i])
                maxr=data->radius[i] ;
        return maxr+w ;
    }
    double windowreal (double r) {return win->window_function(r) ; }
    virtual double window_avg (double r1, double r2) {return (0.5*(windowreal(r1)+windowreal(r2))) ; } // TODO this may be quite wrong, unclear.
    
    LibBase * win = nullptr ; 
    
private: 
    int Nmc=100 ;
    std::mt19937 gen;
    std::uniform_real_distribution<double> random;    
    
};

//---------------------
class LibHann3D : public LibBase {
public:
    LibHann3D(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; cst= 3*M_PI/(2.*(M_PI*M_PI-6)*w*w*w) ; }
    double cst ;
    double window_weight (int id, cv1d & loc, cquat & orient) {double r = distance(id, loc) ; return window_function(r) ; }
    double window_function (double r) {if (r>=w) return 0 ; else return cst*pow(cos(M_PI*r/(2.*w)),2) ;}
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
  double windows_function(double r) {return Lucy(r) ; }
  double window_weight (int id, cv1d & loc, cquat & orient) override {double r=distance(id, loc) ; return (Lucy(r)) ;}
} ;
//---------------------
class LibLucyND_Periodic : public LibLucyND
{
  public :
  LibLucyND_Periodic(struct Data * D, double ww, double dd, int periodic, vector<int>boxes, vector<double> deltas)
    : LibLucyND(D,ww,dd-__builtin_popcount(periodic))
  {
    printf("%g ", scale) ;
    maskperiodic = 0 ;
    for (size_t i =0 ; i<boxes.size() ; i++)
      if ((periodic&(1<<i)) && boxes[i]==1)
      {
        maskperiodic |= (1<<i) ;
        scale /= deltas[i] ;
      }
    printf("NB: do not use periodic_atoms with LibLucyND_Periodic! %g \n", scale) ;
  }

  double distance (int id, cv1d & loc) override
  {
    double res=0 ;
    for (int i=0 ; i<d ; i++)
      if ((maskperiodic&(1<<i))==0)
        res+=(loc[i]-data->pos[i][id])*(loc[i]-data->pos[i][id]) ;
    return sqrt(res) ;
  }

  int maskperiodic ;
};
//--------------------
class LibRVE: public LibBase
{
public :
    LibRVE (double s) { scale = s ; }

    double window_weight(int id, cv1d & loc, cquat & orient) override {return scale ;}
    double distance (int id, cv1d & loc) override {return 0. ;}
    void set_volume (double vol) { scale = 1./vol ; }

    double scale=1 ;
} ;
