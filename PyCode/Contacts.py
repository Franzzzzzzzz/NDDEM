import numpy as np ; 
from numpy.linalg import norm ;


class Contacts():
    def __init__ (self,d,N,dt,Kn,Kt,Gamman,Gammat,μ):
        self.d=d ; self.N=N ; self.dt=dt ;  
        self.Kn=Kn ; self.Kt=Kt ; 
        self.μ=μ ; 
        self.Gamman=Gamman ; self.Gammat=Gammat ; 
        #Dummy matrices for proper indexing & coefficient for matrix multiplications
        self.MSigns=np.reshape(np.array([(i<j)*1 + (i>j)*(-1) for i in range(0,d) for j in range(0,d)]),(d,d)) ;
        self.MIndexAS=np.zeros((d,d), dtype=int) ; 
        n=0 ; 
        self.MASIndex=[] ; 
        for i in range(0,d) :
            for j in range(i+1,d) :
                self.MIndexAS[i,j]=n ; 
                self.MASIndex.append((i,j)) ; 
                n+=1 ; 
        self.MIndexAS=self.MIndexAS+np.transpose(self.MIndexAS) ;
        self.Torquei=np.zeros(d*(d-1)//2) ; self.Torquej=np.zeros(d*(d-1)//2) ; 
        self.vrel=np.zeros(d) ;  
        self.History=dict() ; 

    def clean_history(self) :
        tmpcontacts=list(self.History.items()) ; 
        for a,(b,c) in tmpcontacts: 
            if b==False: del self.History[a] ; 
            else : self.History[a]=(False,c);   

    def unitvec (self,n):
        ZeroVec=np.zeros(self.d) ; 
        ZeroVec[n]=1 ;
        return(ZeroVec) ; 

    def forcen(self,cn,ovlp,vn): #modify this to change the contact force call
        return (self.force_kngn(cn, ovlp, vn)) ; 
    def force_kn(self,cn,ovlp): # Hooke spring, no damping, no friction. >=0
        return (ovlp*self.Kn*cn) ; 
    def force_kngn (self,cn,ovlp,vn): # Hooke spring, Normal damping, no friction. >=0
        return ((ovlp*self.Kn*cn - self.Gamman*vn)) ;
    
#---------------------- particle particle contact ----------------------------
    def particle_particle (self, Xi, Vi, Ωi, ri, i,
                                 Xj, Vj, Ωj, rj, j):
        #Contact properties:
        contactlength=norm(Xi-Xj) ;
        ovlp=ri+rj-contactlength ; 
        if (ovlp<=0): return (0,0,0,0) ; 
        cn=(Xi-Xj)/contactlength ;
        
        #Relative velocity at contact
        rri=+(ri-ovlp/2.)*cn ; 
        rrj=-(rj-ovlp/2.)*cn ; 
        for k in range(0,self.d): 
            self.vrel[k]=Vi[k]-Vj[k] - np.sum(Ωi[self.MIndexAS[k//self.d,:]]*self.MSigns[k//self.d,:]*rri) \
                                     + np.sum(Ωj[self.MIndexAS[k//self.d,:]]*self.MSigns[k//self.d,:]*rrj) ; 
        vn=np.dot(self.vrel,cn)*cn ;
        vt=self.vrel-vn*cn ; 
        
        #Fn=2*(m[i]*m[j])/(m[i]+m[j])*force(X[i,:],X[j,:],V[i,:],V[j,:],r[i],r[j]) ; not sure why I got the reduced mass there ...
        #Normal force
        Fn=self.forcen(cn, ovlp, vn) ; 
    
        #Tangential force computation: retrieve contact or create new contact
        _,tspr=self.History.setdefault((i,j),(True,np.zeros(self.d))) ; 
        tspr=tspr-np.dot(tspr,cn)*cn ; #WARNING: might need an additional scaling so that |tsprnew|=|tspr|
        Ft=-self.Kt*tspr-self.Gammat*vt ; 
        Coulomb=self.μ*norm(Fn) ; 
        tvec=Ft/norm(Ft) ;
        Ftc=Coulomb*tvec ;
        tspr=tspr+vt*self.dt ;
        tsprn=norm(tspr) ; 
        tsprc=-(Ftc+self.Gammat*vt)/self.Kt ;
        tsprcn=norm(tsprc) ; 
        if norm(Ft) >= Coulomb : Ft=Ftc ;
        if tsprn>tsprcn : tspr=tspr/tsprn*tsprnc ; 
#        if norm(Ft) < self.μ*norm(Fn) : 
#            tspr=tspr+vt*self.dt ; 
#        else :
#            tvec=Ft/norm(Ft) ;
#            Ft=self.μ*norm(Fn)*tvec ;
#            tspr=-(Ft+self.Gammat*vt)/self.Kt ;
        for k,(a,b) in enumerate(self.MASIndex):
            self.Torquei[k]=rri[a]*Ft[b]-rri[b]*Ft[a] ;
            self.Torquej[k]=rrj[a]*Ft[b]-rrj[b]*Ft[a] ;
    
        #Update contact history
        self.History[(i,j)]=(True, tspr) ; 
        
        return (Fn, Ft, self.Torquei, self.Torquej)

#-------------------- Particle ghost contact ----------------------------
    def particle_ghost (self, Xi, Vi, Ωi, ri, i,
                              Xj, Vj, Ωj, rj, j):
        return self.particle_particle(Xi, Vi, Ωi, ri, i,
                                      Xj, Vj, Ωj, rj, j) ; 
        #Just an alias for particle particle, the ghosts are regular particles, just not integrated


#--------------------Particle wall contact---------------------------------
    def particle_wall(self, Xi, Vi, Ωi, ri, i,
                            j,k,l,BoundaryLoc):
        contactlength=np.abs(Xi[j]-BoundaryLoc) ; 
        ovlp=ri-contactlength ; 
        if (ovlp<=0): return(0,0,0) ; 
        cn=-self.unitvec(j)*l ; 
        
        #Relative velocity at contact
        rri=+(ri-ovlp/2.)*cn ; 
        for kk in range(0,self.d): 
            self.vrel[kk]=Vi[kk] - np.sum(Ωi[self.MIndexAS[kk//self.d,:]]*self.MSigns[kk//self.d,:]*rri ); 
        vn=np.dot(self.vrel,cn)*cn ;
        vt=self.vrel-vn*cn ; 
        
        Fn=self.forcen(cn, ovlp, vn) ; 
    
        #Tangential force computation: retrieve contact or create new contact
        _,tspr=self.History.setdefault((i,-(2*j+k+1)),(True,np.zeros(self.d))) ; 
        tspr=tspr-np.dot(tspr,cn)*cn ; #WARNING: might need an additional scaling so that |tsprnew|=|tspr|
        Ft=-self.Kt*tspr-self.Gammat*vt ; 
        Coulomb=self.μ*norm(Fn) ; 
        tvec=Ft/norm(Ft) ;
        Ftc=Coulomb*tvec ;
        tspr=tspr+vt*self.dt ;
        tsprn=norm(tspr) ; 
        tsprc=-(Ftc+self.Gammat*vt)/self.Kt ;
        tsprcn=norm(tsprc) ; 
        if norm(Ft) >= Coulomb : Ft=Ftc ;
        if tsprn>tsprcn : tspr=tspr/tsprn*tsprcn ; 
        
        for ii,(a,b) in enumerate(self.MASIndex):
            self.Torquei[ii]=rri[a]*Ft[b]-rri[b]*Ft[a] ;
    
        #Update contact history
        self.History[(i,-(2*j+k+1))]=(True, tspr) ; 
        
        return (Fn,Ft,self.Torquei);                     
                       




