/*****************************************************************************
 *
 * ALPS DMFT Project
 *
 * Copyright (C) 2005 - 2009 by Emanuel Gull <gull@phys.columbia.edu>
 *                              Philipp Werner <werner@itp.phys.ethz.ch>,
 *                              Sebastian Fuchs <fuchs@theorie.physik.uni-goettingen.de>
 *                              Matthias Troyer <troyer@comp-phys.org>
 *
 *
 * This software is part of the ALPS Applications, published under the ALPS
 * Application License; you can use, redistribute it and/or modify it under
 * the terms of the license, either version 1 or (at your option) any later
 * version.
 * 
 * You should have received a copy of the ALPS Application License along with
 * the ALPS Applications; see the file LICENSE.txt. If not, the license is also
 * available from http://alps.comp-phys.org/.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT 
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE 
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/

#ifndef FOURIER_TRANSFORM_H
#define FOURIER_TRANSFORM_H
#include <complex>
#include "types.h" // for the multiple_vector_type
#include "green_function.h" // for the multiple_vector_type
#include <alps/parameter.h>


inline std::complex<double> f_omega(std::complex<double> iw, double c1, double c2, double c3) {
  std::complex<double> iwsq=iw*iw;
  return c1/iw + c2/(iwsq) + c3/(iw*iwsq);
}


inline double f_tau(double tau, double beta, double c1, double c2, double c3) {
  return -0.5*c1 + (c2*0.25)*(-beta+2.*tau) + (c3*0.25)*(beta*tau-tau*tau);
}


class FourierTransformer
{
public:
  
  FourierTransformer(const double beta, const int n_flavor, const int n_site)
  {
    beta_=beta;
    //mu_=mu;
    c1_.resize(n_flavor);
    c2_.resize(n_flavor);
    c3_.resize(n_flavor);
    Sc0_.resize(n_flavor);
    Sc1_.resize(n_flavor);
    Sc2_.resize(n_flavor);
    for(int f=0;f<n_flavor;++f){
      c1_[f].resize(n_site);
      c2_[f].resize(n_site);
      c3_[f].resize(n_site);
      Sc0_[f].resize(n_site);
      Sc1_[f].resize(n_site);
      Sc2_[f].resize(n_site);
      for(int i=0;i<n_site;++i){
        c1_[f][i].resize(n_site);
        c2_[f][i].resize(n_site);
        c3_[f][i].resize(n_site);
        Sc0_[f][i].resize(n_site);
        Sc1_[f][i].resize(n_site);
        Sc2_[f][i].resize(n_site);
        for(int j=0;j<n_site;++j){
          c1_[f][i][j] = (i==j) ? 1. : 0.;
          c2_[f][i][j]= 0.;
          c3_[f][i][j]= 0.;
          Sc0_[f][i][j] = 0.;
          Sc1_[f][i][j]= 0.;
          Sc2_[f][i][j]= 0.;
        }
      }
    } 
  }
  
  
  virtual ~FourierTransformer() {}
  virtual void forward_ft(const itime_green_function_t &G_tau, matsubara_green_function_t &G_omega) const;
  virtual void backward_ft(itime_green_function_t &G_tau, const matsubara_green_function_t &G_omega) const;
  virtual void append_tail(matsubara_green_function_t& G_omega, const matsubara_green_function_t& G0_omega,
                           const int nfreq_measured) const;
  
  static void generate_transformer(const alps::Parameters &parms,
                                   boost::shared_ptr<FourierTransformer> &fourier_ptr);
  static void generate_transformer_U(const alps::Parameters &parms,
                                     boost::shared_ptr<FourierTransformer> &fourier_ptr,
                                     const std::vector<double> &densities);
  static void generate_transformer_U(const alps::Parameters &parms,
                                     boost::shared_ptr<FourierTransformer> &fourier_ptr,
                                     const std::vector<double> &densities,
                                     const std::vector<double> &magnetization);
  
protected:
  
  double beta_;
  std::vector<std::vector<std::vector<double> > > c1_;
  std::vector<std::vector<std::vector<double> > > c2_;
  std::vector<std::vector<std::vector<double> > > c3_;
  std::vector<std::vector<std::vector<double> > > Sc0_; //coefficients for the self-energy
  std::vector<std::vector<std::vector<double> > > Sc1_;  
  std::vector<std::vector<std::vector<double> > > Sc2_;  
  
  
};



class SimpleG0FourierTransformer : public FourierTransformer
{
public: 
  SimpleG0FourierTransformer(const double beta, const double mu, const double h, const int n_flavor, 
                             const std::vector<double>& eps, const std::vector<double>& epssq)
  : FourierTransformer(beta, n_flavor, 1)
  {
    for(int f=0; f<n_flavor; f++) {
      double hsign = f%2 ? h : -h;
      double mub = mu + hsign;
      c1_[f][0][0] = 1.;
      c2_[f][0][0] = eps[f] - mub;
      c3_[f][0][0] = epssq[f] - 2*mub*eps[f] + mub*mub;
      // Note: the coefficients are the same if h is uniform field as if it is staggered
    }
  }
};


class GFourierTransformer : public FourierTransformer
{
public: 
  GFourierTransformer(const double beta, const double mu, const double U, const int n_flavor, const int n_site, 
                      const std::vector<double>& densities, 
                      const std::vector<std::vector<double> >& eps, const std::vector<std::vector<double> >& epssq)
  : FourierTransformer(beta, n_flavor, n_site)  
  {
    for(int f=0;f<n_flavor;++f){
      int fbar = f%2==0 ? f+1 : f-1;
      //std::cerr << "dens: " << densities[fbar] << std::endl;
      for(int i=0;i<n_site;++i){
        c1_[f][i][i] = 1.;
        c2_[f][i][i] = eps[f][i] - mu + U*densities[fbar]; 
        c3_[f][i][i] = epssq[f][i] - 2.*mu*eps[f][i] + mu*mu 
        + 2.*U*densities[fbar]*(eps[f][i]-mu) + U*U*densities[fbar];
        Sc0_[f][i][i] = U * (densities[fbar]-0.5);
        Sc1_[f][i][i] = U*U * densities[fbar] * (1-densities[fbar]);
        //std::cout << "eps: " << f << " " << i << " " << eps[f][i] << "\n";
        Sc2_[f][i][i] = 0;
      }
    }
  }
};



class FFunctionFourierTransformer:public FourierTransformer
{
public:
  FFunctionFourierTransformer(double beta, double mu, double epsilonsq_av, int n_flavor, int n_site)
  :FourierTransformer(beta, n_flavor, n_site){
    //std::cout<<"FFourier Transformer: beta: "<<beta<<" mu: "<<mu<<"epsilonsq_av: "<<epsilonsq_av<<std::endl;
    epsilonsq_av_=epsilonsq_av; //this is the integral of the second moment of the dos: \int_-\infty^\infty e^2 rho(e) de. It is t^2 for semicircle...
    for(int f=0;f<n_flavor;++f){
      for(int i=0;i<n_site;++i){
        for(int j=0;j<n_site;++j){
          c1_[f][i][j]=epsilonsq_av;
          c2_[f][i][j]=0;//-(2*epsilonsq_av*mu+mu*mu*mu);
          c3_[f][i][j]=0;
        }
      }
    }
  }
  
  virtual void forward_ft(const itime_green_function_t &G_tau, matsubara_green_function_t &G_omega) const{
    std::cout<<"implement 2nd derivatives for F function before using this!"<<std::endl;
    FourierTransformer::forward_ft(G_tau, G_omega);
  }
  
  virtual void backward_ft(itime_green_function_t &G_tau, const matsubara_green_function_t &G_omega) const{
    FourierTransformer::backward_ft(G_tau, G_omega);
    for(unsigned int j=0;j<G_tau.nflavor();++j){
      G_tau(G_tau.ntime()-1,j)=-epsilonsq_av_-G_tau(0,j);
    }
  }
  
  virtual ~FFunctionFourierTransformer(){}
  
private:
  double epsilonsq_av_;
};

#endif
