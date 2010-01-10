/*****************************************************************************
*
* ALPS Project Applications
*
* Copyright (C) 2010 by Sebastian  Fuchs <fuchs@comp-phys.org>
*                       Thomas Pruschke <pruschke@comp-phys.org>
*                       Matthias Troyer <troyer@comp-phys.org>
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

#include "maxent.hpp"
#include <boost/numeric/bindings/lapack/gesv.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/numeric/bindings/traits/ublas_matrix.hpp>





MaxEntSimulation::MaxEntSimulation(const alps::ProcessList& w, const boost::filesystem::path& fn) :
  alps::scheduler::Task(w,fn), MaxEntHelper(parms), 
  alpha(parms["N_ALPHA"]), norm(parms.value_or_default("NORM", 1.)), hartree(parms.value_or_default("HARTREE", 0.))
{
  const double alpha_min = parms["ALPHA_MIN"];
  const double alpha_max = parms["ALPHA_MAX"];
  alpha[0] = alpha_max;
  for (int a=1; a<alpha.size(); ++a) 
    alpha[a] =  alpha[a-1] * std::pow(alpha_min/alpha_max, 1./double(alpha.size()-1));
  boost::filesystem::path dir = fn.branch_path();
  std::string name = fn.leaf();
  name.erase(name.size()-6, 6);
  spex_str = new std::ofstream(boost::filesystem::complete(name+"spex.dat", dir).string().c_str());
  chisq_str = new std::ofstream(boost::filesystem::complete(name+"chi2.dat", dir).string().c_str());
  avspec_str = new std::ofstream(boost::filesystem::complete(name+"avspec.dat", dir).string().c_str());
  maxspec_str = new std::ofstream(boost::filesystem::complete(name+"maxspec.dat", dir).string().c_str());
  chispec_str = new std::ofstream(boost::filesystem::complete(name+"chispec.dat", dir).string().c_str());
  prob_str = new std::ofstream(boost::filesystem::complete(name+"prob.dat", dir).string().c_str());
  complete_str = new std::ofstream(boost::filesystem::complete(name+"complete.dat", dir).string().c_str());
}


MaxEntSimulation::~MaxEntSimulation() 
{
  spex_str->close();
  delete spex_str;
  avspec_str->close();
  delete avspec_str;
  maxspec_str->close();
  delete maxspec_str;
  chispec_str->close();
  delete chispec_str;
  prob_str->close();
  delete prob_str;
  complete_str->close();
  delete complete_str;
}




void MaxEntSimulation::dostep() 
{
  if (finished())
    return;
  vector_type lprob(alpha.size());
  vector_type chi_sq(alpha.size());
  std::vector<vector_type> spectra(alpha.size());
  vector_type u = transform_into_singular_space(Default());
  for (int a=0; a<alpha.size(); ++a) {
    std::cerr << "alpha it: " << a << "\t";
    u = levenberg_marquardt(u, alpha[a]);
    vector_type A = get_spectrum(u);
    std::cerr << "norm: " << boost::numeric::ublas::sum(transform_into_real_space(u)) << "\t";
    for (int i=0; i<A.size(); ++i) 
      *spex_str << alpha[a] << " " << omega_coord(i) << " " << A[i] << "\n";
    *spex_str << "\n";
    lprob[a] = log_prob(u, alpha[a]);
    spectra[a] = A;
    double chi_squared = chi2(transform_into_real_space(u));
    chi_sq[a] = chi_squared;
    std::cerr << "chi2  : " << chi_squared << std::endl;
  }
  *spex_str << "\n";
  for (int a=0; a<chi_sq.size(); ++a) 
    *chisq_str << alpha[a] << " " << chi_sq[a] << std::endl;
  double a_chi = 0;
  double diff = std::abs(chi_sq[0]-1);
  for (int a=1; a<chi_sq.size(); ++a) {
    double diff_new = std::abs(chi_sq[a]-1);
    if (diff_new < diff) {
      diff = diff_new;
      a_chi = a;
    }
  }
  vector_type def = get_spectrum(transform_into_singular_space(Default()));
  for (int i=0; i<spectra[0].size(); ++i) 
    *chispec_str << omega_coord(i) << " " << spectra[a_chi][i]*norm << " " << def[i]*norm << std::endl;
  boost::numeric::ublas::vector<double>::const_iterator max_lprob = std::max_element(lprob.begin(), lprob.end());  
  const int max_a = max_lprob-lprob.begin();
  const double factor = chi_scale_factor(spectra[max_a], chi_sq[max_a], alpha[max_a]);
  std::cerr << "chi scale factor: " << factor << std::endl;
  for (int i=0; i<spectra[0].size(); ++i) 
    *maxspec_str << omega_coord(i) << " " << spectra[max_a][i]*norm << " " << def[i]*norm << std::endl;
  vector_type prob(lprob.size());
  for (int a=0; a<prob.size(); ++a) 
    prob[a] = exp(lprob[a]-*max_lprob);
  double probnorm = 0;
  for (int a=0; a<prob.size()-1; ++a) 
    probnorm += 0.5*(prob[a]+prob[a+1])*(alpha[a]-alpha[a+1]);
  prob /= probnorm;
  for (int a=0; a<prob.size(); ++a) {
    *prob_str << alpha[a] << "\t" << prob[a] << "\n";
  }
  double postprobdef = 0;
  for (int a=0; a<lprob.size()-1; ++a) 
    postprobdef += 0.5*(exp(lprob[a])+exp(lprob[a+1]))*(alpha[a]-alpha[a+1]);
  std::cout << "posterior probability of the default model: " << postprobdef << std::endl;
  vector_type avspec(spectra[0].size());
  for (int i=0; i<avspec.size(); ++i) {
    avspec[i] = 0.;
    for (int a=0; a<prob.size()-1; ++a) 
      avspec[i] += 0.5*(prob[a]*spectra[a][i] +prob[a+1]*spectra[a+1][i])*(alpha[a]-alpha[a+1]);
  }
  for (int i=0; i<avspec.size(); ++i) 
    *avspec_str << omega_coord(i) << " " << avspec[i]*norm << " " << def[i]*norm << std::endl;
  omega_complex_type complete_spec = real_and_imaginary_part(avspec, norm, hartree);
  for (int i=0; i<complete_spec.first.size(); ++i) 
    *complete_str << complete_spec.first[i] << " " 
		  << complete_spec.second[i].real() << " " << complete_spec.second[i].imag() << std::endl;
  finish();
}




MaxEntSimulation::vector_type MaxEntSimulation::levenberg_marquardt(vector_type u, const double alpha) const 
{
  using namespace boost::numeric;
  double mu = 1e-18;
  const double nu = 1.3;
  const int max_it = 1000;
  //double Q_new;
  double Q1;
  int it = 0;
  for (; it<max_it; it++) {
    vector_type delta;
    // double Q_old = Q(u, alpha);
    //std::cerr << "Q: " << Q_old << std::endl;
    int it2 = 0;
    for (; it2<max_it; ++it2) {
      delta = iteration(u, alpha, mu);
      // const double Q_new = Q(u+delta, alpha);
      Q1 = Q(u+delta, alpha);
      if (step_length(delta, u)<=0.02) {
	break;
      }
      else if (mu<1e20) {
	mu *= nu;
      }
      /*vector_type delta_mu = iteration(u, alpha, mu);
      vector_type delta_mu_nu = iteration(u, alpha, mu/nu);
      const double Q_mu = Q(u+delta_mu, alpha);
      const double Q_mu_nu = Q(u+delta_mu_nu, alpha);
      //std::cout << "mu: " << mu << std::endl;
      if (Q_mu<Q_old) {
	if (Q_mu_nu<Q_mu) { 
	  delta = delta_mu_nu;
	  Q_new = Q_mu_nu;
	  if (mu > 1e-20)
	    mu /= nu;
	}
	else {
	  delta = delta_mu;
	  Q_new = Q_mu;
	}
	break;
      }
      else if (mu<1e20) 
	mu *= nu;
      else {
	delta = delta_mu;
	Q_new = Q_mu;
	break;
	}*/
    } 
    u += delta;
    //std::cerr << mu << " " << convergence(u, alpha) << "\n";
    //if (ublas::norm_2(delta)<1e-6 || fabs(Q_new-Q_old)<1e-6)
    if (convergence(u, alpha)<=1e-4)
      break;
  }
  std::cerr <<"Iterations: " << it+1 << "\t"
	    << "Q: " << Q1 << "\t";
  return u;
}




MaxEntSimulation::vector_type MaxEntSimulation::iteration(vector_type u, const double alpha, const double mu) const 
{
  using namespace boost::numeric;
  matrix_type M = left_side(u);
  /*for (int i=0; i<M.size1(); ++i) {
    for (int j=0; j<M.size1(); ++j) {
      std::cout << i << " " << j << " " << M(i,j) << std::endl;
    }
    std::cout << std::endl;
  }
  exit(1);*/
  for (int i=0; i<M.size1(); ++i) 
    M(i,i) += alpha + mu;//*M(i,i);
  vector_type b = right_side(u) + alpha*u;
  matrix_type B(b.size(),1);
  for (int i=0; i<M.size1(); ++i) 
    B(i,0) = -b[i];
  bindings::lapack::gesv(M, B);
  return ublas::matrix_column<matrix_type>(B, 0);
}




void MaxEntSimulation::write_xml_body(alps::oxstream& out, const boost::filesystem::path&) const
{
  out << alps::start_tag("AVERAGES");
  out << alps::start_tag("SCALAR_AVERAGE") << alps::attribute("name","Zeug") << alps::no_linebreak
      << alps::start_tag("MEAN") << 42 << alps::end_tag("MEAN")
      << alps::end_tag("SCALAR_AVERAGE");
  out << alps::end_tag("AVERAGES");
}
