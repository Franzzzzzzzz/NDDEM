#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Jun 15 21:54:18 2017

@author: franz
"""
import numpy as np ;

d=4 ; #spatial dimensions
N=1 ; #number of particles
r=0.1*np.ones((N,1)) ; #particle size
m=np.ones((N,1)) ; #particle mass
dt=1 ; #timestep
T=100 ; #number of timesteps
tdump=10 ; #dump data every these timesteps

K=0; 1 ; #Hooke stiffness
g=np.zeros(d) ; g[2]=0; -0.01 ; #Gravity

Boundaries=np.zeros((d,4)) ; #Boundary type in [:,3]: 0=regular pbc, 1=wall

X=np.zeros((N,d)) ; 
Xold=np.zeros((N,d)) ; 



