#include <random>
#include <optional>
#include "ShapeLibrary.h"
enum class Windows {Rect3D, Rect3D_half, Rect3D_halfinterface, Rect3D_scale, RectND, Hann3D,
                    Sphere3DIntersect, SphereNDIntersect, 
                    Lucy3D, Lucy3DFancyInt, Lucy3D_half, Lucy3D_halfinterface, Lucy3D_scale, LucyND, LucyND_Periodic,
                    Dirac3D, MonteCarloIntersect3D, MonteCarloOrientationConvolution, RVE} ;
enum class AngularWindows {World, RectAxiSymmetric, LucyAxiSymmetric} ; 
using cquat = const Eigen::Quaternion<double> ;
/** \brief A window base class that needs to be specialised to a specific CG window
 */
class LibBase {
public:
    LibBase(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ;}
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
    virtual double cutoff (void) {return w ;}
    virtual double window_function(double r) {return 0 ;} ; 
    
    virtual std::tuple<std::array<double,3>,double> get_sampling_point (cv1d& loc, cquat & orient, std::array<double,3> scaling = {1.0,1.0,1.0}, uint8_t halved=0) {
        // Rejection sampling, tested faster than spherical sampling
        std::array<double,3> res ; 
        while (true) 
        {
          res[0] = random(gen) ; 
          res[1] = random(gen) ; 
          res[2] = random(gen) ;
          double r = res[0]*res[0] + res[1]*res[1] + res[2]*res[2] ; 
          if (halved==1 && res[0]<0.) continue ; 
          if (r<1.0)
          {
           double winvalue = window_function(sqrt(r)*w) ; 
           if (halved==2 && res[0]<0.) winvalue = -winvalue ; 
           res[0] *= w*scaling[0] ; res[1] *= w*scaling[1] ; res[2] *= w*scaling[2] ;
           Eigen::Vector3d rotated = orient * Eigen::Vector3d(res[0], res[1], res[2]) ;        
           rotated[0] += loc[0] ; rotated[1] += loc[1] ; rotated[2]+= loc[2] ; 
           return {{rotated(0), rotated(1), rotated(2)},winvalue} ;  
          }
        }
    }
    virtual double get_sampling_volume () { return 4./3.*M_PI*w*w*w ; }
    
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
    virtual void disp_name() {printf("=> %s\n", name.c_str()) ; }
    void reset_rng(int seed)
    {
        gen.seed(seed) ; 
        random.reset() ; 
    }
    
protected: 
    virtual double window_int(int p, int q, cv1d & loc, cquat & orient) {return window_avg(p, q, loc, orient) ; }
    virtual double window_avg (int p, int q, cv1d & loc, cquat & orient) {return (0.5*(window_weight(p, loc, orient)+window_weight(q, loc, orient))) ; }
    std::mt19937_64 gen=std::mt19937_64(1234567); // general use random generator 
    std::uniform_real_distribution<double> random=std::uniform_real_distribution<double>(-1.0, 1.0) ;;
    std::string name ; 
} ;

//---------------------------------------------------------------
class LibMonteCarloIntersect3D : public LibBase {
public: 
    LibMonteCarloIntersect3D (struct Data * D, double ww, double dd) { name="MonteCarloIntersect3D"; data=D; w=ww ; d=dd ;}
    
    double window_weight (int id, cv1d & loc, cquat & orient)
    {        
        if (win == nullptr) printf("ERROR: window needs to be set to use the LibMonteCarlo Intersect") ; 
        win->reset_rng(1234567) ; 
        
        double total=0 ; 
        for (int i=0 ; i<Nmc ; i++)
        {
            auto [spt, winweight] = win->get_sampling_point(loc, orient) ;
            total += shape->weight(id, spt) * winweight ; 
        }
        total = win->get_sampling_volume() * total / static_cast<double>(Nmc) ; 
        
        return total ; 
    }    
    double cutoff (void) { return win->cutoff() + shape->cutoff() ; }
    double windowreal (double r) {return win->window_function(r) ; }
    virtual double window_avg (double r1, double r2) {return (0.5*(windowreal(r1)+windowreal(r2))) ; } // TODO this may be quite wrong, unclear.
    void set_nsample(int n) {Nmc = n ;}
    
    LibBase * win = nullptr ; 
    LibShape * shape = nullptr ; 
    
    void disp_name() override {
        printf("=> %s\n", name.c_str()) ; 
        printf("\t ") ; win->disp_name() ;
        printf("\t ") ; shape->disp_name() ;
    }
    
private: 
    int Nmc=100 ;
    
};
//--------------------------------------------------
class LibAngular : public LibBase {
public: 
    LibAngular (struct Data * D, double ww, double dd) {name="LibAngular"; data=D; w=ww ; d=dd ;}

    double window_weight (int id, cv1d & loc, cquat & orient)
    {
     double angularwinweight = angularwin->window_weight(id, loc, orient) ; 
     if (angularwinweight == 0)
         return 0 ;
     else
        return  angularwinweight * spatialwin->window_weight(id, loc, orient) ;    
    }
    
    void disp_name() override {
        printf("=> %s\n", name.c_str()) ; 
        printf("\t # ") ; spatialwin->disp_name() ;
        printf("\t ° ") ; angularwin->disp_name() ;
    }    
    LibBase * spatialwin = nullptr ; 
    LibBase * angularwin = nullptr ; 
} ; 


class LibRect_AngularAxisymmetric : public LibBase {
public:
    LibRect_AngularAxisymmetric (struct Data *D, double ww, double dd) {name="Rect_AngularAxisymmetric"; data=D; w=ww ; d=dd ; cst = 1.0 ; /*1.0/(1.0-cos(w)) ;*/} //Normalisation for axisymmetric particles
    double window_weight (int id, cv1d & loc, cquat & orient)
    {
        double dst = shape->angulardistance(id, orient) ; 
        if (dst > w) return 0.0;
        return cst ; 
    }    
    LibShape * shape = nullptr ; // Eventually if want to use anything other than dirac ...
private:
    double cst ; 
} ;

class LibLucy_AngularAxisymmetric : public LibBase {
public:
    LibLucy_AngularAxisymmetric (struct Data *D, double ww, double dd) {name="Lucy_AngularAxisymmetric"; data=D; w=ww ; d=dd ; cst = lucyNormalization(w) ;} //Normalisation for axisymmetric particles
    double window_weight (int id, cv1d & loc, cquat & orient)
    {
        double dst = shape->angulardistance(id, orient) ; 
        if (dst > w) return 0.0;
        
        const double s = dst / w;
        const double oneMinusS = 1.0 - s;
     
        return cst * (1.0 + 3.0 * s) * oneMinusS * oneMinusS * oneMinusS;
    }
    double lucyNormalization(double t)
    {
        const double denom = std::pow(t, 4) + 12.0 * t * t  + 24.0 * t * std::sin(t) + 72.0 * std::cos(t) - 72.0;
        return std::pow(t, 4) / denom;
    }
    LibShape * shape = nullptr ; // Eventually if want to use anything other than dirac ...
private:
    double cst ; 
} ;


//---------------------
class LibLucy3D : public LibBase {
public :
    LibLucy3D(struct Data * D, double ww, double dd) { name="Lucy3D"; data=D; w=ww ; d=dd ; cst=105./(16*M_PI*w*w*w) ; }
    LibLucy3D() {} ;
    double Lucy (double r) {if (r>=w) return 0 ; else {double f=r/w ; return (cst*(-3*f*f*f*f + 8*f*f*f - 6*f*f +1)) ; }}
    double window_function(double r) {return Lucy(r) ; }
    double window_weight(int id, cv1d & loc, cquat & orient) { return Lucy(distance(id,loc)) ;}
    
    virtual double cutoff() override {return w ;}
    //double cutoff (void) {return 2*w ;} // added by benjy to get rid of extra (hopefully useless) data //Not sure what that is
    //double window_int(double r1, double r2) {return window_avg(r1, r2) ; }
    //double window_avg (double r1, double r2) {return (0.5*(Lucy(r1)+Lucy(r2))) ; }
  protected:
    double cst ;
};
//--------------------
class LibLucy3D_half : public LibLucy3D {
public:     
    LibLucy3D_half(struct Data * D, double ww, double dd) : LibLucy3D(D, ww, dd) {name="Lucy3D_half"; mainaxis={1.,0.,0.} ; }
    double window_weight(int id, cv1d & loc, cquat & orient)
    {
        Eigen::Vector3d v(data->pos[0][id]-loc[0], data->pos[1][id]-loc[1], data->pos[2][id]-loc[2]); 
        Eigen::Vector3d rotated = orient.conjugate() * v ; // Maybe is orient^-1 // .conjugate to do that
        if (rotated[0]>0) return window_function(rotated.norm()) ; 
        else return 0. ;    
    }
    double window_function(double r) { return LibLucy3D::window_function(r)*2. ; }
    virtual std::tuple<std::array<double,3>,double> get_sampling_point (cv1d& loc, cquat & orient, std::array<double,3> scaling = {1.0,1.0,1.0}, uint8_t halved=0) 
    {   return LibBase::get_sampling_point(loc, orient, {1.,1.,1.}, true) ; }
    virtual double get_sampling_volume () { return LibBase::get_sampling_volume()/2.0 ; }
} ;
//--------------------
class LibLucy3D_halfinterface : public LibLucy3D {
public:     
    LibLucy3D_halfinterface(struct Data * D, double ww, double dd) : LibLucy3D(D, ww, dd) {name="Lucy3D_halfinterface"; mainaxis={1.,0.,0.} ; }
    double window_weight(int id, cv1d & loc, cquat & orient)
    {
        Eigen::Vector3d v(data->pos[0][id]-loc[0], data->pos[1][id]-loc[1], data->pos[2][id]-loc[2]); 
        Eigen::Vector3d rotated = orient.conjugate() * v ; // Maybe is orient^-1 // .conjugate to do that
        if (rotated[0]>0) return window_function(rotated.norm()) ;
        else return -window_function(rotated.norm()) ;    
    }
    double window_function(double r) { return LibLucy3D::window_function(r) ; }
    virtual std::tuple<std::array<double,3>,double> get_sampling_point (cv1d& loc, cquat & orient, std::array<double,3> scaling = {1.0,1.0,1.0}, uint8_t halved=0) 
    {   return LibBase::get_sampling_point(loc, orient, {1.,1.,1.}, 2) ; }
    virtual double get_sampling_volume () { return LibBase::get_sampling_volume() ; }
} ;
//--------------------
class LibLucy3D_scale : public LibLucy3D {
public:     
    LibLucy3D_scale(struct Data * D, double ww, double dd) : LibLucy3D(D, ww, dd) {name="Lucy3D_scale";}
    double window_weight(int id, cv1d & loc, cquat & orient)
    {
        Eigen::Vector3d v(data->pos[0][id]-loc[0], data->pos[1][id]-loc[1], data->pos[2][id]-loc[2]);
        Eigen::Vector3d rotated = orient.conjugate() * v ; // Maybe is orient^-1 // .conjugate to do that
        rotated[0] /= scalex ; 
        rotated[1] /= scaley ; 
        rotated[2] /= scalez ; 
        return Lucy(rotated.norm()) ;
    }
    void set_scale(double sx, double sy, double sz) {scalex=sx; scaley=sy; scalez=sz ; cst /= (sx*sy*sz) ;}
    virtual std::tuple<std::array<double,3>,double> get_sampling_point (cv1d& loc, cquat & orient, std::array<double,3> scaling = {1.0,1.0,1.0}) 
    { return LibBase::get_sampling_point(loc, orient, {scalex, scaley, scalez}) ; }
    virtual double get_sampling_volume () { return 4./3.*M_PI*w*w*w*scalex*scaley*scalez ; }
    double cutoff() override {return w*std::max(scalex, std::max(scaley, scalez)) ; }
    
private: 
    double scalex=1., scaley=1., scalez=1. ; 
} ;

//--------------------
class LibLucy3DFancyInt : public LibLucy3D {
public :
    LibLucy3DFancyInt (struct Data * D, double ww, double dd) { name="Lucy3DFancyInt"; data=D; w=ww ; d=dd ; cst=105./(16*M_PI*w*w*w) ; }
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
    LibRect3D(struct Data * D, double ww, double dd) { name="Rect3D"; data=D; w=ww ; d=dd ; cst= 1./(4./3.*M_PI*w*w*w) ; }
    double cst ;
    double window_weight(int id, cv1d & loc, cquat & orient) {double r=distance(id, loc) ; return window_function(r) ; }
    double window_function(double r) {if (r>=w) return 0 ; else return cst ; }
};
//---------------------
class LibRect3D_half : public LibRect3D {
public:     
    LibRect3D_half(struct Data * D, double ww, double dd) : LibRect3D(D, ww, dd) {name="Rect3D_half"; mainaxis={1.,0.,0.} ; }
    double window_weight(int id, cv1d & loc, cquat & orient)
    {
        Eigen::Vector3d v(data->pos[0][id]-loc[0], data->pos[1][id]-loc[1], data->pos[2][id]-loc[2]); //TODO change EVERYWHERE
        Eigen::Vector3d rotated = orient.conjugate() * v ; // Maybe is orient^-1 // .conjugate to do that
        if (rotated[0]>0) return window_function(rotated.norm()) ; 
        else return 0. ;    
    }
    double window_function(double r) { return LibRect3D::window_function(r)*2. ; }
    virtual std::tuple<std::array<double,3>,double> get_sampling_point (cv1d& loc, cquat & orient, std::array<double,3> scaling = {1.0,1.0,1.0}, uint8_t halved=0) 
    {   return LibBase::get_sampling_point(loc, orient, {1.,1.,1.}, true) ; }
    virtual double get_sampling_volume () { return LibBase::get_sampling_volume()/2.0 ; }
} ;
//---------------------
class LibRect3D_halfinterface : public LibRect3D {
public:     
    LibRect3D_halfinterface(struct Data * D, double ww, double dd) : LibRect3D(D, ww, dd) {name="Rect3D_halfinterface"; mainaxis={1.,0.,0.} ; }
    double window_weight(int id, cv1d & loc, cquat & orient)
    {
        Eigen::Vector3d v(data->pos[0][id]-loc[0], data->pos[1][id]-loc[1], data->pos[2][id]-loc[2]); //TODO change EVERYWHERE
        Eigen::Vector3d rotated = orient.conjugate() * v ; // Maybe is orient^-1 // .conjugate to do that
        if (rotated[0]>0) return window_function(rotated.norm()) ; 
        else return -window_function(rotated.norm()) ; 
    }
    double window_function(double r) { return LibRect3D::window_function(r) ; }
    virtual std::tuple<std::array<double,3>,double> get_sampling_point (cv1d& loc, cquat & orient, std::array<double,3> scaling = {1.0,1.0,1.0}, uint8_t halved=0) 
    {   return LibBase::get_sampling_point(loc, orient, {1.,1.,1.}, 2) ; }
    virtual double get_sampling_volume () { return LibBase::get_sampling_volume() ; }
} ;
//---------------------
class LibRect3D_scale : public LibRect3D {
public:
    LibRect3D_scale(struct Data * D, double ww, double dd) : LibRect3D(D, ww, dd) {name="Rect3D_scale"; }
    double window_weight(int id, cv1d & loc, cquat & orient) 
    {
        Eigen::Vector3d v(data->pos[0][id]-loc[0], data->pos[1][id]-loc[1], data->pos[2][id]-loc[2]);
        Eigen::Vector3d rotated = orient.conjugate() * v ; // Maybe is orient^-1 // .conjugate to do that
        rotated[0] /= scalex ; 
        rotated[1] /= scaley ; 
        rotated[2] /= scalez ; 
        return window_function(rotated.norm()) ;
    }
    void set_scale(double sx, double sy, double sz) {scalex=sx; scaley=sy; scalez=sz ; cst /= (sx*sy*sz) ; }
    virtual std::tuple<std::array<double,3>,double> get_sampling_point (cv1d& loc, cquat & orient, std::array<double,3> scaling = {1.0,1.0,1.0}) 
    {   return LibBase::get_sampling_point(loc, orient, {scalex, scaley, scalez}) ; }
    virtual double get_sampling_volume () { return 4./3.*M_PI*w*w*w*scalex*scaley*scalez ; }
    double cutoff() override {return w*std::max(scalex, std::max(scaley, scalez)) ; }
    
private: 
    double scalex=1., scaley=1., scalez=1. ; 
};

//---------------------
class LibRectND : public LibBase {
public:
  LibRectND(struct Data * D, double ww, double dd) { name="RectND"; data=D; w=ww ; d=dd ; }
  double window_weight(int id, cv1d & loc, cquat & orient) {double r=distance(id, loc) ; return window_function(r) ; }
  double window_function(double r) {if (r>=w) return 0 ; else {double a =1 ; for (int b=0 ; b<d ; b++,a*=w) {} return 1/a ;} }
};
//---------------------
class LibDirac3D : public LibBase {
public:
    LibDirac3D(struct Data * D, double ww, double dd) { name="Dirac3D"; data=D; w=ww ; d=dd ;}   
    
    double window_weight (int id, cv1d &loc, cquat & orient)
    {
        return shape->weight(id, {loc[0], loc[1], loc[2]} ) * data->mass[id]/shape->volume(id) ; 
    }
    double cutoff() {return  shape->cutoff() ; }
    
    LibShape *shape ; 
    
} ; 
//---------------------
class LibSphere3DIntersect : public LibBase {
public:
    LibSphere3DIntersect(struct Data * D, double ww, double dd) { name="Sphere3DIntersect"; data=D; w=ww ; d=dd ; cst=1./(4./3.*M_PI*w*w*w) ; }
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
        if (data->N==0) 
            return 2*w ;
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
    LibSphereNDIntersect(struct Data * D, double ww, double dd) { name="SphereNDIntersect"; data=D; w=ww ; d=dd ; cst=1./(nballvolume()* pow(w, d)) ; }
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
//---------------------
class LibHann3D : public LibBase {
public:
    LibHann3D(struct Data * D, double ww, double dd) { name="Hann3D"; data=D; w=ww ; d=dd ; cst= 3*M_PI/(2.*(M_PI*M_PI-6)*w*w*w) ; }
    double cst ;
    double window_weight (int id, cv1d & loc, cquat & orient) {double r = distance(id, loc) ; return window_function(r) ; }
    double window_function (double r) {if (r>=w) return 0 ; else return cst*pow(cos(M_PI*r/(2.*w)),2) ;}
} ;
//---------------------
class LibLucyND : public LibBase {
public:
  LibLucyND(struct Data * D, double ww, double dd) 
    {
        name="LucyND" ;
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
    name = "LucyND_Periodic";
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
    LibRVE (double s) { name="RVE"; scale = s ; }

    double window_weight(int id, cv1d & loc, cquat & orient) override {return scale ;}
    double distance (int id, cv1d & loc) override {return 0. ;}
    void set_volume (double vol) { scale = 1./vol ; }

    double scale=1 ;
} ;
