
enum class Windows {Rect3D, Sphere3DIntersect, SphereNDIntersect, Lucy3D, Lucy3DFancyInt,  Hann3D, RectND, LucyND, LucyND_Periodic} ;

/** \brief A window base class that needs to be specialised to a specific CG window
 */
class LibBase {
public:
    LibBase(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; }
    LibBase() {}
    virtual ~LibBase() {}

    struct Data * data ;
    double w, d ;

    virtual double window (double) = 0 ;  //Purely virtual, need to be defined in derived classes
    virtual std::pair<double,double> window_contact_weight (int p, int q, const v1d & loc) {
        double rp=distance(p, loc) ;
        double rq=distance(q, loc) ;
        double wpqs = window_avg (rp, rq) ;
        double wpqf = window_int (rp, rq) ;
        return (make_pair(wpqs, wpqf)) ;
    }
    virtual double window_int(double r1, double r2) {return window_avg(r1, r2) ; }
    virtual double window_avg (double r1, double r2) {return (0.5*(window(r1)+window(r2))) ; }
    virtual double distance (int id, v1d loc) {double res=0 ; for (int i=0 ; i<d ; i++) res+=(loc[i]-data->pos[i][id])*(loc[i]-data->pos[i][id]) ; return sqrt(res) ; } ///< function for mixed particle id / vector informations.
    virtual double cutoff (void) {return 2.5*w ;}
} ;
//---------------------
class LibLucy3D : public LibBase {
public :
    LibLucy3D(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; cst=105./(16*M_PI*w*w*w) ; }
    LibLucy3D() {} ;
    double Lucy (double r) {if (r>=w) return 0 ; else {double f=r/w ; return (cst*(-3*f*f*f*f + 8*f*f*f - 6*f*f +1)) ; }}
    double window(double r) { return (Lucy(r)) ;}
    double cutoff (void) {return 2*w ;} // added by benjy to get rid of extra (hopefully useless) data
    //double window_int(double r1, double r2) {return window_avg(r1, r2) ; }
    //double window_avg (double r1, double r2) {return (0.5*(Lucy(r1)+Lucy(r2))) ; }
  protected:
    double cst ;
};
//--------------------
class LibLucy3DFancyInt : public LibLucy3D {
public :
    LibLucy3DFancyInt (struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; cst=105./(16*M_PI*w*w*w) ;}
    std::pair<double,double> window_contact_weight (int p, int q, const v1d & loc)
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

      first=prev=Lucy(distance(p,loc)) ;
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
      return (make_pair(window_avg(first,cur),res)) ;
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
    double window (double r) {if (r>=w) return 0 ; else return cst ;}
};
//---------------------
class LibRectND : public LibBase {
public:
  LibRectND(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; }
  double window (double r) {if (r>=w) return 0 ; else {double a =1 ; for (int b=0 ; b<d ; b++,a*=w) {} return 1/a ;}}
};
//---------------------
class LibSphere3DIntersect : public LibBase {
public:
    LibSphere3DIntersect(struct Data * D, double ww, double dd) { data=D; w=ww ; d=dd ; cst=1./(4./3.*M_PI*w*w*w) ; }
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
        if (data->N==0) return 2*w ;
        for (int i=0 ; i<data -> N ; i++)
            if (maxr<data->radius[i])
                maxr=data->radius[i] ;
        return maxr+w ;
    }
    double window (double r) {return r ; } // The value calculated by the distance measurement is the one returned, a bit of a hack there ...
    double windowreal (double r) {if (r>=w) return 0 ; else return cst ;}
    virtual std::pair<double,double> window_contact_weight (int p, int q, const v1d & loc)
    {
        double rp=LibBase::distance(p, loc) ;
        double rq=LibBase::distance(q, loc) ;
        double wpqs = window_avg (rp, rq) ;
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
    virtual double window_avg (double r1, double r2) {return (0.5*(windowreal(r1)+windowreal(r2))) ; }
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
    double distance(int id, v1d loc)
    {
        double r=0 ;
        for (int i=0 ; i<d ; i++)
            r+=(loc[i]-data->pos[i][id])*(loc[i]-data->pos[i][id]) ;
        r = sqrt(r) ;
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
    double window (double r) {return r ; } // The value calculated by the distance measurement is the one returned, a bit of a hack there ...
    double windowreal (double r) {if (r>=w) return 0 ; else return cst ;}
    virtual std::pair<double,double> window_contact_weight (int p, int q, const v1d & loc)
    {
        double rp=LibBase::distance(p, loc) ;
        double rq=LibBase::distance(q, loc) ;
        double wpqs = window_avg (rp, rq) ;
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
    virtual double window_avg (double r1, double r2) {return (0.5*(windowreal(r1)+windowreal(r2))) ; }
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
  double window(double r) override {return (Lucy(r)) ;}
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

  double distance (int id, v1d loc) override
  {
    double res=0 ;
    for (int i=0 ; i<d ; i++)
      if ((maskperiodic&(1<<i))==0)
        res+=(loc[i]-data->pos[i][id])*(loc[i]-data->pos[i][id]) ;
    return sqrt(res) ;
  }

  int maskperiodic ;
};
