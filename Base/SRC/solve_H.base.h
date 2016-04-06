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
 * File: solve_H.h
 *
 * Purpose - Contains prototype for solving the projected eigenproblem.
 *
 ******************************************************************************/

#ifndef SOLVE_H_H
#define SOLVE_H_H

int solve_H_@(pre)primme(@(type) *H, int basisSize, int ldH, @(type) *R, int ldR,
   @(type) *QV, int ldQV, @(type) *hU, int ldhU, @(type) *hVecs, int ldhVecs,
   double *hVals, double *hSVals, int numConverged, double machEps, int lrwork,
   @(type) *rwork, int *iwork, primme_params *primme);

int prepare_vecs_@(pre)primme(int basisSize, int i0, int blockSize,
      @(type) *H, int ldH, double *hVals, double *hSVals, @(type) *hVecs,
      int ldhVecs, int targetShiftIndex, int *arbitraryVecs, int *flags,
      int RRForAll, double machEps, int rworkSize, @(type) *rwork, int *iwork,
      primme_params *primme);


#endif
