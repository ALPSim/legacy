/***************************************************************************
* ALPS++/alea library
*
* alps/alea/simpleobsdata.h     Monte Carlo observable class
*
* $Id$
*
* Copyright (C) 1994-2003 by Matthias Troyer <troyer@comp-phys.org>,
*                            Beat Ammon <beat.ammon@bluewin.ch>, 
*                            Andreas Laeuchli <laeuchli@comp-phys.org>,
*                            Synge Todo <wistaria@comp-phys.org>,
*                            Andreas Lange <alange@phys.ethz.ch>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
**************************************************************************/

#ifndef ALPS_ALEA_SIMPLEOBSDATA_H
#define ALPS_ALEA_SIMPLEOBSDATA_H

#include <alps/config.h>
#include <alps/functional.h>
#include <alps/alea/simpleobservable.h>
#include <alps/parser/parser.h>

#include <boost/compose.hpp>
#include <iostream>
#include <numeric>
#include <vector>
#ifdef HAVE_VALARRAY
# include <valarray>
#endif

#ifndef ALPS_WITHOUT_OSIRIS
# include <alps/osiris.h>
#endif

#ifdef HAVE_VALARRAY
template <class T> std::ostream& operator<<(std::ostream& o, const std::valarray<T>&) { return o;}
#endif

namespace alps {

//=======================================================================
// SimpleObservableData
//
// Observable class with variable autocorrelation analysis and binning
//-----------------------------------------------------------------------


template <class T>
class SimpleObservableData
{
public:
  typedef T value_type;
  typedef typename obs_value_traits<T>::time_type time_type;
  typedef typename obs_value_traits<T>::size_type size_type;
  typedef typename obs_value_traits<T>::count_type count_type;
  typedef typename obs_value_traits<T>::result_type result_type;

  // constructors
  SimpleObservableData();
  template <class U, class S>
  SimpleObservableData(const SimpleObservableData<U>& x, S s);
  SimpleObservableData(const SimpleObservable<value_type>& obs);
  SimpleObservableData(std::istream&, const XMLTag&);

  // friend SimpleObservableData<typename obs_value_slice<T,typename obs_value_traits<T>::index_type> >;
  // friend SimpleObservableData<double>::SimpleObservableData<double>(SimpleObservableData&,int);

  template <class S>
  SimpleObservableData<typename obs_value_slice<T,S>::value_type> slice(S s) {
    return SimpleObservableData<typename obs_value_slice<T,S>::value_type>(*this, s);
  }

  void read_xml(std::istream&, const XMLTag&);
  void read_xml_scalar(std::istream&, const XMLTag&);
  void read_xml_vector(std::istream&, const XMLTag&);
  
  void set_thermalization(uint32_t todiscard);
  uint32_t get_thermalization() const;
  bool can_set_thermalization() const { return can_set_thermal_ && !nonlinear_operations_;}

  uint32_t count() const { return changed_ ? bin_size()*bin_number() : count_;}
  const result_type& mean() const;
  const result_type& error() const;
  const result_type& variance() const;
  const time_type& tau() const;
  const value_type& min() const;
  const value_type& max() const;
  
  bool has_variance() const { return has_variance_;}
  bool has_tau() const { return has_tau_;}
  bool has_minmax() const { return has_minmax_;}

  uint32_t bin_size() const { return binsize_;}
  uint32_t bin_number() const { return values_.size()-discardedbins_;}
  uint32_t bin_number2() const { return discardedbins_ ? 0 : values2_.size();}
  const value_type& bin_value(uint32_t i) const {
    return values_[i+discardedbins_];
  }
  const value_type& bin_value2(uint32_t i) const {
    return values2_[i+discardedbins_];
  }
  
  template <class S>
    SimpleObservableData<typename obs_value_slice<T,S>::value_type> slice(S s) const
      {
        return SimpleObservableData<typename obs_value_slice<T,S>::value_type>(*this,s);
      }

  void compact();
  
#ifndef ALPS_WITHOUT_OSIRIS
  void extract_timeseries(ODump& dump) const;
  void save(ODump& dump) const;
  void load(IDump& dump);
#endif
 
  void set_bin_size(uint32_t);
  void set_bin_number(uint32_t);
 
  // collect information from many data objects
  void collect_from(const std::vector<SimpleObservableData<T> >& runs);

  // unary operation: neagtion
  void negate();
  
  // operations with constant
  template <class X> SimpleObservableData<T>& operator+=(X);
  template <class X> SimpleObservableData<T>& operator-=(X);
  template <class X> SimpleObservableData<T>& operator*=(X);
  template <class X> SimpleObservableData<T>& operator/=(X);
  template<class X> void subtract_from(const X& x);
  template<class X> void divide(const X& x);
  

  // operations with another observable
  SimpleObservableData<T>& operator+=(const SimpleObservableData<T>&);
  SimpleObservableData<T>& operator-=(const SimpleObservableData<T>&);
  template <class X>
  SimpleObservableData<T>& operator*=(const SimpleObservableData<X>&);
  template <class X>
  SimpleObservableData<T>& operator/=(const SimpleObservableData<X>&);

  template <class OPV, class OPR> void transform(OPV opv, OPR opr);

  std::string evaluation_method(Target t) const;

protected:
  void collect_bins(uint32_t howmany);
  void analyze() const;
  void jackknife() const;
  void fill_jack() const;

  template <class OPV, class OPR, class X>
  void transform(const SimpleObservableData<X>& x, OPV opv, OPR opr);
  template <class OPV, class OPR> void transform_linear(OPV opv, OPR opr);

private:  
  mutable uint32_t count_;          

  mutable bool has_variance_;
  mutable bool has_tau_;
  mutable bool has_minmax_;
  mutable bool can_set_thermal_;

  mutable uint32_t binsize_;
  mutable uint32_t thermalcount_; 
  mutable uint32_t discardedmeas_;
  mutable uint32_t discardedbins_;
    
  bool changed_;
  mutable bool valid_;
  mutable bool jack_valid_;
  bool nonlinear_operations_; // nontrivial operations
    
  mutable result_type mean_;     // valid only if (valid_)
  mutable result_type error_;    // valid only if (valid_)
  mutable result_type variance_; // valid only if (valid_ && has_variance_)
  mutable time_type tau_;        // valid only if (valid_ && has_tau_)
  mutable value_type min_, max_; // valid only if (valid_ && has_minmax_)
  
  mutable std::vector<value_type> values_;
  mutable std::vector<value_type> values2_;
  mutable std::vector<result_type> jack_;

  std::string eval_method_;
};


template <class T>
inline SimpleObservableData<T>::SimpleObservableData()
 : count_(0),
   has_variance_(false),
   has_tau_(false),
   has_minmax_(false),
   can_set_thermal_(false),
   binsize_(0),
   thermalcount_(0),
   discardedmeas_(0),
   discardedbins_(0),
   changed_(false),
   valid_(true),
   jack_valid_(true),
   nonlinear_operations_(false),
   mean_(), error_(), variance_(), tau_(), min_(), max_(),
   values_(), values2_(), jack_()
{}

template <class T>
template <class U, class S>
inline
SimpleObservableData<T>::SimpleObservableData(const SimpleObservableData<U>& x, S s)
 : count_(x.count_),          
   has_variance_(x.has_variance_),
   has_tau_(x.has_tau_),
   has_minmax_(x.has_minmax_),
   can_set_thermal_(x.can_set_thermal_),
   binsize_(x.binsize_),
   thermalcount_(x.thermalcount_),
   discardedmeas_(x.discardedmeas_),
   discardedbins_(x.discardedbins_),
   changed_(x.changed_),
   valid_(x.valid_),
   jack_valid_(x.jack_valid_),
   nonlinear_operations_(x.nonlinear_operations_),
   mean_(), error_(), variance_(), tau_(), min_(), max_(),
   values_(), values2_(), jack_()
{
  if (valid_) {
    mean_ = obs_value_slice<typename obs_value_traits<U>::result_type,S>()(x.mean_, s);
    error_ = obs_value_slice<typename obs_value_traits<U>::result_type,S>()(x.error_, s);
    if (has_variance_)
      variance_ = obs_value_slice<typename obs_value_traits<U>::result_type,S>()(x.error_, s);
    if (has_tau_)
      tau_ = obs_value_slice<typename obs_value_traits<U>::time_type,S>()(x.tau_, s);
  }
  if(has_minmax_) {
    min_ = obs_value_slice<U,S>()(x.min_, s);
    max_ = obs_value_slice<U,S>()(x.max_, s);
  }
  
  std::transform(x.values_.begin(), x.values_.end(), values_.begin(),
                 boost::bind2nd(obs_value_slice<U,S>(),s));
  std::transform(x.values2_.begin(), x.values2_.end(), values2_.begin(),
		 boost::bind2nd(obs_value_slice<U,S>(),s));
  if (jack_valid_)
    std::transform(x.jack_.begin(), x.jack_.end(), jack_.begin(),
		   boost::bind2nd(obs_value_slice<U,S>(),s));
}

template <class T>
inline SimpleObservableData<T>::SimpleObservableData(const SimpleObservable<T>& obs)
 : count_(obs.count()),
   has_variance_(obs.has_variance()),
   has_tau_(obs.has_tau()),
   has_minmax_(obs.has_minmax()),
   can_set_thermal_(obs.can_set_thermalization()),
   binsize_(obs.bin_size()),
   thermalcount_(obs.get_thermalization()),
   discardedmeas_(0),
   discardedbins_(0),
   changed_(false),
   valid_(false),
   jack_valid_(false),
   nonlinear_operations_(false),
   mean_(), error_(), variance_(), tau_(), min_(), max_(),
   values_(), values2_(), jack_()
{
  if (count()) {
    obs_value_traits<result_type>::copy(mean_, obs.mean());
    obs_value_traits<result_type>::copy(error_, obs.error());
    if (has_variance())
      obs_value_traits<result_type>::copy(variance_, obs.variance());
    if (has_tau())
      obs_value_traits<time_type>::copy(tau_, obs.tau());
    if (has_minmax()) {
      obs_value_traits<result_type>::copy(min_, obs.min());
      obs_value_traits<result_type>::copy(max_, obs.max());
    }

    for (int i = 0; i < obs.bin_number(); ++i)
      values_.push_back(obs.bin_value(i));
    for (int i = 0; i < obs.bin_number2(); ++i)
      values2_.push_back(obs.bin_value2(i));
    
    if (bin_size() != 1 && bin_number() > 128) set_bin_number(128);
  }
}

template <class T>
inline SimpleObservableData<T>::SimpleObservableData(std::istream& infile, const XMLTag& intag)
  : count_(0),
    has_variance_(false),
    has_tau_(false),
    has_minmax_(false),
    can_set_thermal_(false),
    binsize_(0),
    thermalcount_(0),
    discardedmeas_(0),
    discardedbins_(0),
    changed_(false),
    valid_(true),
    jack_valid_(false),
    nonlinear_operations_(false),
    mean_(), error_(), variance_(), tau_(), min_(), max_(),
    values_(), values2_(), jack_()
{
  read_xml(infile,intag);
}

template <class T>
inline void SimpleObservableData<T>::read_xml_scalar(std::istream& infile, const XMLTag& intag)
{
  if (intag.name != "SCALAR_AVERAGE")
    boost::throw_exception(std::runtime_error ("Encountered tag <" +intag.name +
 "> instead of <SCALAR_AVERAGE>"));
  if (intag.type ==XMLTag::SINGLE)
    return;

  XMLTag tag = parse_tag(infile);
  while (tag.name !="/SCALAR_AVERAGE") {
    if (tag.name=="COUNT") {
      if (tag.type !=XMLTag::SINGLE) {
        count_ = boost::lexical_cast<std::size_t,std::string>(parse_content(infile));
        check_tag(infile,"/COUNT");
      }
    }
    else if (tag.name=="MEAN") {
      if (tag.type !=XMLTag::SINGLE) {
        mean_ = boost::lexical_cast<double,std::string>(parse_content(infile));
        check_tag(infile,"/MEAN");
      }
    }
    else if (tag.name=="ERROR") {
      if (tag.type !=XMLTag::SINGLE) {
        std::string val=parse_content(infile);
        error_=((val=="NaN" || val=="nan") ? sqrt(-1.) : boost::lexical_cast<double,std::string>(val));
        eval_method_=tag.attributes["method"]; 
        check_tag(infile,"/ERROR");
      }
    }
    else if (tag.name=="VARIANE") {
      if (tag.type !=XMLTag::SINGLE) {
        has_variance_=true;
        variance_ = boost::lexical_cast<double,std::string>(parse_content(infile
));
        check_tag(infile,"/VARIANCE");
      }
    }
    else if (tag.name=="AUTOCORR") {
      if (tag.type !=XMLTag::SINGLE) {
        has_tau_=true;
        std::string val=parse_content(infile);
        tau_=((val=="NaN" || val=="nan") ? sqrt(-1.) : boost::lexical_cast<double,std::string>(val));
        check_tag(infile,"/AUTOCORR");
      }
    }
    else 
      skip_element(infile,tag);
    tag = parse_tag(infile);
  }
}

template <class T>
inline void SimpleObservableData<T>::read_xml_vector(std::istream& infile, const XMLTag& intag)
{
  if (intag.name != "VECTOR_AVERAGE")
    boost::throw_exception(std::runtime_error ("Encountered tag <" + intag.name + "> instead of <VECTOR_AVERAGE>"));
  if (intag.type == XMLTag::SINGLE)
    return;
  XMLTag tag(intag);
  std::size_t s = boost::lexical_cast<std::size_t,std::string>(tag.attributes["nvalues"]);
  obs_value_traits<result_type>::resize(mean_,s);
  obs_value_traits<result_type>::resize(error_,s);
  obs_value_traits<result_type>::resize(variance_,s);
  obs_value_traits<time_type>::resize(tau_,s);
  
  tag = parse_tag(infile);
  int i=0;
  while (tag.name =="SCALAR_AVERAGE") {
    tag = parse_tag(infile);
    while (tag.name !="/SCALAR_AVERAGE") {
      if (tag.name=="COUNT") {
        if (tag.type != XMLTag::SINGLE) {
          count_=boost::lexical_cast<std::size_t,std::string>(parse_content(infile));
          check_tag(infile,"/COUNT");
        }
      }
      else if (tag.name=="MEAN") {
        if (tag.type !=XMLTag::SINGLE) {
          mean_[i]=boost::lexical_cast<double,std::string>(parse_content(infile));
          check_tag(infile,"/MEAN");
        }
      }
      else if (tag.name=="ERROR") {
        if (tag.type != XMLTag::SINGLE) {
          std::string val=parse_content(infile);
          error_[i]=((val=="NaN" || val=="nan") ? sqrt(-1.) : boost::lexical_cast<double,std::string>(val));
          eval_method_=tag.attributes["method"]; 
          check_tag(infile,"/ERROR");
        }
      }
      else if (tag.name=="VARIANE") {
        if (tag.type !=XMLTag::SINGLE) {
          has_variance_=true;
          variance_[i]=boost::lexical_cast<double,std::string>(parse_content(infile));
          check_tag(infile,"/VARIANCE");
        }
      }
      else if (tag.name=="AUTOCORR") {
        if (tag.type !=XMLTag::SINGLE) {
          has_tau_=true;
          std::string val=parse_content(infile);
          tau_[i]=((val=="NaN" || val=="nan") ? 0. : boost::lexical_cast<double,std::string>(val));
          check_tag(infile,"/AUTOCORR");
        }
      }
      else 
        skip_element(infile,tag);
      tag = parse_tag(infile);
    }
    ++i;
    tag = parse_tag(infile);
  }
  if (tag.name!="/VECTOR_AVERAGE")
    boost::throw_exception(std::runtime_error("Encountered unknow tag <"+tag.name+"> in <VECTOR_AVERAGE>"));
}

namespace detail {

template <bool arrayvalued> struct input_helper {};
  
template <> struct input_helper<true>
{
  template <class T>
  static void read_xml(SimpleObservableData<T>& obs, std::istream& infile, const XMLTag& tag) {
    obs.read_xml_vector(infile,tag);
  }
};
  
template <> struct input_helper<false>
{
  template <class T>
  static void read_xml(SimpleObservableData<T>& obs, std::istream& infile, const XMLTag& tag) {
    obs.read_xml_scalar(infile,tag);
  }
};

} // namespace detail

template <class T>
inline void SimpleObservableData<T>::read_xml(std::istream& infile, const XMLTag& intag)
{
  detail::input_helper<obs_value_traits<T>::array_valued>::read_xml(*this,infile,intag);
}

template <class T> 
inline std::string SimpleObservableData<T>::evaluation_method(Target t) const
{
  if (t==Variance)
    return "simple";
  else if (eval_method_!="")
    return eval_method_;
  else if (jack_.size())
    return "jackknife";
  else if (has_tau_)
    return "binning";
  else
    return "simple";
}

template <class T> 
inline SimpleObservableData<T>& SimpleObservableData<T>::operator+=(const SimpleObservableData<T>& x)
{
  using std::sqrt;
  using alps::sqrt;
  if(count() && x.count())
    error_=sqrt(error_*error_+x.error()*x.error());
  transform(x,std::plus<value_type>(),std::plus<result_type>());
  return (*this);
}

template <class T>
inline SimpleObservableData<T>& SimpleObservableData<T>::operator-=(const SimpleObservableData<T>& x)
{
  using std::sqrt;
  using alps::sqrt;
  if(count() && x.count())
    error_=sqrt(error_*error_+x.error()*x.error());
  transform(x,std::minus<value_type>(),std::minus<result_type>());
  return (*this);
}

template <class T>
template<class X>
inline SimpleObservableData<T>& SimpleObservableData<T>::operator*=(const SimpleObservableData<X>& x)
{
  using std::sqrt;
  using alps::sqrt;
  if(count() && x.count())
    error_=sqrt(error()*error()*x.mean()*x.mean()+mean()*mean()*x.error()*x.error());
  transform(x,alps::multiplies<value_type,X,value_type>(), alps::multiplies<
	    result_type, typename SimpleObservableData<X>::result_type,
	    result_type>());
  return (*this);
}

template <class T>
template<class X>
inline SimpleObservableData<T>& SimpleObservableData<T>::operator/=(const SimpleObservableData<X>& x)
{
  using std::sqrt;
  using alps::sqrt;
  if(count() && x.count())
  error_ = sqrt((error()*error()-mean()*mean()*x.error()*x.error()/x.mean()/x.mean())/x.mean()/x.mean());
  transform(x,alps::divides<value_type,X,value_type>(),alps::divides<
  result_type,typename SimpleObservableData<X>::result_type,result_type>());
  return (*this);
}

template <class T>
template <class OPV, class OPR, class X>
inline
void SimpleObservableData<T>::transform(const SimpleObservableData<X>& x,
					OPV opv, OPR opr)
{
  if ((count()==0) || (x.count()==0))
    boost::throw_exception(std::runtime_error("both observables need measurements"));
    
  if(!jack_valid_) fill_jack();
  if(!x.jack_valid_) x.fill_jack();

  valid_ = false;
  nonlinear_operations_ = true;
  changed_ = true;
  mean_ = opr(mean_, x.mean_);
  has_minmax_ = false;
  has_variance_ = false;
  has_tau_=false;
  values2_.clear();
  bool delete_bins = (bin_number() != x.bin_number() ||
		      bin_size() != x.bin_size() ||
		      jack_.size() != x.jack_.size() );
  if (delete_bins) {
    values_.clear();
    jack_.clear();
  } else {
    for (int i = 0; i < bin_number(); ++i)
      values_[i] = opv(values_[i], x.values_[i]);
    for (int i = 0; i < jack_.size(); ++i)
      jack_[i] = opr(jack_[i], x.jack_[i]);
  }
}

template <class T>
inline void SimpleObservableData<T>::compact()
{
  analyze();
  values_.clear();
  values2_.clear();
  jack_.clear();
}

template <class T> template <class OPV, class OPR>
inline void SimpleObservableData<T>::transform_linear(OPV opv, OPR opr)
{
  mean_ = opv(mean_);
  std::transform(values_.begin(), values_.end(), values_.begin(), opv);
  fill_jack();
  std::transform(jack_.begin(), jack_.end(), jack_.begin(), opr);
}

template <class T> template <class OPV, class OPR>
inline void SimpleObservableData<T>::transform(OPV opv, OPR opr)
{
  valid_ = false;
  nonlinear_operations_ = true;
  changed_ = true;
  has_variance_ = false;
  has_tau_ = false;
  values2_.clear();
  has_minmax_ = false;
  std::transform(values_.begin(), values_.end(), values_.begin(), opv);
  fill_jack();
  std::transform(jack_.begin(), jack_.end(), jack_.begin(), opr);
}

template <class T>
inline void SimpleObservableData<T>::negate()
{
  if (count()) {
    if (has_minmax_) {
      value_type tmp(min_);
      min_ = -max_;
      max_ = -tmp;
    }
    transform_linear(std::negate<value_type>(),std::negate<result_type>());
  }
}

template <class T> template <class X>
inline SimpleObservableData<T>& SimpleObservableData<T>::operator+=(X x)
{
  if (count()) {
    if (has_minmax_) {
      min_ += x;
      max_ += x;
    }
    transform_linear(std::bind2nd(alps::plus<value_type,X,value_type>(),x),std::bind2nd(alps::plus<result_type,X,result_type>(),x));
    for (int i=0;i<values2_.size();++i)
      values2_[i] += 2.*values_[i]*x+x*x;
  }
  return *this;
}

template <class T> template <class X>
inline SimpleObservableData<T>& SimpleObservableData<T>::operator-=(X x)
{
  if(count()) {
    if (has_minmax_) {
      min_ -= x;
      max_ -= x;
    }
    transform_linear(std::bind2nd(alps::minus<value_type,X,value_type>(),x),std::bind2nd(alps::minus<result_type,X,result_type>(),x));
    for (int i=0;i<values2_.size();++i)
      values2_[i] += -2.*values_[i]*x+x*x;
  }
  return (*this);
}

template <class T> template <class X>
inline void SimpleObservableData<T>::subtract_from(const X& x)
{
  if (count()) {
    if(has_minmax_) {
      min_ = x-max_;
      max_ = x-min_;
    }
    transform_linear(std::bind1st(alps::minus<X,value_type,value_type>(),x),std::bind1st(alps::minus<X,result_type,result_type>(),x));
    for (int i=0;i<values2_.size();++i)
      values2_[i] += -2.*values_[i]*x+x*x;
  }
}

template <class T> template <class X>
inline SimpleObservableData<T>& SimpleObservableData<T>::operator*=(X x)
{
  if (count()) {
    error_ *= x;
    if(has_variance_)
      variance_ *= x*x;
    has_minmax_ = false;
    
    transform_linear(std::bind2nd(alps::multiplies<value_type,X,value_type>(),x),std::bind2nd(alps::multiplies<result_type,X,result_type>(),x));
    std::transform(values2_.begin(),values2_.end(),values2_.begin(),
                   std::bind2nd(alps::multiplies<value_type,X,value_type>(),x*x));
  }
  return (*this);
}

template <class T> template <class X>
inline SimpleObservableData<T>& SimpleObservableData<T>::operator/=(X x)
{
  if (count()) {
    error_ /= x;
    if(has_variance_)
      variance_ /= x*x;
    has_minmax_ = false;
    
    transform_linear(std::bind2nd(alps::divides<value_type,X,value_type>(),x),std::bind2nd(alps::divides<result_type,X,result_type>(),x));
    std::transform(values2_.begin(),values2_.end(),values2_.begin(),
                   std::bind2nd(alps::divides<value_type,X,value_type>(),x*x));
  }
  return (*this);
}

template <class T> template <class X>
inline void SimpleObservableData<T>::divide(const X& x)
{
  if (count()) {
    error_ = x *error_/mean_/mean_;
    has_minmax_ = false;
    has_variance_ = false;
    values2_.clear();
    has_tau_ = false;
    nonlinear_operations_ = true;
    changed_ = true;
    transform_linear(std::bind1st(alps::divides<X,value_type,value_type>(),x),std::bind1st(alps::divides<X,result_type,result_type>(),x));
  }
}

template <class T>
inline void SimpleObservableData<T>::collect_from(const std::vector<SimpleObservableData<T> >& runs)
{
  bool got_data = false;

  count_ = 0;

  changed_ = false;
  valid_ = false;
  jack_valid_ = false;
  nonlinear_operations_ = false;

  discardedbins_ = 0;
  discardedmeas_ = 0;
  has_variance_ = false;
  has_tau_ = false;
  has_minmax_ = false;

  values_.clear();
  values2_.clear();
  jack_.clear();

  // find smallest and largest bin sizes
  uint32_t minsize = std::numeric_limits<uint32_t>::max();
  uint32_t maxsize = 0;
  for (typename std::vector<SimpleObservableData<T> >::const_iterator
	 r = runs.begin(); r != runs.end(); ++r) {
    if (r->count()) {
      if (r->bin_size() < minsize) minsize = r->bin_size();
      if (r->bin_size() > maxsize) maxsize = r->bin_size();
    }
  }

  binsize_ = maxsize;
  
  for (typename std::vector<SimpleObservableData<T> >::const_iterator
	 r = runs.begin(); r != runs.end(); ++r) {
    if (r->count()) {
      if (!got_data) {
	// initialize
	jack_valid_ = true;

        has_variance_ = r->has_variance_;
        has_tau_ = r->has_tau_;
        has_minmax_ = r->has_minmax_;
        can_set_thermal_ = r->can_set_thermal_;
	nonlinear_operations_ = r->nonlinear_operations_;
        changed_ = r->changed_;
        obs_value_traits<result_type>::copy(mean_,r->mean_);
        obs_value_traits<result_type>::copy(error_,r->error_);
        if (has_minmax_) {
          obs_value_traits<value_type>::copy(min_, r->min_);
          obs_value_traits<value_type>::copy(max_, r->max_);
        }
        if(has_variance_)
          obs_value_traits<result_type>::copy(variance_,r->variance_);
        if(has_tau_)
          obs_value_traits<time_type>::copy(tau_,r->tau_);
        thermalcount_ = r->thermalcount_;
        discardedmeas_ = r->discardedmeas_;
        count_ = r->count();

	if (r->bin_size() == maxsize) {
	  r->fill_jack();
	  values_ = r->values_;
	  values2_ = r->values2_;
	  jack_ = r->jack_;
	} else {
	  SimpleObservableData<T> tmp(*r);
	  tmp.set_bin_size(maxsize);
	  tmp.fill_jack();
	  values_ = tmp.values_;
	  values2_ = tmp.values2_;
	  jack_ = tmp.jack_;
	}
        got_data=true;
      } else {
	// add
	jack_valid_ = false;

        has_variance_ = has_variance_ && r->has_variance_;
        has_tau_ = has_tau_ && r->has_tau_;
        has_minmax_ = has_minmax_ && r->has_minmax_;
        can_set_thermal_ = can_set_thermal_ && r->can_set_thermal_;
	nonlinear_operations_ = nonlinear_operations_ || r->nonlinear_operations_;
        changed_ = changed_ && r->changed_;
        if(has_minmax_) {
          obs_value_traits<value_type>::check_for_min(min_, r->min_);
          obs_value_traits<value_type>::check_for_max(max_, r->max_);
	}
        mean_ = (double(count_)*mean_+double(r->count_)*r->mean_)
                / double(count_ + r->count_);
        using std::sqrt;
        using alps::sqrt;
        error_ = sqrt(double(count_)*double(count_)*error_*error_
		      +double(r->count_)*double(r->count_)*r->error_*r->error_)
	  / double(count_ + r->count_);
        if(has_variance_)
          variance_ = (double(count_)*variance_+double(r->count_)*r->variance_)
	    / double(count_ + r->count_);
        if(has_tau_)
          tau_ = (double(count_)*tau_+double(r->count_)*r->tau_)
	    / double(count_ + r->count_);
        if(has_minmax_) {
	  obs_value_traits<value_type>::check_for_min(min_,r->min_);
          obs_value_traits<value_type>::check_for_max(max_,r->max_);
        }

        thermalcount_ = std::min(thermalcount_, r->thermalcount_);
        discardedmeas_ = std::min(discardedmeas_, r->discardedmeas_);
        count_ += r->count();

	if (r->bin_size() == maxsize) {
	  std::copy(r->values_.begin(), r->values_.end(),
		    std::back_inserter(values_));
	  std::copy(r->values2_.begin(), r->values2_.end(),
		    std::back_inserter(values2_));
	} else {
	  SimpleObservableData<T> tmp(*r);
	  tmp.set_bin_size(maxsize);
	  std::copy(tmp.values_.begin(), tmp.values_.end(),
		    std::back_inserter(values_));
	  std::copy(tmp.values2_.begin(), tmp.values2_.end(),
		    std::back_inserter(values2_));
	}
      }
    }
  }

  analyze();
}

#ifndef ALPS_WITHOUT_OSIRIS

template <class T>
inline void SimpleObservableData<T>::extract_timeseries(ODump& dump) const
{
  dump << binsize_ << uint32_t(values_.size()) << binsize_ << values_;
}

template <class T>
inline void SimpleObservableData<T>::save(ODump& dump) const
{
  dump << count_ << mean_ << error_ << variance_ << tau_ << has_variance_
       << has_tau_ << has_minmax_ << thermalcount_ << can_set_thermal_ << min_ << max_
       << binsize_ << discardedmeas_ << discardedbins_ << valid_ << jack_valid_ << changed_
       << nonlinear_operations_ << values_ << values2_ << jack_;
}

template <class T>
inline void SimpleObservableData<T>::load(IDump& dump)
{
  dump >> count_ >> mean_ >> error_ >> variance_ >> tau_ >> has_variance_
       >> has_tau_ >> has_minmax_ >> thermalcount_ >> can_set_thermal_ >> min_ >> max_
       >> binsize_ >> discardedmeas_ >> discardedbins_ >> valid_ >> jack_valid_ >> changed_
       >> nonlinear_operations_ >> values_ >> values2_ >> jack_;
}

#endif

template <class T>
void SimpleObservableData<T>::fill_jack() const
{
  // build jackknife data structure
  if (bin_number() && !jack_valid_) {
    if (nonlinear_operations_)
      boost::throw_exception(std::runtime_error("Cannot rebuild jackknife data structure after nonlinear operations"));
    jack_.clear();
    jack_.resize(bin_number() + 1);

#ifndef PALM_OLD_JACKKNIFE
    // Order-N initialization of jackknife data structure
    obs_value_traits<result_type>::resize_same_as(jack_[0], bin_value(0));
    for(uint32_t i = 0; i < bin_number(); ++i)
      jack_[0] += obs_value_cast<result_type,value_type>(bin_value(i)) / count_type(bin_size());
    for(uint32_t i = 0; i < bin_number(); ++i) {
      obs_value_traits<result_type>::resize_same_as(jack_[i+1], jack_[0]);
      jack_[i+1] = jack_[0]
	- (obs_value_cast<result_type,value_type>(bin_value(i)) / count_type(bin_size()));
      jack_[i+1] /= count_type(bin_number() - 1);
    }
    jack_[0] /= count_type(bin_number());
#else
    // Original order-N^2 initialization
    for(int32_t i=0;i<bin_number()+1;++i) {
      obs_value_traits<result_type>::resize_same_as(jack_[i],bin_value(0));
      for(int32_t j(0);j<bin_number();++j){
	if(j+1!=i)
	  jack_[i]+=obs_value_cast<result_type,value_type>(bin_value(j)) / count_type(bin_size());
      }
    }
    jack_[0]/=count_type(bin_number());
    for(uint32_t j = 1; j < jack_.size(); ++j)
      jack_[j]/=count_type(bin_number()-1);
#endif
  }
  jack_valid_ = true;
}

template <class T>
void SimpleObservableData<T>::analyze() const
{
  if (valid_) return;

  if (bin_number())
  {
    count_ = bin_size()*bin_number();

    // calculate mean and error
    jackknife();

    // calculate variance and tau
    if (!values2_.empty()) {
      has_variance_ = true;
      has_tau_ = true;
      obs_value_traits<result_type>::resize_same_as(variance_, bin_value2(0));
      variance_ = 0.;
      for (int i=0;i<values2_.size();++i)
        variance_+=obs_value_cast<result_type,value_type>(values2_[i]);
      // was: variance_ = std::accumulate(values2_.begin(), values2_.end(), variance_);
      variance_ -= mean_ * mean_ * count_type(count());
      variance_ /= count_type(count()-1);
      obs_value_traits<result_type>::resize_same_as(tau_, error_);
      tau_ = 0.5*(error_*error_*count_type(count())/variance_-1.);
    } else {
      has_variance_ = false;
      has_tau_ = false;
    }
  }
  valid_ = true;
}

template <class T>
inline void SimpleObservableData<T>::jackknife() const
{
  fill_jack();

  if (jack_.size()) {
    result_type rav;
    obs_value_traits<result_type>::resize_same_as(mean_, jack_[0]);  
    obs_value_traits<result_type>::resize_same_as(error_, jack_[0]);  
    obs_value_traits<result_type>::resize_same_as(rav, jack_[0]);  
    uint32_t k = jack_.size()-1;

    rav = 0;
    rav = std::accumulate(jack_.begin()+1, jack_.end(), rav);
    rav /= count_type(k);
    mean_ = jack_[0] - (rav - jack_[0]) * count_type(k - 1);

    error_ = 0.0;
    for (uint32_t i = 1; i < jack_.size(); ++i)
      error_ += jack_[i] * jack_[i];
    
    error_ = (error_ / count_type(k) - rav * rav);
    error_ *= count_type(k - 1);
    using std::sqrt;
    using alps::sqrt;
    error_ = sqrt(error_);
  }
}


template <class T>
inline uint32_t SimpleObservableData<T>::get_thermalization() const
{
  return thermalcount_ + discardedmeas_;
}

template <class T>
inline void SimpleObservableData<T>::set_thermalization(uint32_t thermal)
{
  if (nonlinear_operations_)
    boost::throw_exception(std::runtime_error("cannot set thermalization after nonlinear operations"));
  if (!can_set_thermalization())
    boost::throw_exception(std::runtime_error("cannot set thermalization"));
  if (binsize_) {
    discardedmeas_ = thermal - thermalcount_;
    discardedbins_ = (discardedmeas_ + binsize_ - 1) / binsize_;  
    changed_ = true;
    valid_ = false;
    jack_valid_ = false;
  }
}

template <class T>
inline void SimpleObservableData<T>::collect_bins(uint32_t howmany)
{
  if (nonlinear_operations_)
    boost::throw_exception(std::runtime_error("cannot change bins after nonlinear operations"));
  if (values_.empty() || howmany <= 1) return;
    
  uint32_t newbins = values_.size() / howmany;
  
  // fill bins
  for (uint32_t i = 0; i < newbins; ++i) {
    values_[i] = values_[howmany * i];
    if (!values2_.empty()) values2_[i] = values2_[howmany * i];
    for (uint32_t j = 1; j < howmany; ++j) {
      values_[i] += values_[howmany * i + j];
      if (!values2_.empty()) values2_[i] += values2_[howmany * i + j];
    }
  }
  
  binsize_ *= howmany;
  discardedbins_ = (discardedmeas_ + binsize_ - 1) / binsize_;

  values_.resize(newbins);
  if (!values2_.empty()) values2_.resize(newbins);
  
  changed_ = true;
  jack_valid_ = false;
  valid_ = false;
}

template <class T>
inline void SimpleObservableData<T>::set_bin_size(uint32_t s)
{
  collect_bins((s-1)/binsize_+1);
  binsize_=s;
}

template <class T>
inline void SimpleObservableData<T>::set_bin_number(uint32_t binnum)
{
  collect_bins((values_.size()-1)/binnum+1);
}

template <class T>
inline
const typename SimpleObservableData<T>::result_type& SimpleObservableData<T>::mean() const
{
  if (count() == 0) boost::throw_exception(NoMeasurementsError());
  analyze();
  return mean_;
}

template <class T>
inline
const typename SimpleObservableData<T>::result_type& SimpleObservableData<T>::error() const
{
  if (count() == 0) boost::throw_exception(NoMeasurementsError());
  analyze();
  return error_;
}

template <class T>
inline const typename SimpleObservableData<T>::result_type& SimpleObservableData<T>::variance() const
{
  if (count() == 0) boost::throw_exception(NoMeasurementsError());
  if (!has_variance_)
    boost::throw_exception(std::logic_error("observable does not have variance"));
  analyze();
  return variance_;
}

template <class T>
inline
const typename SimpleObservableData<T>::time_type& SimpleObservableData<T>::tau() const
{
  if (count() == 0) boost::throw_exception(NoMeasurementsError());
  if (!has_tau_)
    boost::throw_exception(std::logic_error("observable does not have autocorrelation information"));
  analyze();
  return tau_;
}

template <class T>
inline
const typename SimpleObservableData<T>::value_type& SimpleObservableData<T>::min() const
{
  if (count() == 0) boost::throw_exception(NoMeasurementsError());
  if (!has_minmax_)
    boost::throw_exception(std::logic_error("observable does not have minimum"));
  return min_;
}

template <class T>
inline
const typename SimpleObservableData<T>::value_type& SimpleObservableData<T>::max() const
{
  if (count() == 0) boost::throw_exception(NoMeasurementsError());
  if(!has_minmax_)
    boost::throw_exception(std::logic_error("observable does not have maximum"));
  return max_;
}

} // end namespace alps


//
// OSIRIS support
//

#ifndef ALPS_WITHOUT_OSIRIS

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
namespace alps {
#endif

template<class T>
inline alps::ODump& operator<<(alps::ODump& od, const alps::SimpleObservableData<T>& m)
{ m.save(od); return od; }

template<class T>
inline alps::IDump& operator>>(alps::IDump& id, alps::SimpleObservableData<T>& m)
{ m.load(id); return id; }

#endif // !ALPS_WITHOUT_OSIRIS

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
} // namespace alps
#endif

#endif // PALM_ALEA_SIMPLEOBSDATA_H
