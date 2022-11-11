namespace vtkwriter {
  
FILE * open (char path[], int d) {
  FILE *out ; 
  static bool warn ; 
  out=fopen(path, "w") ; if (out==NULL) {printf("Cannot open out file\n") ; return nullptr;}
  if (d>3 && warn==false) {
   printf("WARN: writevtk might misbehave with dimension higher than 3. The 3d projection is always centered in all other dimensions\n") ;
   warn=true ;
  }
  fprintf(out, "# vtk DataFile Version 2.0\nMost Useless DEM (tm) output file\nASCII\nDATASET POLYDATA\n") ;
  return out ; 
}
//------------------------------------------------------------------------------  
int write_points (FILE * out, cv2d & X, int d)  
{
 fprintf(out, "POINTS %ld float\n", X.size()) ;
 for (uint i=0 ; i<X.size() ; i++) fprintf(out, "%g %g %g\n", X[i][0], X[i][1], d<3?0:X[i][2]) ;
 fprintf(out, "VERTICES %ld %ld\n", X.size(), 2*X.size()) ;
 for (uint i=0 ; i<X.size() ; i++) fprintf(out, "1 %d\n", i) ;
 return 0;
}
//------------------------------------------------------------------------------
int write_contactlines (FILE *out, cv2d & ids)
{
  
fprintf(out, "LINES %ld %ld\n", ids.size(), ids.size()*3) ; 
for (uint i=0 ; i<ids.size() ; i++) fprintf(out, "2 %g %g\n", ids[i][0], ids[i][1]) ; 
return 0 ; 
}

//------------------------------------------------------------------------------
int start_pointdata (FILE *out, cv2d & X)
{
 fprintf(out, "\nPOINT_DATA %ld", X.size()) ;
 return 0;
}
//------------------------------------------------------------------------------
int start_celldata (FILE *out, int N, int Ncf)
{
  fprintf(out, "\nCELL_DATA %d\n", N+Ncf) ; 
  return 0 ; 
}
//------------------------------------------------------------------------------  
int write_dimension_data (FILE *out, cv2d &X, cv1d &r, int d, vector < vector <double> > Boundaries)
{
 vector <float> projectioncenter  ;
 for (int i=3 ; i<d ; i++) projectioncenter.push_back((Boundaries[i][1]+Boundaries[i][0])/2) ;

 for (uint j=3 ; j<X[0].size() ; j++)
 {
   fprintf(out, "\nSCALARS Dimension%d float 1 \nLOOKUP_TABLE default \n", j) ;
   for (uint i=0 ; i<X.size() ; i++)
       fprintf(out, "%g ", X[i][j]) ;
 }

 fprintf(out, "\n\nSCALARS RadiusProjected float 1 \nLOOKUP_TABLE default\n");
 for (uint i=0 ; i<X.size() ; i++)
 {
   float value = r[i]*r[i] ;
   for (int j=3 ; j<d ; j++) value-=(X[i][j]-projectioncenter[j-3])*(X[i][j]-projectioncenter[j-3]) ;
   if (value<0) fprintf(out, "%g ", 0.0) ;
   else fprintf(out, "%g ", sqrt(value)) ;
 }
 return 0;
}
//------------------------------------------------------------------------------  
int write_data (FILE *out, TensorInfos v, int d)
{
 switch (v.order) {
   case TensorType::SCALAR:  fprintf(out, "\nSCALARS %s double 1 \nLOOKUP_TABLE default \n", v.name.c_str()) ;//scalar
            for (uint i=0 ; i<(*v.data)[0].size() ; i++)
              fprintf(out, "%g ", (*v.data)[0][i]) ;
            break ;
   case TensorType::VECTOR:  fprintf(out, "\nVECTORS %s double \n", v.name.c_str()) ;//vector
            for (auto i : (*v.data))
              fprintf(out, "%g %g %g\n", i[0], i[1], d<3?0:i[2]) ;
            break ;
   case TensorType::TENSOR:  fprintf(out, "\nTENSORS %s double \n", v.name.c_str()) ;//tensor
            for (auto i : (*v.data))
              fprintf(out, "%g %g %g %g %g %g %g %g %g\n", i[0], i[1], i[2], i[d], i[d+1], i[d+2], i[2*d], i[2*d+1], i[2*d+2]) ;
            break ;
   case TensorType::SYMTENSOR:  fprintf(out, "\nTENSORS %ssym double \n", v.name.c_str()) ;//tensor
            for (auto i : (*v.data))
              fprintf(out, "%g %g %g %g %g %g %g %g %g\n", i[0], i[1], i[2], i[1], i[d], i[d+1], i[2], i[d+1], i[2*d-1]) ;
            break ;
   case TensorType::SKEWTENSOR:  fprintf(out, "\nTENSORS %sskew double \n", v.name.c_str()) ;//tensor
             for (v1d i : (*v.data))
               fprintf(out, "%g %g %g %g %g %g %g %g %g\n", 0.0, i[0], i[1], -i[0], 0.0, i[d-1], -i[1], -i[d-1], 0.0) ;
            break ;
   default: break ; /*fprintf(out, "\nPOINT_DATA %ld\nSCALARS %s double 1 \nLOOKUP_TABLE default \n",(*data.data).size(), data.name.c_str()) ;//scalar norm
              for (uint i=0 ; i<(*data.data).size() ; i++)
                 fprintf(out, "%g ", Tools<d>::norm((*data.data)[i])) ;*/
 }
 return 0 ;
}
//------------------------------------------------------------------------------  
int write_celldata (FILE *out, std::string name, cv2d & v, TensorType order, int idx, int N, int d)
{
 switch (order) {
   case TensorType::VECTOR:  fprintf(out, "\nVECTORS %s double \n", name.c_str()) ;//vector
            for (int i=0 ; i<N ; i++) fprintf(out, "0 0 0\n") ; 
            for (auto i : v)
              fprintf(out, "%g %g %g\n", i[idx], i[idx+1], d<3?0:i[idx+2]) ;
            break ;
   case TensorType::SCALARMASK:  fprintf(out, "\nSCALARS %s long 1 \nLOOKUP_TABLE default \n", name.c_str()) ;//tensor
            for (int i=0 ; i<N ; i++) {fprintf(out, "0 ") ; } fprintf(out, "\n") ;
            for (auto i : v)
              fprintf(out, "%ld \n", static_cast<long int>(i[idx])) ;
            break ;
   default: break ; /*fprintf(out, "\nPOINT_DATA %ld\nSCALARS %s double 1 \nLOOKUP_TABLE default \n",(*data.data).size(), data.name.c_str()) ;//scalar norm
              for (uint i=0 ; i<(*data.data).size() ; i++)
                 fprintf(out, "%g ", Tools<d>::norm((*data.data)[i])) ;*/
 }
 return 0 ;
}
//------------------------------------------------------------------------------
void close(FILE *out ) { fclose(out) ;}
} ; 











