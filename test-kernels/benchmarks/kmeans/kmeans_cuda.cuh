#ifndef KMEANS_CUDA_CUH
#define KMEANS_CUDA_CUH

void allocateMemory(int npoints, int nfeatures, int nclusters, float **features);
void deallocateMemory();
int kmeansCuda(float  **feature,				/* in: [npoints][nfeatures] */
           int      nfeatures,				/* number of attributes for each point */
           int      npoints,				/* number of data points */
           int      nclusters,				/* number of clusters */
           int     *membership,				/* which cluster the point belongs to */
		   float  **clusters,				/* coordinates of cluster centers */
		   int     *new_centers_len,		/* number of elements in each cluster */
           float  **new_centers				/* sum of elements in each cluster */
	   );

#endif
