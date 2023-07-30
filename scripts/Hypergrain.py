import math




def hypersphere_volume (dimension, radius=0.5):
  d=dimension ; R=radius ; 
  return ((math.pi**(d/2.)) / (math.gamma(d/2.+1)) * (R**d)) ; 

def density (dimension, radius=0.5, mass=1):
  return mass/hypersphere_volume(dimension, radius) ; 

def rcp_volumefraction(dimension):
  if dimension==1 : return 1 ; 
  elif dimension==2: return 0.81 ;
  elif dimension==3: return 0.581 ;
  elif dimension==4: return 0.392 ;
  elif dimension==5: return 0.257 ;
  elif dimension==6: return 0.163 ;
  else : return -1 ;







