#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Mar 20 19:11:41 2018

@author: franz
"""

#test of the verlet intergration for the orientation in NDDEM

import numpy as np ; 
import matplotlib.pyplot as plt ; 

d=2 ; 
A=np.eye(d) ; 
Omega=np.zeros((d,d)) ; 
Omega[0,1]=-1 ; Omega[1,0]=1 ;
Torque=1 ; #=np.zeros((d)) ; 

N=10 ; dt=0.1 ; 
Ares=np.zeros((N,d,d)) ; 
Ares[0,:,:]=A ; 
for i in range(1,N):
    Ares[i,:,:]=(Ares[i-1,:,:] + 
         np.matmul(Omega, Ares[i-1,:,:])*dt + 
         T*Ares[i-1,:,:] *dt *dt +
         np.matmul(np.matmul(Omega, Omega), Ares[i-1,:,:])/2*dt*dt) ; 

plt.clf() ; 
plt.plot (Ares[:,0,0], Ares[:,1,0]) ; 
plt.plot (Ares[:,0,1], Ares[:,1,1]) ; 

