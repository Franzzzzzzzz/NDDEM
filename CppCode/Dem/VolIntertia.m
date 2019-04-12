clear all

maxD=30 ; 

syms R z rho pi ;

g=pi*R^2 ;
V(2)=g ; 

f=pi * rho *R^4 /2 ; %Inertia of a disk in in 2D
I(2)=f ; 

for i=3:maxD 
    f=subs(f, R, sqrt(R^2-z^2)) ; 
    g=subs(g, R, sqrt(R^2-z^2)) ; 
    I(i)=int(f, [-R, R]) ; 
    V(i)=int(g, [-R, R]) ; 
    f=I(i) ; 
    g=V(i) ; 
end ; 


for i=3:maxD
    disp(I(i)) ; 
end ; 
