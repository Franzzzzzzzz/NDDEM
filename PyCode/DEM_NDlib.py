#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Thu Jun 15 22:23:35 2017

@author: franz
"""

import numpy as np ; 
from DEM_Parameters import * ; 
from scipy.special import gamma

def unitvec (n):
    ZeroVec=np.zeros(d) ; 
    ZeroVec[n]=1 ;
    return(ZeroVec) ; 

def volume (r,d2=d) : #d dimensional volume
    return (np.pi**(d2/2))/(gamma(d2/2+1))*(r**d2) ;

def area (r,d2=d) : #d-1 dimensional area
    return ((2*np.pi**(d2/2))/(gamma(d2/2))*(r**(d2-1)))