/*****************************************************************************
*
* ALPS Project: Algorithms and Libraries for Physics Simulations
*
* ALPS Libraries
*
* Copyright (C) 1994-2004 by Matthias Troyer <troyer@itp.phys.ethz.ch>,
*                            Beat Ammon <ammon@ginnan.issp.u-tokyo.ac.jp>,
*                            Andreas Laeuchli <laeuchli@itp.phys.ethz.ch>,
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

#ifndef ALPS_ALEA_ABSTRACTSIMPLEOBSERVABLE_H
#define ALPS_ALEA_ABSTRACTSIMPLEOBSERVABLE_H

#include <alps/alea/observable.h>
#include <alps/alea/recordableobservable.h>
#include <alps/alea/output_helper.h>
#include <alps/alea/hdf5.h>

namespace alps {

//=======================================================================
// AbstractSimpleObservable
//
// Observable class interface
//-----------------------------------------------------------------------

template <class T> class SimpleObservableEvaluator;

template <class T>
class AbstractSimpleObservable: public Observable
{
public:
  //@{
  //@name Type members
        
  /// the data type of the observable
  typedef T value_type;

  /// the data type of averages and errors
  typedef typename obs_value_traits<T>::result_type result_type;
    
  typedef typename obs_value_traits<result_type>::slice_iterator slice_iterator;
  /// the count data type: an integral type
  typedef std::size_t count_type;

  /// the data type for autocorrelation times
  typedef typename obs_value_traits<T>::time_type time_type;
  //@}
  
  AbstractSimpleObservable(const std::string& name="") : Observable(name) {}
  
  virtual ~AbstractSimpleObservable() {}
  //@{
  //@name Properties of the observable
                  
  /// the number of measurements
  virtual count_type count() const =0;
  
  /// the mean value
  virtual result_type mean() const =0;
  
  /// the variance
  virtual result_type variance() const { return boost::throw_exception(std::logic_error("No variance provided in observable"));  result_type();}
  
  /// the error
  virtual result_type error() const =0;

  /// is information about the minimum and maximum value available?
  
  virtual bool has_minmax() const { return false;}
  
  /// the minimum value
  virtual value_type min() const { boost::throw_exception(std::logic_error("No min provided in observable")); return value_type();}
  
  /// the maximum value
  virtual value_type max() const { boost::throw_exception(std::logic_error("No max provided in observable")); return value_type();}
  
  /// is autocorrelation information available ? 
  virtual bool has_tau() const { return false;}
    
  /// the autocorrelation time, throws an exception if not available
  virtual time_type tau() const { return boost::throw_exception(std::logic_error("No autocorelation time provided in observable")); time_type();}

  /// is variance  available ? 
  virtual bool has_variance() const { return false;}

  //@}


  // virtual void set_thermalization(uint32_t todiscard) = 0;
  // virtual uint32_t get_thermalization() = 0;
  
  //@{
  //@name binning information
  /// the number of bins
  virtual count_type bin_number() const { return 0;}
  /// the number of measurements per bin
  virtual count_type bin_size() const { return 0;}
  /// the value of a bin
  virtual const value_type& bin_value(count_type) const 
  { boost::throw_exception(std::logic_error("bin_value called but no bins present")); return *(new value_type());}
  /// the number of bins with squared values
  virtual count_type bin_number2() const { return 0;}
  /// the squared value of a bin
  virtual const value_type& bin_value2(count_type) const 
  { boost::throw_exception(std::logic_error("bin_value2 called but no bins present")); return *(new value_type());}

  //@name Slicing of observables
          
  /** slice the data type using a single argument.
      This can easily be extended when needed to more data types. 
      @param s the slice
      @param newname optionally a new name for the slice. Default is the
                     same name as the original observable
      */
       
  template <class S>
  SimpleObservableEvaluator<typename obs_value_slice<T,S>::value_type>
    slice(S s, const std::string& newname="") const;

  template <class S>
  SimpleObservableEvaluator<typename obs_value_slice<T,S>::value_type>
    operator[](S s) const { return slice(s);}
  //@}
#ifndef ALPS_WITHOUT_OSIRIS
  void extract_timeseries(ODump& dump) const;
#endif

  void write_xml(oxstream&, const boost::filesystem::path& = boost::filesystem::path()) const;
  void write_xml_scalar(oxstream&, const boost::filesystem::path&) const;
  void write_xml_vector(oxstream&, const boost::filesystem::path&) const;

  virtual std::string evaluation_method(Target) const { return "";}
  
  operator SimpleObservableEvaluator<value_type> () const { return make_evaluator();}

private:
  virtual SimpleObservableEvaluator<value_type> make_evaluator() const
  { 
    return SimpleObservableEvaluator<value_type>(*this,name());
  }
  friend class SimpleObservableEvaluator<value_type>;
  
  virtual void write_more_xml(oxstream&, slice_iterator = slice_iterator()) const {}
};


//=======================================================================
// Implementations
//=======================================================================

template <class T>
void AbstractSimpleObservable<T>::write_xml(oxstream& oxs, const boost::filesystem::path& fn_hdf5) const
{
  output_helper<obs_value_traits<T>::array_valued>::write_xml(*this, oxs, fn_hdf5);
}

template <class T>
#ifdef ALPS_HAVE_HDF5
void AbstractSimpleObservable<T>::write_xml_scalar(oxstream& oxs, const boost::filesystem::path& fn_hdf5) const
#else
void AbstractSimpleObservable<T>::write_xml_scalar(oxstream& oxs, const boost::filesystem::path&) const
#endif
{
  if (count())
  {
    std::string mm = evaluation_method(Mean);
    std::string em = evaluation_method(Error);
    std::string vm = evaluation_method(Variance);
    std::string tm = evaluation_method(Tau);

    oxs << start_tag("SCALAR_AVERAGE") << attribute("name", name());
    if (is_signed())
      oxs << attribute("signed","true");     

    oxs << start_tag("COUNT") << no_linebreak << count() << end_tag;

    oxs << start_tag("MEAN") << no_linebreak;
    if (mm != "") oxs << attribute("method", mm);
    int prec=int(4-std::log10(std::abs(error()/mean())));
    prec = (prec>=3 && prec<20 ? prec : 8);
    oxs << precision(mean(),prec) << end_tag;

    oxs << start_tag("ERROR") << no_linebreak;
    if (em != "") oxs << attribute("method", em);
    oxs << precision(error(), 3) << end_tag;

    if (has_variance()) {
      oxs << start_tag("VARIANCE") << no_linebreak;
      if (vm != "") oxs << attribute("method", vm);
      oxs << precision(variance(), 3) << end_tag;
    }
    if (has_tau()) {
      oxs << start_tag("AUTOCORR") << no_linebreak;
      if (tm != "") oxs << attribute("method", tm);
      oxs << precision(tau(), 3) << end_tag;
    }

#ifdef ALPS_HAVE_HDF5
    if (!fn_hdf5.empty() && bin_size() == 1) {
      //write tag for timeseries and the hdf5-file
      oxs << start_tag("TIMESERIES") << attribute("format", "HDF5")
          << attribute("file", fn_hdf5.leaf()) << attribute("set", name())
          << end_tag;

      //open the hdf5 file and write data
      H5File hdf5(fn_hdf5.native_file_string().c_str(),H5F_ACC_CREAT | H5F_ACC_RDWR);
      hsize_t dims[1];
      dims[0]=bin_number();
      DataSpace dataspace(1,dims);
      IntType datatype(HDF5Traits<T>::pred_type());
      DataSet dataset=hdf5.createDataSet(name().c_str(),datatype,dataspace);
      vector<T> data(bin_number());
      for(int j=0;j<bin_number();j++) data[j]=bin_value(j);
      dataset.write(&(data[0]),HDF5Traits<T>::pred_type());
    }
#endif
    write_more_xml(oxs);
    oxs << end_tag("SCALAR_AVERAGE");
  }
}

template <class T>
#ifdef ALPS_HAVE_HDF5
void AbstractSimpleObservable<T>::write_xml_vector(oxstream& oxs, const boost::filesystem::path& fn_hdf5) const
#else
void AbstractSimpleObservable<T>::write_xml_vector(oxstream& oxs, const boost::filesystem::path&) const
#endif
{
  if(count())
  {
    std::string mm = evaluation_method(Mean);
    std::string em = evaluation_method(Error);
    std::string vm = evaluation_method(Variance);
    std::string tm = evaluation_method(Tau);
    result_type mean_(mean());
    result_type error_(error());
    result_type variance_;
    result_type tau_;
    if(has_tau())
    {
      obs_value_traits<T>::resize_same_as(tau_,mean_);
      obs_value_traits<T>::copy(tau_,tau());
    }
    if(has_variance())
    {
      obs_value_traits<T>::resize_same_as(variance_,mean_);
      obs_value_traits<T>::copy(variance_,variance());
    }

    oxs << start_tag("VECTOR_AVERAGE")<< attribute("name", name())
        << attribute("nvalues", obs_value_traits<T>::size(mean()));
    if (is_signed())
      oxs << attribute("signed","true");     

    typename obs_value_traits<result_type>::slice_iterator it=obs_value_traits<result_type>::slice_begin(mean_);
    typename obs_value_traits<result_type>::slice_iterator end=obs_value_traits<result_type>::slice_end(mean_);
    while (it!=end)
    {
      oxs << start_tag("SCALAR_AVERAGE")
          << attribute("indexvalue", obs_value_traits<result_type>::slice_name(mean_,it));
      oxs << start_tag("COUNT") << no_linebreak << count() << end_tag;
      int prec=(count()==1) ? 20 : int(4-std::log10(std::abs(obs_value_traits<result_type>::slice_value(error_,it)/obs_value_traits<result_type>::slice_value(mean_,it)))); 
      
      oxs << start_tag("MEAN") << no_linebreak;
      if (mm != "") oxs << attribute("method", mm);
      oxs << precision(obs_value_traits<result_type>::slice_value(mean_, it), prec)
          << end_tag;
      
      oxs << start_tag("ERROR") << no_linebreak;
      if (em != "") oxs << attribute("method", em);
      oxs << precision(obs_value_traits<result_type>::slice_value(error_, it), 3)
          << end_tag;
      
      if (has_variance()) {
        oxs << start_tag("VARIANCE") << no_linebreak;
        if (vm != "") oxs << attribute("method", vm);
        oxs << precision(obs_value_traits<result_type>::slice_value(variance_, it), 3)
            << end_tag;
      }
      if (has_tau()) {
        oxs << start_tag("AUTOCORR") << no_linebreak;
        if (tm != "") oxs << attribute("method", tm);
        oxs << precision(obs_value_traits<time_type>::slice_value(tau_, it), 3)
            << end_tag;
      }
      
#ifdef ALPS_HAVE_HDF5
      if(!fn_hdf5.empty() && bin_size() == 1) {
        //write tag for timeseries and the hdf5-file
        oxs << start_tag("TIMESERIES") << attribute("format", "HDF5")
            << attribute("file", fn_hdf5.leaf()) << attribute("set", name())
            << end_tag;

        //open the hdf5 file and write data
        H5File hdf5(fn_hdf5.native_file_string().c_str(),H5F_ACC_CREAT | H5F_ACC_RDWR);
        hsize_t dims[1];
        dims[0]=bin_number();
        DataSpace dataspace(1,dims);
        IntType datatype(HDF5Traits<T>::pred_type());
        DataSet dataset=hdf5.createDataSet(name().c_str(),datatype,dataspace);
        vector<T> data(bin_number());
        for(int j=0;j<bin_number();j++) data[j]=bin_value(j)[it];
        dataset.write(&(data[0]),HDF5Traits<T>::pred_type());
      }
#endif
      write_more_xml(oxs,it);

      ++it;
      oxs << end_tag("SCALAR_AVERAGE");
    }
    oxs << end_tag("VECTOR_AVERAGE");
  }
}

} // end namespace alps


#include <alps/alea/simpleobseval.h>

namespace alps {

template <class T> template <class S>
inline SimpleObservableEvaluator<typename obs_value_slice<T,S>::value_type> 
AbstractSimpleObservable<T>::slice (S s, const std::string& n) const
{
  if (dynamic_cast<const SimpleObservableEvaluator<T>*>(this)!=0)
    return dynamic_cast<const SimpleObservableEvaluator<T>*>(this)->slice(s,n);
  else
    return SimpleObservableEvaluator<T>(*this).slice(s,n);        
}

} // end namespace alps


#endif // ALPS_ALEA_ABSTRACTSIMPLEOBSERVABLE_H
