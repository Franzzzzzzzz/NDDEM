/** \addtogroup DEM Discrete Element Simulations
 * This module handles the Discrete Element Simulations.
 *  @{ */

#include <vector>
#include "Typedefs.h"

// All vector operators
v1d operator* (v1d a, double b)  {for (uint i=0 ; i<a.size() ; i++) a[i]*=b    ; return a ; }
v1f operator* (v1f a, float b)  {for (uint i=0 ; i<a.size() ; i++) a[i]*=b    ; return a ; }
v1d operator* (v1d a, cv1d &b)    {for (uint i=0 ; i<a.size() ; i++) a[i]*=b[i] ; return a ; }
v1f operator* (v1f a, cv1f &b)    {for (uint i=0 ; i<a.size() ; i++) a[i]*=b[i] ; return a ; }
v1d operator+ (v1d a, double b)  {for (uint i=0 ; i<a.size() ; i++) a[i]+=b    ; return a ; }
v1d operator+ (v1d a, cv1d &b)    {for (uint i=0 ; i<a.size() ; i++) a[i]+=b[i] ; return a ; }
v1f operator+ (v1f a, cv1f &b)    {for (uint i=0 ; i<a.size() ; i++) a[i]+=b[i] ; return a ; }

v1d operator- (v1d a, double &b)  {for (uint i=0 ; i<a.size() ; i++) a[i]-=b    ; return a ; }
v1d operator- (v1d a, cv1d &b)    {for (uint i=0 ; i<a.size() ; i++) a[i]-=b[i] ; return a ; }
v1d operator- (v1d a, const double * b)   {for (uint i=0 ; i<a.size() ; i++) a[i]-=b[i] ; return a ; }
v1d operator- (const double * a, v1d b)   {for (uint i=0 ; i<b.size() ; i++) b[i]=a[i]-b[i] ; return b ; }
v1d operator- (v1d a)            {for (uint i=0 ; i<a.size() ; i++) a[i]=-a[i] ; return a ; }
v1f operator- (v1f a, cv1f &b)    {for (uint i=0 ; i<a.size() ; i++) a[i]-=b[i] ; return a ; }


v1d operator/ (v1d a, double b)  {for (uint i=0 ; i<a.size() ; i++) a[i]/=b    ; return a ; }
v1d & operator-= (v1d & a, cv1d &b)  {for (uint i=0 ; i<a.size() ; i++) a[i]-=b[i] ; return a; }
v1d & operator*= (v1d & a, double b)  {for (uint i=0 ; i<a.size() ; i++) a[i]*=b ; return a; }
v1f & operator*= (v1f & a, double b)  {for (uint i=0 ; i<a.size() ; i++) a[i]*=b ; return a; }
v1d & operator+= (v1d & a, cv1d &b)  {for (uint i=0 ; i<a.size() ; i++) a[i]+=b[i] ; return a; }
v1f & operator+= (v1f & a, cv1f &b)  {for (uint i=0 ; i<a.size() ; i++) a[i]+=b[i] ; return a; }
v1f & operator/= (v1f & a, cv1f &b)  {for (uint i=0 ; i<a.size() ; i++) a[i]/=b[i] ; return a; }
v1d & operator/= (v1d & a, double b)  {for (uint i=0 ; i<a.size() ; i++) a[i]/=b ; return a; }
v1f & operator/= (v1f & a, double b)  {for (uint i=0 ; i<a.size() ; i++) a[i]/=b ; return a; }

/** @} */
