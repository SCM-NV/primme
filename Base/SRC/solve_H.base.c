/*******************************************************************************
 *   PRIMME PReconditioned Iterative MultiMethod Eigensolver
 *   Copyright (C) 2015 College of William & Mary,
 *   James R. McCombs, Eloy Romero Alcalde, Andreas Stathopoulos, Lingfei Wu
 *
 *   This file is part of PRIMME.
 *
 *   PRIMME is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   PRIMME is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *******************************************************************************
 * File: solve_H.c
 * 
 * Purpose - Solves the eigenproblem for the matrix V'*A*V.
 *
 ******************************************************************************/

#include <math.h>
#include "primme.h"
#include "solve_H_@(pre).h"
#include "solve_H_private_@(pre).h"
#include "numerical_@(pre).h"
#include "ortho_@(pre).h"

/*******************************************************************************
 * Subroutine solve_H - This procedure solves the eigenproblem for the
 *            matrix H.
 *        
 * INPUT ARRAYS AND PARAMETERS
 * ---------------------------
 * H             The matrix V'*A*V
 * basisSize     Current size of the orthonormal basis V
 * maxBasisSize  The maximum size of the basis V
 * numLocked     Number of eigenvalues locked, to determine ordering shift.
 * lrwork        Length of the work array rwork
 * primme          Strucuture containing various solver parameters
 * 
 * INPUT/OUTPUT ARRAYS
 * -------------------
 * hVecs             The eigenvectors of H
 * hVals             The eigenvalues of H
 * largestRitzValue  Maintains the largest in absolute value Ritz value seen
 * rwork             Must be of size at least 3*maxBasisSize
 * iwork             Permutation array for evecs/evals with desired targeting 
 *                   order. hVecs/hVals are permuted in the right order.
 *
 * Return Value
 * ------------
 * int -  0 upon successful return
#ifdefarithm L_DEFCPLX
 *     - -1 Num_zheev was unsuccsessful
#endifarithm
#ifdefarithm L_DEFREAL
 *     - -1 Num_dsyev was unsuccsessful
#endifarithm
 ******************************************************************************/

int solve_H_@(pre)primme(@(type) *H, @(type) *hVecs, @(type) *Q, @(type) *R, double *hVals, 
   int basisSize, int maxBasisSize, double *largestRitzValue, int numLocked, double machEps, 
   int lrwork, @(type) *rwork, int *iwork, primme_params *primme,
   @(type) *V, @(type) *W, int recentlyConverged) {

   int i, j; /* Loop variables    */
   int info; /* dsyev error value */
   int index;
   int *permu, *permw;
   double targetShift;

   /*lingfei: primme_svds. define temp variables for refined projection*/
   @(type) tpone = @(tpone), tzero = @(tzero);
   int ret;
   @(type) RefShift;

#ifdefarithm L_DEFCPLX
   double  *doubleWork;
#endifarithm

   /* ---------------------- */
   /* Divide the iwork space */
   /* ---------------------- */
   permu  = iwork;
   permw = permu + basisSize;

#ifdef NUM_ESSL
   int apSize, idx;
#endif


   /* ------------------------------------------------------------------- */
   /* Copy the upper triangular portion of H into hvecs.  We need to do   */
   /* this since DSYEV overwrites the input matrix with the eigenvectors. */  
   /* Note that H is maxBasisSize-by-maxBasisSize and the basisSize-by-   */
   /* basisSize submatrix of H is copied into hvecs.                      */
   /* ------------------------------------------------------------------- */

#ifdef NUM_ESSL
   idx = 0;

   if (primme->target != primme_largest) { /* smallest or any of closest_XXX */
      for (j=0; j < basisSize; j++) {
         for (i=0; i <= j; i++) {
            rwork[idx] = H[maxBasisSize*j+i];
            idx++;
         }
      }
   }
   else { /* (primme->target == primme_largest)  */
      for (j=0; j < basisSize; j++) {
         for (i=0; i <= j; i++) {
#ifdefarithm L_DEFCPLX
            rwork[idx].r = -H[maxBasisSize*j+i].r;
            rwork[idx].i = -H[maxBasisSize*j+i].i;
#endifarithm
#ifdefarithm L_DEFREAL
            rwork[idx] = -H[maxBasisSize*j+i];
#endifarithm
            idx++;
         }
      }
   }
   
   apSize = basisSize*(basisSize + 1)/2;
   lrwork = lrwork - apSize;
#ifdefarithm L_DEFCPLX
   /* -------------------------------------------------------------------- */
   /* Assign also 3N double work space after the 2N complex rwork finishes */
   /* -------------------------------------------------------------------- */
   doubleWork = (double *) (&rwork[apsize + 2*basisSize]);

   info = Num_zhpev_zprimme(21, rwork, hVals, hVecs, basisSize, basisSize, 
      &rwork[apSize], lrwork);

   if (info != 0) {
      primme_PushErrorMessage(Primme_solve_h, Primme_num_zhpev, info, __FILE__, 
         __LINE__, primme);
      return NUM_DSPEV_FAILURE;
   }
#endifarithm
#ifdefarithm L_DEFREAL

   info = Num_dspev_dprimme(21, rwork, hVals, hVecs, basisSize, basisSize, 
      &rwork[apSize], lrwork);

   if (info != 0) {
      primme_PushErrorMessage(Primme_solve_h, Primme_num_dspev, info, __FILE__, 
         __LINE__, primme);
      return NUM_DSPEV_FAILURE;
   }
#endifarithm

#else
   if (primme->target != primme_largest) {
      for (j=0; j < basisSize; j++) {
         for (i=0; i <= j; i++) { 
            hVecs[basisSize*j+i] = H[maxBasisSize*j+i];
         }
      }      
   }
   else { /* (primme->target == primme_largest) */
      for (j=0; j < basisSize; j++) {
         for (i=0; i <= j; i++) { 
#ifdefarithm L_DEFCPLX
            hVecs[basisSize*j+i].r = -H[maxBasisSize*j+i].r;
            hVecs[basisSize*j+i].i = -H[maxBasisSize*j+i].i;
#endifarithm
#ifdefarithm L_DEFREAL
            hVecs[basisSize*j+i] = -H[maxBasisSize*j+i];
#endifarithm
         }
      }
   }

#ifdefarithm L_DEFCPLX
   /* -------------------------------------------------------------------- */
   /* Assign also 3N double work space after the 2N complex rwork finishes */
   /* -------------------------------------------------------------------- */
   doubleWork = (double *) (rwork+ 2*basisSize);

   Num_zheev_zprimme("V", "U", basisSize, hVecs, basisSize, hVals, rwork, 
                2*basisSize, doubleWork, &info);

   if (info != 0) {
      primme_PushErrorMessage(Primme_solve_h, Primme_num_zheev, info, __FILE__, 
         __LINE__, primme);
      return NUM_DSYEV_FAILURE;
   }
#endifarithm
#ifdefarithm L_DEFREAL
   Num_dsyev_dprimme("V", "U", basisSize, hVecs, basisSize, hVals, rwork, 
                lrwork, &info);

   if (info != 0) {
      primme_PushErrorMessage(Primme_solve_h, Primme_num_dsyev, info, __FILE__, 
         __LINE__, primme);
      return NUM_DSYEV_FAILURE;
   }
#endifarithm

#endif

   /* ----------------------------------------------------------------------- */
   /* Update the largest absolute Ritz value ever seen as an estimate of ||A||
    * ----------------------------------------------------------------------- */
   *largestRitzValue = Num_fmax_primme(3, 
           *largestRitzValue, fabs(hVals[0]), fabs(hVals[basisSize-1]));

   /* ---------------------------------------------------------------------- */
   /* ORDER the eigenvalues and their eigenvectors according to the desired  */
   /* target:  smallest/Largest or interior closest abs/leq/geq to a shift   */
   /* ---------------------------------------------------------------------- */

   if (primme->target == primme_smallest) 
      return 0;

   if (primme->target == primme_largest) {
      for (i = 0; i < basisSize; i++) {
         hVals[i] = -hVals[i];
      }
   }
   else { 
      /* ---------------------------------------------------------------- */
      /* Select the interior shift. Use the first unlocked shift, and not */
      /* higher ones, even if some eigenpairs in the basis are converged. */
      /* Then order the ritz values based on the closeness to the shift   */
      /* from the left, from right, or in absolute value terms            */
      /* ---------------------------------------------------------------- */

      targetShift = 
        primme->targetShifts[min(primme->numTargetShifts-1, numLocked)];

      if (primme->target == primme_closest_geq) {
   
         /* ---------------------------------------------------------------- */
         /* find hVal closest to the right of targetShift, i.e., closest_geq */
         /* ---------------------------------------------------------------- */
         for (j=0;j<basisSize;j++) 
              if (hVals[j]>=targetShift) break;
           
         /* figure out this ordering */
         index = 0;
   
         for (i=j; i<basisSize; i++) {
            permu[index++]=i;
         }
         for (i=0; i<j; i++) {
            permu[index++]=i;
         }
      }
      else if (primme->target == primme_closest_leq) {
         /* ---------------------------------------------------------------- */
         /* find hVal closest_leq to targetShift                             */
         /* ---------------------------------------------------------------- */
         for (j=basisSize-1; j>=0 ;j--) 
             if (hVals[j]<=targetShift) break;
           
         /* figure out this ordering */
         index = 0;
   
         for (i=j; i>=0; i--) {
            permu[index++]=i;
         }
         for (i=basisSize-1; i>j; i--) {
            permu[index++]=i;
         }
      }
      else if (primme->target == primme_closest_abs) {

         /* ---------------------------------------------------------------- */
         /* find hVal closest but geq than targetShift                       */
         /* ---------------------------------------------------------------- */
         for (j=0;j<basisSize;j++) 
             if (hVals[j]>=targetShift) break;

         i = j-1;
         index = 0;
         while (i>=0 && j<basisSize) {
            if (fabs(hVals[i]-targetShift) < fabs(hVals[j]-targetShift)) 
               permu[index++] = i--;
            else 
               permu[index++] = j++;
         }
         if (i<0) {
            for (i=j;i<basisSize;i++) 
                    permu[index++] = i;
         }
         else if (j>=basisSize) {
            for (j=i;j>=0;j--)
                    permu[index++] = j;
         }
      }

      /* ---------------------------------------------------------------- */
      /* Reorder hVals and hVecs according to the permutation             */
      /* ---------------------------------------------------------------- */
      for (i=0;i<basisSize;i++) 
          permw[i] = permu[i];
#ifdefarithm L_DEFCPLX
      permute_evecs_zprimme(hVals, 1, permu, (double *) rwork, basisSize, 1);
      permute_evecs_zprimme((double *) hVecs, 2, permw, (double *) rwork,
                            basisSize, basisSize);
#endifarithm
#ifdefarithm L_DEFREAL
      permute_evecs_dprimme(hVals, permu, rwork, basisSize, 1);
      permute_evecs_dprimme(hVecs, permw, rwork, basisSize, basisSize);
#endifarithm
   }

   /*lingfei: primme_svds. if the refined projection is used after
   the RayRitz projection.  It is designed for interior eigenvalue
   problem, especially for singular value problem with augmented
   method. In this case, we need compute:
   1) hVecs = Vm**T*A**T*A*Vm - 2*theta*Vm**T*A*Vm + theta*theta*Im
            = WTWm - 2*theta*Hm + theta*theta*Im;
   2) Another approach is for two-stages SVD problem;
      (AVm - theta*Vm) = Wm - thetaVm = QR; */
   if (primme->projectionParams.projection == primme_RR_Refined &&
            primme->projectionParams.refinedScheme == primme_OneAccuShift_QR) {
#ifdefarithm L_DEFCPLX
       RefShift.r = -primme->targetShifts[numLocked+recentlyConverged];
       RefShift.i = 0.0L;
#endifarithm
#ifdefarithm L_DEFREAL
       RefShift = -primme->targetShifts[numLocked+recentlyConverged];
#endifarithm
       /*printf("RefShift = %e\n",RefShift);*/
       if (primme->qr_need == 1 && Q != NULL && R != NULL) {
           for(i = 0; i < basisSize; i++){
               Num_@(pre)copy_@(pre)primme(primme->nLocal, &W[primme->nLocal*i], 
               1, &Q[primme->nLocal*i], 1);
               Num_axpy_@(pre)primme(primme->nLocal, RefShift, &V[primme->nLocal*i],
               1, &Q[primme->nLocal*i], 1);
           }


        /* --------------------------------------------------------------------- */
        /* Zero the R array to prevent floating point traps during all-reduce */
           /* --------------------------------------------------------------------- */
          for (i = 0; i < primme->maxBasisSize*primme->maxBasisSize; i++) {
               R[i] = tzero;
           }
           /*printf("primme->qr_need = %d\n",primme->qr_need);*/
           ret = ortho_@(pre)primme(Q, R, primme->nLocal, 0,   
                basisSize-1, NULL, 0, 0, primme->nLocal, 
                primme->iseed, machEps, rwork, lrwork, primme);
           if (ret < 0) {
               primme_PushErrorMessage(Primme_solve_h, Primme_ortho,
               ret, __FILE__, __LINE__, primme);
               return ORTHO_FAILURE;
           }
           /*reset primme->qr_need = 0 to switch to one-colomn qr updating model*/
           primme->qr_need = 0;
       }
       else { /*primme->qr_need == 0*/
           /*printf("primme->qr_need = %d\n",primme->qr_need);*/
           Num_@(pre)copy_@(pre)primme(primme->nLocal, &W[primme->nLocal*(basisSize-1)], 
                1, &Q[primme->nLocal*(basisSize-1)], 1);
           Num_axpy_@(pre)primme(primme->nLocal, RefShift, &V[primme->nLocal*(basisSize-1)], 
                1, &Q[primme->nLocal*(basisSize-1)], 1);
           ret = ortho_@(pre)primme(Q, R, primme->nLocal, basisSize-1,
                basisSize-1, NULL, 0, 0, primme->nLocal,
                primme->iseed, machEps, rwork, lrwork,primme);
           if (ret < 0) {
               primme_PushErrorMessage(Primme_solve_h, Primme_ortho,
               ret, __FILE__, __LINE__, primme);
               return ORTHO_FAILURE;
            }
       }
    }
         
    if (primme->projectionParams.projection == primme_RR_Refined && 
            primme->projectionParams.refinedScheme == primme_OneAccuShift_QR) {
      for (i = 0; i < primme->maxBasisSize*primme->maxBasisSize; i++) {
         hVecs[i] = tzero;
      }
      for (j=0; j < basisSize; j++) {
         for (i=0; i <= j; i++) { 
            hVecs[basisSize*j+i] = R[maxBasisSize*j+i];
         }
      }      
      /*Since Ritz vectors in hVecs is not needed, we use hVecs to hold refined
        Ritz vectors. Also, we use the last colomn of R to hold the singular 
        value. Note, hvecs holds transpose(V) rather than V and sorted according
        to the descending order*/

#ifdefarithm L_DEFCPLX
/*lingfei: Assign also 5N double work space after the 3N complex rwork finishes */
     doubleWork = (double *) (rwork+ 3*basisSize);  
/*lingfei: you may want to use a temp memory to hold the singular
value returned from zgesvd for debugging. It is in first NULL.*/
     Num_zgesvd_zprimme("N","O", basisSize, basisSize, hVecs, basisSize,
            NULL, NULL, basisSize, NULL,
            basisSize, rwork, 3*basisSize, doubleWork, &info);

      if (info != 0) {
            primme_PushErrorMessage(Primme_solve_h, Primme_num_zgesvd, info, __FILE__, 
                __LINE__, primme);
            return NUM_ZGESVD_FAILURE;
       }
#endifarithm

#ifdefarithm L_DEFREAL
      Num_dgesvd_dprimme("N","O", basisSize, basisSize, hVecs, basisSize, 
            &R[maxBasisSize*(maxBasisSize-1)], NULL, basisSize, NULL, 
            basisSize, rwork, lrwork, &info);
      
      if (info != 0) {
            primme_PushErrorMessage(Primme_solve_h, Primme_num_dgesvd, info, __FILE__, 
                __LINE__, primme);
            return NUM_DGESVD_FAILURE;
       }
#endifarithm

      /* zero the last colomn of R if reaching maxBasisSize-1 to get ready for
         last colomn updating of R when performing one-colomn qr updating*/
      if (basisSize == primme->maxBasisSize -1){
          for(i=0; i< maxBasisSize; i++)
              R[maxBasisSize*(maxBasisSize-1)+i] = tzero;
      }

      /*compute V from transpose(V) and rearrange V according to ascending order*/
      for (j=0; j < basisSize; j++) {
         for (i=0; i < basisSize; i++) { 
            rwork[basisSize*j+i] = hVecs[basisSize*i+ (basisSize-1-j)];
         }
      }      
      Num_@(pre)copy_@(pre)primme(basisSize*basisSize, rwork, 1, hVecs, 1);
   
      /* compute rayleigh quotient lambda(1) = y_1T*H*y_1 */
      Num_symv_@(pre)primme("U", basisSize, tpone, H, maxBasisSize, hVecs, 1, tzero, rwork, 1);
      /*lingfei: in the future, if a block refined projection is developed, 
      we may change Num_symv to Num_symm for computing blocked lamda values.*/
     /* Num_symm_@(pre)primme("L", "U", basisSize, 1, tpone, H, maxBasisSize, hVecs, 
      basisSize, tzero, rwork, basisSize);*/
#ifdefarithm L_DEFCPLX
      RefShift = Num_dot_@(pre)primme(basisSize, hVecs, 1, rwork,1);
      hVals[0] = RefShift.r;
#endifarithm
#ifdefarithm L_DEFREAL
      hVals[0] = Num_dot_@(pre)primme(basisSize, hVecs, 1, rwork, 1);
#endifarithm      
    }

   return 0;   
}

#ifdefarithm L_DEFCPLX
/******************************************************************************
 * Subroutine permute_evecs- This routine permutes a set of vectors according
 *            to a permutation array perm. It is supposed to be called on 
 *            the eigenvectors after the eigenvalues have been sorted, so that 
 *            the vectors are in the same order as the sorted eigenvalues.
 *
 *            Each element of evecs is of size elemSize (=1 or 2 doubles),
 *            so the same function can be used for both double/complex arrays.
 *
 * INPUT ARRAYS AND PARAMETERS
 * ---------------------------
 * elemSize Size of each element: 1 double, or 2 doubles (i.e., complex)
 * rwork  Used as temorary space to swap the vectors
 * nev    Number of eigenvectors/eigenvalues
 * nLocal Number of rows of each vector this process stores
 *
 * INPUT/OUTPUT ARRAYS
 * -------------------
 * perm  The array indicating what order the eigenvectors must be permuted to.
 *       perm[i] indicates that the ith eigenvector in the sorted ordering
 *       should be the perm[i]-th vector from the original ordering.
 * evecs The eigenvectors to be put in sorted order.
 *
 ******************************************************************************/
     
void permute_evecs_zprimme(double *evecs, int elemSize, int *perm, 
   double *rwork, int nev, int nLocal) {

   int currentIndex;     /* Index of eigenvector in sorted order              */
   int sourceIndex;      /* Position of out-of-order vector in original order */
   int destinationIndex; /* Position of out-of-order vector in sorted order   */
   int tempIndex;        /* Used to swap                                      */
   int tmpSize;
   
   currentIndex = 0;
   tmpSize = nLocal*elemSize;

   /* Continue until all eigenvectors are in the sorted order */

   while (1) {

      /* Find a vector that does not belong in its original position */
      while ((currentIndex < nev) && (perm[currentIndex] == currentIndex)) {
         currentIndex++;
      }

      /* Return if they are in the sorted order */
      if (currentIndex >= nev) {
         return;
      }

      /* Copy the vector to a buffer for swapping */
      Num_dcopy_primme(tmpSize, &evecs[currentIndex*tmpSize], 1, rwork, 1);

      destinationIndex = currentIndex;
      /* Copy vector perm[destinationIndex] into position destinationIndex */

      while (perm[destinationIndex] != currentIndex) {

         sourceIndex = perm[destinationIndex];
         Num_dcopy_primme(tmpSize, &evecs[sourceIndex*tmpSize], 1, 
            &evecs[destinationIndex*tmpSize], 1);
         tempIndex = perm[destinationIndex];
         perm[destinationIndex] = destinationIndex;
         destinationIndex = tempIndex;
      }

      /* Copy the vector from the buffer to where it belongs */
      Num_dcopy_primme(tmpSize, rwork, 1, &evecs[destinationIndex*tmpSize], 1);
      perm[destinationIndex] = destinationIndex;

      currentIndex++;
   }

  /***************************************************************************/
} /* end of permute_evecs
   ***************************************************************************/
#endifarithm
#ifdefarithm L_DEFREAL
/******************************************************************************
 * Subroutine permute_evecs- This routine permutes a set of vectors according
 *            to a permutation array perm. It is supposed to be called on 
 *            the eigenvectors after the eigenvalues have been sorted, so that 
 *            the vectors are in the same order as the sorted eigenvalues.
 *
 *
 * INPUT ARRAYS AND PARAMETERS
 * ---------------------------
 * rwork  Used as temorary space to swap the vectors
 * nev    Number of eigenvectors/eigenvalues
 * nLocal Number of rows of each vector this process stores
 *
 * INPUT/OUTPUT ARRAYS
 * -------------------
 * perm  The array indicating what order the eigenvectors must be permuted to.
 *       perm[i] indicates that the ith eigenvector in the sorted ordering
 *       should be the perm[i]-th vector from the original ordering.
 * evecs  The eigenvectors to be put in sorted order.
 *
 ******************************************************************************/
     
void permute_evecs_dprimme(double *evecs, int *perm, double *rwork, int nev, 
   int nLocal) {

   int currentIndex;     /* Index of eigenvector in sorted order              */
   int sourceIndex;      /* Position of out-of-order vector in original order */
   int destinationIndex; /* Position of out-of-order vector in sorted order   */
   int tempIndex;        /* Used to swap                                      */
   
   currentIndex = 0;

   /* Continue until all eigenvectors are in the sorted order */

   while (1) {

      /* Find a vector that does not belong in its original position */
      while ((currentIndex < nev) && (perm[currentIndex] == currentIndex)) {
         currentIndex++;
      }

      /* Return if they are in the sorted order */
      if (currentIndex >= nev) {
         return;
      }

      /* Copy the vector to a buffer for swapping */
      Num_dcopy_primme(nLocal, &evecs[currentIndex*nLocal], 1, rwork, 1);

      destinationIndex = currentIndex;
      /* Copy vector perm[destinationIndex] into position destinationIndex */

      while (perm[destinationIndex] != currentIndex) {

         sourceIndex = perm[destinationIndex];
         Num_dcopy_primme(nLocal, &evecs[sourceIndex*nLocal], 1, 
            &evecs[destinationIndex*nLocal], 1);
         tempIndex = perm[destinationIndex];
         perm[destinationIndex] = destinationIndex;
         destinationIndex = tempIndex;
      }

      /* Copy the vector from the buffer to where it belongs */
      Num_dcopy_primme(nLocal, rwork, 1, &evecs[destinationIndex*nLocal], 1);
      perm[destinationIndex] = destinationIndex;

      currentIndex++;
   }

  /***************************************************************************/
} /* end of permute_evecs
   ***************************************************************************/
#endifarithm

