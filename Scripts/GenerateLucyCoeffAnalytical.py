#!python

import math
import sys

if len(sys.argv)<2 :
    print("Error: expect dimension")

d=int(sys.argv[1]) ; 

a=-3 ; b=+8 ; c=-6 ;
print ("Lucy function: "+str(a)+'x^4+' + str(b) + 'x^3 - +' + str(c) + 'x^2 +1') ;


Vol=(math.pi**(d/2.)) / (math.gamma(d/2.+1)) ; 


coeff= Vol * d * (a / (d+4) + b/(d+3) +c /(d+2) + 1/d) ; 

print("Lucy coefficient: " + '{:.17f}'.format(coeff)) ; 
