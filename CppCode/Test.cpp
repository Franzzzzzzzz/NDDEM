/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the root of the source code       *
 * distribution tree, or in https://support.hdfgroup.org/ftp/HDF5/releases.  *
 * If you do not have access to either file, you may request a copy from     *
 * help@hdfgroup.org.                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*
 *  This example writes a dataset to a new HDF5 file.
 */
#include <iostream>
#include <string>
#include <list>
#include <algorithm>
using namespace std ; 
 
int sgn (u_int8_t a) {return a & (128) ? -1:1 ; }


int main ()
{

int8_t a ; 

a=125 ; 
printf("%d %d %d \n", a, sgn(a), abs(a)) ; 
a=-32 ; 
printf("%d %d %d \n", a, sgn(a), abs(a)) ; 
a=128 ; 
printf("%d %d %d \n", a, sgn(a), abs(a)) ; 
a=-127 ; 
printf("%d %d %d \n", a, sgn(a), abs(a)) ; 
a=-128 ; 
printf("%d %d %d \n", a, sgn(a), abs(a)) ; 
a=12 ; 
printf("%d %d %d \n", a, sgn(a), abs(a)) ; 
a=-12 ; 
printf("%d %d %d \n", a, sgn(a), abs(a)) ; 
    
}
