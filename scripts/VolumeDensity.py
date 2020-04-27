#!/anaconda/bin/python

import math
import sys

if len(sys.argv)<2 :
    print("Error: expect dimension")
elif len(sys.argv)<3 :
    d=int(sys.argv[1]) ; 
    R=0.5 ; 
    print("Assuming R=0.5")  
#if len(sys.argv)<4 :
    print("Assuming M=1")
    M=1;
else :
    print("Not implemented")

print(math.gamma(d/2+1))
Vol=(math.pi**(d/2.)) / (math.gamma(d/2.+1)) * (R**d) ; 
Density = M/Vol

print("Volume " + str(Vol) + "\tDensity " + str(Density))
