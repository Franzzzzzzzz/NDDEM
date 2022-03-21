from evtk.hl import pointsToVTK
import numpy as np
import matplotlib.pyplot as plt 
import glob

filelist = glob.glob('Ghost-*.txt')

for i in filelist :
    
    X=np.atleast_2d(np.loadtxt(i)) ; 
    if (X.size==0): 
        r=np.ascontiguousarray((0))
        x=np.ascontiguousarray((0))
        y=np.ascontiguousarray((0))
        z=np.ascontiguousarray((0))
        vx=np.ascontiguousarray((0))
        vy=np.ascontiguousarray((0))
    else:
        r= np.ascontiguousarray(X[:,4]) ;
        x= np.ascontiguousarray(X[:,0]) ;
        z = x*0 ; 
        y= np.ascontiguousarray(X[:,1]) ;
        vx= np.ascontiguousarray(X[:,2]) ;
        vy= np.ascontiguousarray(X[:,3]) ;
        typ= np.ascontiguousarray(X[:,5]) ;
        ids= np.ascontiguousarray(X[:,6]) ;
    pointsToVTK(i[0:-4], x, y, z, data = {"vx":vx,"vy":vy, 'radius': r, 'type': typ, 'ids': ids})
