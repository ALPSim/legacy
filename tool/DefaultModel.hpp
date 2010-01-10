/*****************************************************************************
*
* ALPS Project Applications
*
* Copyright (C) 2010 by Sebastian Fuchs <fuchs@comp-phys.org>
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


#ifndef ALPS_MAXENT_DEFAULT_MODEL_HPP
#define ALPS_MAXENT_DEFAULT_MODEL_HPP


#include <alps/parameter.h>



class DefaultModel 
{
public:
  DefaultModel(const alps::Parameters& p) : 
    omega_max(p["OMEGA_MAX"]), 
    omega_min((p["KERNEL"] == "bosonic") ? 0. : 
	      static_cast<double>(p.value_or_default("OMEGA_MIN", -omega_max))),
    blow_up_(p.value_or_default("BLOW_UP", 1.))
  {}

  virtual ~DefaultModel(){}
  virtual double omega(const double x) const = 0;
  virtual double D(const double omega) const = 0;
  virtual double x(const double t=0) const = 0;
  double omega_of_t(const double t) const { return omega_min + (omega_max-omega_min)*t; }
  double t_of_omega(const double omega) const { return (omega-omega_min)/(omega_max-omega_min); }
  double blow_up() const { return blow_up_; }

protected:
  const double omega_max;
  const double omega_min;
  const double blow_up_;
};



class FlatDefaultModel : public DefaultModel
{
public:

  FlatDefaultModel(const alps::Parameters& p) : DefaultModel(p) {}

  double omega(const double x) const {
    return x/blow_up()*(omega_max-omega_min) + omega_min;
  }

  double D(const double) const {
    return 1./(omega_max-omega_min);
  }

  double x(const double t) const {
    return t*blow_up();
  }

};



class Model
{
public:
  virtual double operator()(const double omega)=0;
  virtual ~Model(){}
};



class Gaussian : public Model 
{
public:
  Gaussian(const alps::Parameters& p) : sigma(static_cast<double>(p["SIGMA"])) {}
  
  virtual double operator()(const double omega) {
    return std::exp(-omega*omega/2./sigma/sigma)/sqrt(2*M_PI)/sigma;
  }

private:
  const double sigma;
};



class DoubleGaussian : public Gaussian
{
public:
  DoubleGaussian(const alps::Parameters& p) : 
    Gaussian(p), shift(static_cast<double>(p["SHIFT"])){}

  double operator()(const double omega) {
    return 0.5*(Gaussian::operator()(omega-shift) + Gaussian::operator()(omega+shift));
  }

private:
  const double shift;
};



class GeneralDoubleGaussian : public Gaussian
{
public:
  GeneralDoubleGaussian(const alps::Parameters& p) : 
    Gaussian(p), bnorm(static_cast<double>(p["BOSE NORM"])), shift(p.value_or_default("SHIFT", 0.)) {}
  
  double operator()(const double omega) {
    if (omega > 0)
      return Gaussian::operator()(omega);
    else
      return bnorm*Gaussian::operator()(omega+shift);
  }
  
private:
  const double bnorm; 
  const double shift;
};


class TabFunction : public Model
{
public:
  TabFunction(const alps::Parameters& p, char* name) //: index(0)
  {
    std::ifstream defstream(static_cast<std::string>(p[name]).c_str());
    if (!defstream)
      boost::throw_exception(std::invalid_argument("could not open default model file: "+p[name]));
    double om, D;
    while (defstream >> om >> D) {
      Omega.push_back(om);
      Def.push_back(D);
    }
    double omega_max = p["OMEGA_MAX"]; 
    double omega_min = (p["KERNEL"] == "bosonic") ? 0. : 
			static_cast<double>(p.value_or_default("OMEGA_MIN", -omega_max));
    if (Omega[0]!=omega_min || Omega[Omega.size()-1]!=omega_max)
      boost::throw_exception(std::invalid_argument("invalid omega range for default model"));
  }
  
  double operator()(const double omega) {
    /*if (index>0 && Omega[index-1]>=omega) 
      index=0;
    while (Omega[index]<omega) 
      index++;
    if (index==0)
      return Def[0];
      else {*/
    std::vector<double>::const_iterator ub = std::upper_bound(Omega.begin(), Omega.end(), omega);
    int index = ub - Omega.begin();
    if (ub==Omega.end())
      index = Omega.end()-Omega.begin()-1;
    double om1 = Omega[index-1];
    double om2 = Omega[index];
    double D1 = Def[index-1];
    double D2 = Def[index];
    return -(D2-D1)/(om2-om1)*(om2-omega)+D2;      
    //}
  }
   
private:
  std::vector<double> Omega;
  std::vector<double> Def;
  //int index;
};



class GeneralDefaultModel : public DefaultModel
{
public:
  
  GeneralDefaultModel(const alps::Parameters& p, Model* mod) : DefaultModel(p) , Mod(mod), ntab(5001), 
							       xtab(ntab)//, omega_index(0)
  {
    double sum = 0;
    xtab[0] = 0.;
    double delta_omega = (omega_max-omega_min)/(ntab-1);
    for (int o=1; o<ntab; ++o) {
      double omega1 = omega_min + (o-1)*delta_omega;
      double omega2 = omega_min + o*delta_omega;
      sum += ((*Mod)(omega1)+(*Mod)(omega2))/2.*delta_omega;
      xtab[o] = sum;
    }
    for (int o=0; o<ntab; ++o) {
      xtab[o] *= blow_up()/sum;
    }
    //std::cerr << "dev:" << sum-1 << "\n";
    //for (int o=0; o<ntab; ++o) 
    //std::cout << o << "\t" << xtab[o] << "\n";
  }
  
  double omega(const double x) const {
    assert(x<=blow_up() && x>=0.);
    /*if (omega_index>0 && xtab[omega_index-1]>=x) 
      omega_index=0;
      while (xtab[omega_index]<x) 
      omega_index++; */
    std::vector<double>::const_iterator ub = std::upper_bound(xtab.begin(), xtab.end(), x);
    int omega_index = ub - xtab.begin();
    if (ub==xtab.end())
      omega_index = xtab.end() - xtab.begin() - 1;
    double om1 = omega_min + (omega_index-1)*(omega_max-omega_min)/(ntab-1);
    double om2 = omega_min + omega_index*(omega_max-omega_min)/(ntab-1);
    double x1 = xtab[omega_index-1];
    double x2 = xtab[omega_index];
    return -(om2-om1)/(x2-x1)*(x2-x)+om2;      
  }
  
  double D(const double omega) const {
    return (*Mod)(omega);
  }
  
  double x(const double t) const {
    //std::cout << 1-t << "\n";
    assert(t<=1. && t>=0.);
    int od = (int)(t*(ntab-1));
    if (od==(ntab-1)) 
      return blow_up();
    //double o1 = od;
    //double o2 = od+1;
    double x1 = xtab[od];
    double x2 = xtab[od+1];
    //std::cerr << o1 << "\t" << o2 << "\t" << x1 << "\t" << x2 << "\n";
    return -(x2-x1)*(od+1-t*ntab)+x2;      
  }
  
private:
  Model* Mod;
  const int ntab;
  std::vector<double> xtab;
  //int omega_index;
};



inline DefaultModel* make_default_model(const alps::Parameters& parms, char *name) 
{
  if (!parms.defined(name) || parms[name] == "flat") {
    if (alps::is_master())
      std::cerr << "Using flat default model" << std::endl;
    return new FlatDefaultModel(parms);
  }
  else if (parms[name] == "gaussian") {
    if (alps::is_master())
      std::cerr << "Using Gaussian default model" << std::endl;
    Model* Mod = new Gaussian(parms);
    return new GeneralDefaultModel(parms, Mod);
  }
  else if (parms[name] == "double gaussian") {
    if (alps::is_master())
      std::cerr << "Using double Gaussian default model" << std::endl;
    Model* Mod = new DoubleGaussian(parms);
    return new GeneralDefaultModel(parms, Mod);
  }
  else if (parms[name] == "general double gaussian") {
    if (alps::is_master())
      std::cerr << "Using general double Gaussian default model" << std::endl;
    Model* Mod = new GeneralDoubleGaussian(parms);
    return new GeneralDefaultModel(parms, Mod);

  }
  else { 
    if (alps::is_master())
      std::cerr << "Using tabulated default model" << std::endl;
    Model* Mod = new TabFunction(parms, name);
    return new GeneralDefaultModel(parms, Mod);
  }
}



#endif
