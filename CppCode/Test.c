#include "hdf5.h"
#define FILE "dset.h5"

int main() {

   hid_t       file_id, dataset_id, dataspace_id;  /* identifiers */
   hsize_t     dims[4];
   herr_t      status;

   /* Create a new file using default properties. */
   file_id = H5Fcreate(FILE, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

   /* Create the data space for the dataset. */
   dims[0] = 4; 
   dims[1] = 6; 
   dims[2] = 9; 
   dims[3] = 2; 
   dataspace_id = H5Screate_simple(4, dims, NULL);

   /* Create the dataset. */
   dataset_id = H5Dcreate2(file_id, "/dset", H5T_STD_I32BE, dataspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

   
   
   int         i, j, dset_data[4][6][9][2];

   /* Initialize the dataset. */
   //for (i = 0; i < 4; i++)
   //   for (j = 0; j < 6; j++)
   //      dset_data[i][j] = i * 6 + j + 1;
   
   status = H5Dwrite(dataset_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, 
                     dset_data);
   
   /* End access to the dataset and release resources used by it. */
   status = H5Dclose(dataset_id);

   /* Terminate access to the data space. */ 
   status = H5Sclose(dataspace_id);

   /* Close the file. */
   status = H5Fclose(file_id);
}
