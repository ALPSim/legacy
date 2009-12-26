 /*****************************************************************************
 *
 * ALPS DMFT Project
 *
 * Copyright (C) 2005 - 2009 by Emanuel Gull <gull@phys.columbia.edu>
 *                              Philipp Werner <werner@itp.phys.ethz.ch>,
 *                              Matthias Troyer <troyer@comp-phys.org>
 *
 *
 * THIS SOFTWARE NEEDS AN APPROPRIATE LICENSE BLOCK HERE
 *****************************************************************************/

/* $Id: hirschfyeaux.h 328 2008-09-09 19:02:18Z gullc $ */

/// @file hirschfyesim.h
/// @brief the actual Hirsch-Fye simulation

#ifndef ALPS_DMFT_HIRSCHFYEAUX_H
#define ALPS_DMFT_HIRSCHFYEAUX_H

#include "types.h"

#include <alps/osiris/dump.h>
#include <alps/osiris/std/vector.h>

#include <boost/numeric/bindings/traits/ublas_matrix.hpp>
#include <boost/numeric/ublas/io.hpp>

#include <stack>
#include <queue>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
extern "C" void dger_(const void *m, const void* n, const void* alpha,const void*  x, const void* incx, const void*y, const void*  incy,
                      const void* a,const void* lda) ;

typedef boost::numeric::ublas::matrix<double,boost::numeric::ublas::column_major> dense_matrix;
///@brief Save a matrix
///@parm odump Alps dump
///@parm m The dense matrix to be stored
inline alps::ODump& operator << (alps::ODump& odump, const dense_matrix& m) {
  odump << m.size1() << m.size2();
  for (unsigned int i=0; i<m.size1(); i++) {
  	for (unsigned int j=0; j<m.size2(); j++) {
  		odump << m(i,j);
  	}
  }
  return odump;
}

///@brief Restore a matrix
///@parm idump Alps dump
///@parm m The matrix to be restored
inline alps::IDump& operator >> (alps::IDump& idump, dense_matrix& m) {
  int size1, size2;
  idump >> size1;
  idump >> size2;
  m.resize(size1, size2);
  for (unsigned int i=0; i<m.size1(); i++) {
  	for (unsigned int j=0; j<m.size2(); j++) {
  		idump >> m(i,j);
  	}
  }  
  return idump;
}


/// compute Green's function matrix for a given spin configuration, from Greens
//function for spins set to zero. Return sign of det.
double update_from_zero(dense_matrix & Green, dense_matrix & Green0, std::vector<int> & spins, double l);


/// attempt single spin flip and (if successful) compute Green's function matrix for the new configuration
template<class RNG> 
void update_single_spin(RNG & rng, dense_matrix & Green_up, dense_matrix & Green_down, std::vector<int> & spins, double lambda, int &sign) 
{
  unsigned int N(spins.size());
  
  // choose random site
  unsigned int site = (unsigned int)(N*rng());
  // calculate ratio of determinants
  double r_up = 1 + (1-Green_up(site,site))*(exp(-2*lambda*spins[site])-1);
  double r_down = 1 + (1-Green_down(site,site))*(exp(2*lambda*spins[site])-1);
  double det_rat = r_up*r_down; 
  
  // if update successful ...
  if (rng() < fabs(det_rat/(1+det_rat))) {
  	//if(det_rat/(1+det_rat) <0)
  	if(det_rat<0)
      sign *=-1;
  	// update Green's function
  	double tmp1 = exp(-2*lambda*spins[site])-1;
  	double tmp2 = exp(+2*lambda*spins[site])-1;
    
    double vi_up[N], uj_up[N];
    double vi_dn[N], uj_dn[N];
    double alpha_up=tmp1/(1+(1-Green_up(site,site))*tmp1);
    double alpha_dn=tmp2/(1+(1-Green_down(site,site))*tmp2);
    for(uint i=0;i<N;++i){
      vi_up[i]=Green_up(i,site)-(i==site);
      vi_dn[i]=Green_down(i,site)-(i==site);
      uj_up[i]=Green_up  (site, i);
      uj_dn[i]=Green_down(site, i);
    }
    int one=1;
    dger_(&N,&N,&alpha_up,vi_up,&one,uj_up,&one,&(Green_up  (0,0)),&N);
    dger_(&N,&N,&alpha_dn,vi_dn,&one,uj_dn,&one,&(Green_down(0,0)),&N);
  	// update spin
  	spins[site] = -spins[site];
    
  }
}

#endif
