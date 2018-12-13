    
#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Created on Sun Jun 11 19:00:47 2017
"""

import numpy as np ;
from numpy.linalg import norm ;
import sys ; 
from scipy.special import gamma ;
import matplotlib.pyplot as plt ;
from matplotlib.widgets import Slider, CheckButtons; 
from evtk.hl import pointsToVTK ;
import Contacts as C ; 

# Parameters 
d=7 ; #spatial dimensions
N=5 ; #number of particles
r=0.15*np.ones((N,1)) ; #particle size
#rho=1 #density (unit [M.L^(-d)])
#m=np.zeros((N,1)) ; #particle mass, setup during the particle initialization
m=np.ones((N,1))*1e-2 ; # Set the masses directly
I=np.ones((N,1)) ; #Momentum of inertia (particles are sphere -> moment of inertia tensor is i*Id(d) in principle is a 4th order tensor)
dt=1 ; #timestep
T=5000 ; #number of timesteps
tdump=1 ; #dump data every these timesteps
tinfo=T ;
dumpkind=0 ; #How to dump: 0=nothing, 1=csv, 2=vtk

Kn=np.float64(0.0001) ; #Hooke stiffness
Kt=np.float64(0.00001) ; #Tangential stiffness
Gamman = np.float64(0.0001) ; #Normal damping
Gammat = 0 ; #Tengential damping
μ = np.float64(0.5) ; 
g=np.zeros(d) ; g[2]=0#.00001 ; #Gravity

Boundaries=np.zeros((d,4)) ; #Boundary type in [:,3]: 0=regular pbc, 1=wall

# Array initialisations
X=np.zeros((N,d)) ; #Position
V=np.zeros((N,d)) ; #Velocity
A=np.zeros((N,d*d)) ; # Individual orientation matrix, initialize to identity
Ω=np.zeros((N,(d-1)*d//2)) ; #Rotational velocity matrix (antisymetrical)
F=np.zeros((N,d)) ; 
Fold=np.zeros((N,d)) ; 
ℳ=np.zeros((N,d*(d-1)//2)) ;
ℳold=np.zeros((N,d*(d-1)//2)) ;
Contacts=C.Contacts(d,N,dt,Kn,Kt,Gamman,Gammat,μ) ; #Initialize the Contact class object
Ghosts=list() ;
Nghosts=0 ;  

# General ND functions
def unitvec (n):
    ZeroVec=np.zeros(d) ; 
    ZeroVec[n]=1 ;
    return(ZeroVec) ; 
def volume (r,d2=d) : #d dimensional volume
    return (np.pi**(d2/2))/(gamma(d2/2+1))*(r**d2) ;
def area (r,d2=d) : #d-1 dimensional area
    return ((2*np.pi**(d2/2))/(gamma(d2/2))*(r**(d2-1)))
def project_radius (x,r,dims,otherdims):
    rproj=np.zeros(N) ;
    otherdims[[dims]]=0 ; 
    for i in range(0,N):
        tmp=np.copy(x[i,:]) ; tmp[[dims]]=0 ; 
        dst=norm(tmp-otherdims) ; 
        if (dst>r[i]): rproj[i]=0 ;
        else : rproj[i]=np.sqrt(r[i]*r[i]-dst*dst) ; 
    return (rproj) ;     
def info ():
    print('Max velocity under gravity (number of radius displaced by ts): '+ str(np.sqrt(2*norm(g[:])*np.max(Boundaries[:,2]))/r[0])) ; 
    print('Needed interpenetration to balance gravity (fraction or radius): ' + str(m[0]*norm(g)/Kn/r[0])) ;


# Distance measurements
def dst (x1,x2):
    return (norm(x1-x2)) ;
def overlap (xi,xj,r):
    return (2*r-dst(xi,xj)) ;
def overlap_thresh (xi,xj,ri,rj): #defined as >=0 when particles overlap
    return (np.max((0,ri+rj-dst(xi,xj)))) ;
def overlap_thresh_opt (l,ri,rj):
    return (np.max((0,ri+rj-l))) ; 
def overlap_opt (l,ri,rj):
    return (ri+rj-l) ; 

# Initialisation function
def set_boundaries(): 
    for i in range(0,d):
        Boundaries[i,0]= 0 ; 
        Boundaries[i,1]= 1 ;
        Boundaries[i,2]=Boundaries[i,1]-Boundaries[i,0] ; #Precomputed to increase speed
    Boundaries[5,3]=1 ; 
    np.savetxt('Boundaries.csv', Boundaries , delimiter=',', fmt='%.6g', header='Low,High,Size,Type', comments='')
def init_particles(): #random initial position in the simulation box, no velocity (Xold=X)
    for i in range(0,N) :
        X[i,:]=np.random.rand(d)*(Boundaries[:,2]-Boundaries[:,3]*2*r[i])+(Boundaries[:,0]+Boundaries[:,3]*r[i]) ; #avoid too much overlap with walls
        #m[i]=volume(r[i])*rho ; 
        A[i,:]=np.reshape(np.identity(d),d*d) ; #initial orientation matrix
    X[0,5]=0.14 ;
    #V[0,:]=np.zeros((1,d))+0.0001*unitvec(0) ; 
    #X[1,:]=np.ones((1,d))*0.7 ; X[1,5]=0.2 ; X[1,4]=0.71 ; 
    #X[1,:]=np.array([0.21,0.21,0.9,0.2]) ;
    
#==============================================================================
#temporary variables, initialised just once
Atmp=np.zeros(d*d) ;

#Beginning of the actual computation (probably need some def __main__ here)
set_boundaries() ; 
init_particles() ; #Xold=np.copy(X) ; 

Res=np.zeros((T,N,d)) ; ResF=np.zeros((T,N,d)) ; ResM=np.zeros((T,N,d)) ;
info() ; 
for t in range(0,T):
    if t%tinfo==0 : print (t) ;
    
    if t%500==0:
        V[0,:]=(np.random.rand(d)-0.5)*0.002
        V[0,5]=0 ; 
    
    #----- Velocity Verlet step 1 : compute the new positions
    for i in range(0,N):
        X[i,:]=X[i,:]+V[i,:]*dt+Fold[i,:]/m[i]*dt*dt/2.
        for j in range (0,d*d):
            #A[i,j]=A[i,j]+ np.sum(Ω[i,[MIndexAS[j//d,:]]]*MSigns[j//d,:]*A[i,j%d::d])*dt+(1/I[i])*ℳold[i,MIndexAS[j//d,j%d]]*dt*dt/2 ; 
            Atmp[j]=A[i,j]+ np.sum((Ω[i,Contacts.MIndexAS[j//d,:]]*Contacts.MSigns[j//d,:]*dt +
                ℳold[i,Contacts.MIndexAS[j//d,:]]*Contacts.MSigns[j//d,:]/I[i]/2.*dt*dt
                )*A[i,j%d::d]) ; 
        A[i,:]=Atmp ; 
        
        
    # ---------- Velocity Verlet step 2 : compute the forces and torques
    # Force / Torque computation (incredibly slow)
    F[:]=0 ; ℳ[:]=0 ; 
    for i in range(0,N):
        F[i,:]+=m[i]*g ; 
        
        #-----Particle - particle contacts
        for j in range(i+1,N):
            #Contact properties:
            (Fn,Ft,Torquei,Torquej)=Contacts.particle_particle(X[i,:],V[i,:],Ω[i,:],r[i], i,
                                                               X[j,:],V[j,:],Ω[j,:],r[j], j) ; 
            F[i,:]=F[i,:]+Fn+Ft ; 
            F[j,:]=F[j,:]-Fn-Ft ; 
            ℳ[i,:]=ℳ[i,:]+Torquei ; 
            ℳ[j,:]=ℳ[j,:]+Torquej ; 
            
        #----- Particle - ghosts contacts
        for j in range(0, Nghosts):
            (ghost_idx, ghost_d, ghost_delta)=Ghosts[j] ; 
            (Fn,Ft,Torquei,Torquej)=Contacts.particle_ghost(X[i,:],V[i,:],Ω[i,:],r[i], i,
                      X[ghost_idx,:]+unitvec(ghost_d)*ghost_delta,V[ghost_idx,:],Ω[ghost_idx,:],
                      r[ghost_idx], N+ghost_idx) ;             
            F[i,:]+=Fn+Ft ; 
            ℳ[i,:]+=Torquei ; 
        
#        #----- Particle - Walls contacts
        for j in range(0,d): 
            if Boundaries[j,3]!=1: continue;
            for k,l in ((0,-1), (1,1)):
                (Fn,Ft,Torquei)=Contacts.particle_wall(X[i,:],V[i,:],Ω[i,:],r[i],i,j,k,l,Boundaries[j,k]) ; 
                F[i,:]+= Fn+Ft ; 
                ℳ[i,:]+= Torquei ;   

    # TEMPORARY 
    Res[t,:,:]=X ; 
    ResF[t,:,:]=F ; 
    #ResM[t,:,:]=ℳ ; 
    F[0,:]=0 ; 
    ℳ[0,:]=0 ; 
    
    #---------- Velocity Verlet step 3 : compute the new velocities
    del Ghosts[:] ; 
    for i in range(0,N):
        V[i,:]=V[i,:]+(F[i,:]+   Fold[i,:])*dt/2./m[i] ; 
        Ω[i,:]=Ω[i,:]+(ℳ[i,:]+ℳold[i,:])*dt/2./I[i] ; 
        Fold[i,:]=F[i,:] ; 
        ℳold[i,:]=ℳ[i,:] ;     
        for j in range(0,d): # Perform PBC
            if (Boundaries[j,3]!=0): continue ; # not a PBC      
            if (X[i,j]<Boundaries[j,0]): X[i,j]+=Boundaries[j,2] ; 
            elif (X[i,j]>Boundaries[j,1]): X[i,j]-=Boundaries[j,2] ; 
        for j in range(0,d):   #Perform ghosts: 2 separated loop, to be sure we are in the simu box before constructing the ghosts.
            if (Boundaries[j,3]!=0): continue ; # not a PBC   
            if (X[i,j]<Boundaries[j,0]+2*r[i]): Ghosts.append((i, j, Boundaries[j,2])) ; 
            elif (X[i,j]>Boundaries[j,1]-2*r[i]): Ghosts.append((i, j, -Boundaries[j,2])) ; 
    Nghosts=len(Ghosts) ;     
    
    #Contact history cleaning and preparation for the next iteration
    Contacts.clean_history() ;   
        
    # Output something at some point I guess
    if (t % tdump ==0) : 
        if dumpkind==1: 
            np.savetxt('dump-'+str(t)+'.csv', np.hstack((X,r)) , delimiter=',', fmt='%.6g', header='X,Y,Z,W,R', comments='')
        elif dumpkind==2:
            dat=dict() ; 
            for i in range(3,d):
                dat["d"+str(i)]=np.ascontiguousarray(np.squeeze(X[:,i]))
            dat["radius"]=np.squeeze(r) ; 
            pointsToVTK("dump-"+str(t), np.ascontiguousarray(np.squeeze(X[:,0])), np.ascontiguousarray(np.squeeze(X[:,1])), np.ascontiguousarray(np.squeeze(X[:,2])), data = dat) ; 
            #pointsToVTK("dump-"+str(t), np.ascontiguousarray(np.squeeze(X[:,0])), np.ascontiguousarray(np.squeeze(X[:,1])), np.ascontiguousarray(np.squeeze(X[:,2])), data = {"d3" : np.ascontiguousarray(np.squeeze(X[:,3])),"Radius" : np.squeeze(r)}) ; 
        else: 
            continue ; 
 
plt.close('all') ; 
plt.plot(Res[:,0,:])
#plt.plot(Res[:,0,1])
plt.figure()
plt.plot(np.sqrt(ResF[:,0,0]**2+ResF[:,0,1]**2+ResF[:,0,2]**2+ResF[:,0,3]**2+ResF[:,0,4]**2+ResF[:,0,6]**2))
#plt.plot(ResF[:,0,0])
#plt.plot(ResF[:,0,1]) 
#plt.plot(ResM[:,0,2])
sys.exit() ; 






# ======= Final display ========
fig, ax = plt.subplots(); 
plt.subplots_adjust(left=0.1, bottom=0.25)
plt.axis([Boundaries[0,0], Boundaries[0,1], Boundaries[1,0], Boundaries[1,1]])
dims=(0,1) ;

def projected_radius (x,dims,normalvec):
    rproj=np.zeros(N) ; 
    for i in range(0,N):
        tmp=np.copy(x[i,:]) ; 
        tmp[dims[0]]=0 ; tmp[dims[1]]=0 ; 
        dst=np.linalg.norm(tmp-normalvec) ; 
        if (dst>r): rproj[i]=0 ;
        else : rproj[i]=np.sqrt(r*r-dst*dst) ; 
    return (rproj) ;     


rproj=projected_radius(X,(0,1), np.zeros(d)) ;
circles=[plt.Circle((X[i,0], X[i,1]), rproj[i], color='b') for i in range(0,N)] ; 
for i in range(0,N) : ax.add_artist(circles[i]); 
axs=[plt.axes([0.25, 0.03+0.03*i, 0.65, 0.03]) for i in range(0,d)] ;
slider = [Slider(axs[i], 'd'+str(i), Boundaries[i,0], Boundaries[i,1], valinit=0) for i in range(0,d)] ; 

axc = plt.axes([0.01, 0.03, 0.2, 0.03*d])
cb=['d'+str(d-i-1) for i in range(0,d)] ; 
cbc=[0*i for i in range(d-1,-1,-1)] ; cbc[-1]=1 ; cbc[-2]=1 ; 
checks = CheckButtons(axc, cb, cbc)

def update(val):
    normalvec=np.zeros(d) ; rproj=np.zeros(N) ; 
    for i in range(0,d) : normalvec[i]=slider[i].val ; 
    normalvec[dims[0]]=0 ; normalvec[dims[1]]=0 ; 
    rproj=projected_radius(X,dims,normalvec) ;  
    for i in range(0,N):
        circles[i].radius=rproj[i] ; 
    fig.canvas.draw_idle() ;
def updatechk ():
    pass;
for i in range (0,d) : slider[i].on_changed(update) ;
checks.on_clicked(updatechk) ; 
plt.show()

    
        
        
        
        
        
        
        
        
        
        
        
        
        
        
