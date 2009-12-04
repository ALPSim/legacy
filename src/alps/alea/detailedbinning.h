/*****************************************************************************
*
* ALPS Project: Algorithms and Libraries for Physics Simulations
*
* ALPS Libraries
*
* Copyright (C) 1994-2009 by Matthias Troyer <troyer@comp-phys.org>,
*                            Beat Ammon <ammon@ginnan.issp.u-tokyo.ac.jp>,
*                            Andreas Laeuchli <laeuchli@comp-phys.org>,
*                            Synge Todo <wistaria@comp-phys.org>
*
* This software is part of the ALPS libraries, published under the ALPS
* Library License; you can use, redistribute it and/or modify it under
* the terms of the license, either version 1 or (at your option) any later
* version.
* 
* You should have received a copy of the ALPS Library License along with
* the ALPS Libraries; see the file LICENSE.txt. If not, the license is also
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

/* $Id$ */

#ifndef ALPS_ALEA_DETAILEDBINNING_H
#define ALPS_ALEA_DETAILEDBINNING_H

#include <alps/config.h>
#include <alps/alea/observable.h>
#include <alps/alea/simpleobservable.h>
#include <alps/alea/simplebinning.h>
#include <boost/config.hpp>

#ifdef ALPS_HAVE_VALARRAY
# include <valarray>
#endif

//=======================================================================
// DetailedBinning
//
// detailed binning strategy
//-----------------------------------------------------------------------

namespace alps{

template <class T=double>
class BasicDetailedBinning : public SimpleBinning<T> {
public:
  typedef T value_type;  
  typedef typename obs_value_traits<T>::time_type time_type;
  typedef typename obs_value_traits<T>::size_type size_type;
  typedef typename obs_value_traits<T>::count_type count_type;
  typedef typename obs_value_traits<T>::result_type result_type;

  BOOST_STATIC_CONSTANT(bool, has_tau=true);
  BOOST_STATIC_CONSTANT(int, magic_id=3);

  BasicDetailedBinning(uint32_t binsize=1, uint32_t binnum=std::numeric_limits<uint32_t>::max BOOST_PREVENT_MACRO_SUBSTITUTION ());

  void reset(bool=false);
  void operator<<(const T& x);

  
  uint32_t max_bin_number() const { return maxbinnum_;}
  uint32_t bin_number() const;
  uint32_t filled_bin_number() const;
  uint32_t filled_bin_number2() const 
  { return(values2_.size() ? filled_bin_number() : 0);}
  
  void set_bin_number(uint32_t binnum);
  void collect_bins(uint32_t howmany);
  
  uint32_t bin_size() const { return binsize_;}
  void set_bin_size(uint32_t binsize);

  const value_type& bin_value(uint32_t i) const { return values_[i];}
  const value_type& bin_value2(uint32_t i) const { return values2_[i];}
  
  void compact();
  
#ifdef ALPS_HAVE_HDF5
  template<typename E> void read_hdf5 (const E &engine);
  template<typename E> void write_hdf5 (E &engine) const;
#endif  
#ifndef ALPS_WITHOUT_OSIRIS
  virtual void save(ODump& dump) const;
  virtual void load(IDump& dump);
  void extract_timeseries(ODump& dump) const { dump << binsize_ << values_.size() << binentries_ << values_;}
#endif

#ifdef ALPS_HAVE_HDF5
	void serialize(hdf5oarchive & ar) const;
#endif

private:
  uint32_t binsize_;       // number of measurements per bin
  uint32_t minbinsize_;    // minimum number of measurements per bin
  uint32_t maxbinnum_;      // maximum number of bins 
  uint32_t  binentries_; // number of measurements in last bin
  std::vector<value_type> values_; // bin values
  std::vector<value_type> values2_; // bin values of squares
};

#ifndef BOOST_NO_INCLASS_MEMBER_INITIALIZATION
template <class T> const bool BasicDetailedBinning<T>::has_tau;
#endif

template<class T> class DetailedBinning : public BasicDetailedBinning<T>
{
public:
  typedef T value_type;
  BOOST_STATIC_CONSTANT(int, magic_id=4);
  DetailedBinning(uint32_t binnum=128) 
  : BasicDetailedBinning<T>(1,binnum==0 ? 128 : binnum) {}
};

template<class T> class FixedBinning : public BasicDetailedBinning<T>
{
public:
  typedef T value_type;
  BOOST_STATIC_CONSTANT(int, magic_id=5);
  FixedBinning(uint32_t binsize=1) 
  : BasicDetailedBinning<T>(binsize,std::numeric_limits<uint32_t>::max BOOST_PREVENT_MACRO_SUBSTITUTION ()) {}
};

typedef SimpleObservable<int32_t,DetailedBinning<int32_t> > IntObservable;
typedef SimpleObservable<double,DetailedBinning<double> > RealObservable;
typedef SimpleObservable<float,DetailedBinning<float> > FloatObservable;
typedef SimpleObservable<std::complex<double>,DetailedBinning<std::complex<double> > > ComplexObservable;
typedef SimpleObservable<double,FixedBinning<double> > RealTimeSeriesObservable;
typedef SimpleObservable<int32_t,FixedBinning<int32_t> > IntTimeSeriesObservable;

#ifdef ALPS_HAVE_VALARRAY
typedef SimpleObservable< std::valarray<int32_t> , 
                         DetailedBinning<std::valarray<int32_t> > > IntVectorObservable;
typedef SimpleObservable< std::valarray<double> , 
                         DetailedBinning<std::valarray<double> > > RealVectorObservable;
typedef SimpleObservable< std::valarray<float> , 
                         DetailedBinning<std::valarray<float> > > FloatVectorObservable;
//typedef SimpleObservable< std::valarray<std::complex<double> > , 
//                         DetailedBinning<std::valarray<std::complex<double> > > > ComplexVectorObservable;
typedef SimpleObservable< std::valarray<int32_t> , 
                         FixedBinning<std::valarray<int32_t> > > IntVectorTimeSeriesObservable;
typedef SimpleObservable< std::valarray<double> , 
                         FixedBinning<std::valarray<double> > > RealVectorTimeSeriesObservable;
//typedef SimpleObservable< std::valarray<std::complex<double> > , 
//                         FixedBinning<std::valarray<std::complex<double> > > > ComplexVectorTimeSeriesObservable;
#endif


template <class T>
inline BasicDetailedBinning<T>::BasicDetailedBinning(uint32_t binsize, uint32_t binnum)
 : SimpleBinning<T>(),
   binsize_(0), minbinsize_(binsize), maxbinnum_(binnum), binentries_(0)    
{
  reset();
}

template <class T>
inline void BasicDetailedBinning<T>::compact()
{
  std::vector<value_type> tmp1;
  std::vector<value_type> tmp2;
  values_.swap(tmp1);
  values2_.swap(tmp2);
  binsize_=minbinsize_;
  binentries_=0;
}


template <class T>
inline void BasicDetailedBinning<T>::reset(bool forthermal)
{
  compact();
  SimpleBinning<T>::reset(forthermal);
}


template <class T>
inline void BasicDetailedBinning<T>::operator<<(const T& x)
{
  if (values_.empty())
  { 
    // start first bin
    values_.push_back(x);
    values2_.push_back(x*x);
    binentries_ = 1;
    binsize_=1;
  }
  else if (values_.size()==1 && binentries_ < minbinsize_)
  {
    // fill first bin
    values_[0]+=x;
    values2_[0]+=x*x;
    binentries_++;
    binsize_++;
  }
  else if (binentries_==binsize_) // have a full bin
  {
    if(values_.size()<maxbinnum_)
    {
      // start a new bin
      values_.push_back(x);
      values2_.push_back(x*x);
      binentries_ = 1;
    }
    else
    {
      // halve the bins and add
      collect_bins(2);
      *this << x; // and just call again
      return;
    }
  }
  else
  {
    values_[values_.size()-1] += x;
    values2_[values_.size()-1] += x*x;
    ++binentries_;
  }
  SimpleBinning<T>::operator<<(x);
}

template <class T>
void BasicDetailedBinning<T>::collect_bins(uint32_t howmany)
{
  if (values_.empty() || howmany<=1)
    return;
    
  uint32_t newbins = (values_.size()+howmany-1)/howmany;
  
  // full bins
  for (uint32_t i=0;i<values_.size()/howmany;++i)
  {
    if(howmany*i !=i){
      values_[i]=values_[howmany*i];
      values2_[i]=values2_[howmany*i];
    }
    for (uint32_t j = 1 ; j<howmany;++j)
    {
      values_[i] += values_[howmany*i+j];
      values2_[i] += values2_[howmany*i+j];
    }
  }
  
  // last part of partly full bins
  values_[newbins-1]=values_[howmany*(newbins-1)];
  values2_[newbins-1]=values2_[howmany*(newbins-1)];
  for ( uint32_t i=howmany*(newbins-1)+1;i<values_.size();++i){
    values_[newbins-1]+=values_[i];
    values2_[newbins-1]+=values2_[i];
  }
    
  // how many in last bin?
  binentries_+=((values_.size()-1)%howmany)*binsize_;
  binsize_*=howmany;

  values_.resize(newbins);
  values2_.resize(newbins);
}

template <class T>
void BasicDetailedBinning<T>::set_bin_size(uint32_t minbinsize)
{
  minbinsize_=minbinsize;
  if(binsize_ < minbinsize_ && binsize_ > 0)
    collect_bins((minbinsize-1)/binsize_+1);
}

template <class T>
void BasicDetailedBinning<T>::set_bin_number(uint32_t binnum)
{
  maxbinnum_=binnum;
  if(values_.size() > maxbinnum_)
    collect_bins((values_.size()-1)/maxbinnum_+1);
}

template <class T>
inline uint32_t BasicDetailedBinning<T>::bin_number() const 
{ 
  return values_.size();
}

template <class T>
inline uint32_t BasicDetailedBinning<T>::filled_bin_number() const 
{ 
  if(values_.size()==0) return 0;
  else return values_.size() + (binentries_ ==binsize_ ? 0 : -1);
}

#ifndef ALPS_WITHOUT_OSIRIS
template <class T>
inline void BasicDetailedBinning<T>::save(ODump& dump) const
{
  SimpleBinning<T>::save(dump);
  dump << binsize_ << minbinsize_ << maxbinnum_<< binentries_ << values_
       << values2_;
}

template <class T>
inline void BasicDetailedBinning<T>::load(IDump& dump) 
{
  SimpleBinning<T>::load(dump);
  dump >> binsize_ >> minbinsize_ >> maxbinnum_ >> binentries_ >> values_
       >> values2_;
}
#endif

#ifdef ALPS_HAVE_HDF5
	template <class T> inline void BasicDetailedBinning<T>::serialize(hdf5oarchive & ar) const {
		SimpleBinning<T>::serialize(ar);
//		ar << make_pvp("timeseries/values", values_) << make_pvp("timeseries/values/@binsize", binsize_);
		
//		std::cout << "detailedbinning: " << values_.size() << ", binsize: " << binsize_ << ", minbinsize: " << minbinsize_ << ", maxbinnum: " << maxbinnum_ << ", binentries: " << binentries_ << std::endl;
/*		

dump << binsize_ << minbinsize_ << maxbinnum_<< binentries_ << values_ << values2_;


	// TODO: HDF5 -> write to timeseries

// TODO: move to detailed binning
			ar << make_pvp("bins/bin_size", bin_size()) << make_pvp("bins/number", bin_number()) << make_pvp("bins2/value", bin_number2());
			for(std::size_t i = 0; i < bin_number(); ++i)
				// TODO: write as vector data
				ar << make_pvp("mean/bins/" + boost::lexical_cast<std::string>(i) + "/value", bin_value(i))
				   << make_pvp("mean/bins2/" + boost::lexical_cast<std::string>(i) + "/value", bin_value2(i));
*/
	}
#endif


} // end namespace alps

#ifndef ALPS_WITHOUT_OSIRIS

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
namespace alps {
#endif

template<class T>
inline alps::ODump& operator<<(alps::ODump& od, const alps::BasicDetailedBinning<T>& m)
{ m.save(od); return od; }

template<class T>
inline alps::IDump& operator>>(alps::IDump& id, alps::BasicDetailedBinning<T>& m)
{ m.load(id); return id; }

template<class T>
inline alps::ODump& operator<<(alps::ODump& od, const alps::DetailedBinning<T>& m)
{ m.save(od); return od; }

template<class T>
inline alps::IDump& operator>>(alps::IDump& id, alps::DetailedBinning<T>& m)
{ m.load(id); return id; }

template<class T>
inline alps::ODump& operator<<(alps::ODump& od, const alps::FixedBinning<T>& m)
{ m.save(od); return od; }

template<class T>
inline alps::IDump& operator>>(alps::IDump& id, alps::FixedBinning<T>& m)
{ m.load(id); return id; }

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
} // namespace alps
#endif

#endif

#endif // ALPS_ALEA_DETAILEDBINNING_H
