/*****************************************************************************
*
* ALPS Project: Algorithms and Libraries for Physics Simulations
*
* ALPS Libraries
*
* Copyright (C) 2001-2005 by Matthias Troyer <troyer@itp.phys.ethz.ch>,
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

#ifndef ALPS_PARSER_PARAMETERS_H
#define ALPS_PARSER_PARAMETERS_H

#include <alps/osiris/dump.h>
#include <alps/osiris/std/string.h>
#include <alps/parser/parser.h>
#include <alps/stringvalue.h>
#include <alps/xml.h>

#include <boost/serialization/collections_save_imp.hpp>
#include <boost/serialization/collections_load_imp.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/string.hpp>
#include <boost/throw_exception.hpp>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

/// \file parameters.h
/// \brief classes to store simulation parameters

namespace alps {

/// \brief a class to store a single parameter value
///
/// the parameter name (key) is stored as a std::string
/// the parameter value is stores as a StringValue.
class Parameter
{
public:
  /// the parameter name (key) is stored as a std::string
  typedef std::string key_type;
  /// the parameter value is stores as a StringValue.
  typedef StringValue value_type;

  /// deault constructor: no name and no value
  Parameter() : key_(), value_() {}
  /// a parameter with a name but no value
  Parameter(const key_type& k) : key_(k), value_() {}
  /// \brief a parameter with a name and value.
  ///
  /// Arbitrary types can be stored. The StringValue constructor will convert 
  /// them to a string using boost::lexical_cast
  template<class U>
  Parameter(const key_type& k, const U& v) : key_(k), value_(v) {}

  /// returns the key (parameter name)
  key_type& key() { return key_; }
  /// returns the key (parameter name)
  const key_type& key() const { return key_; }
  /// returns the value
  value_type& value() { return value_; }
  /// returns the value
  const value_type& value() const { return value_; }

  /// support for Boost serialization
  template<class Archive>
  void serialize(Archive & ar, const unsigned int)
  { ar & key_ & value_; }
  
private:
  key_type key_;
  value_type value_;
};

/// \brief a class storing a set of parameters
///
/// the class acts like an associative array but at the same time remembers the order in which elements were added
class Parameters
{
public:
  /// the key (parameter name) is a string 
  typedef std::string                     key_type;
  /// the parameter value is a String Value, able to store any type in a text representation
  typedef StringValue                     value_type;
  /// the name-value pair is stored as a Parameter
  typedef Parameter                       parameter_type;

  /// the type of container used internally to store the sequential order
  typedef std::vector<parameter_type>     list_type;
  /// an integral type to store the number oif elements
  typedef list_type::size_type            size_type;
  /// the type of container used internally to implment the associative array access
  typedef std::map<key_type, size_type>   map_type;


  /// the pointer type
  typedef parameter_type *            pointer_type;
  /// the const pointer type
  typedef const parameter_type *      const_pointer_type;
  /// the reference type
  typedef parameter_type &            reference_type;
  /// the const reference type
  typedef const parameter_type &      const_reference_type;
  /// \brief the iterator type
  ///
  /// iteration goes in the order of insertion into the class, not alphabetically like in a std::map
  typedef list_type::iterator         iterator;
  /// \brief the const iterator type
  ///
  /// iteration goes in the order of insertion into the class, not alphabetically like in a std::map
  typedef list_type::const_iterator   const_iterator;

  /// an empty container of parameters
  Parameters() {}
  /// parameters read from a text file
  Parameters(std::istream& is) { parse(is); }

  /// read parameters from a text file
  void parse(std::istream& is);

  /// erase all parameters
  void clear() { list_.clear(); map_.clear(); }
  /// the number of parameters
  size_type size() const { return list_.size(); }

  /// does a parameter with the given name exist?
  bool defined(const key_type& k) const { return (map_.find(k) != map_.end());}

  /// accessing parameters by key (name)
  value_type& operator[](const key_type& k) {
    if (defined(k)) {
      return list_[map_.find(k)->second].value();
    } else {
      push_back(k, value_type());
      return list_.rbegin()->value();
    }
  }

  /// accessing parameters by key (name)
  const value_type& operator[](const key_type& k) const {
    if (!defined(k))
      boost::throw_exception(std::runtime_error("parameter " + k + " not defined"));
    return list_[map_.find(k)->second].value();
  }

  /// \brief returns the value or a default
  /// \param k the key (name) of the parameter
  /// \param v the default value
  /// \return if a parameter with the given name \a k exists, its value is returned, otherwise the default v
  value_type value_or_default(const key_type& k, const value_type& v) const {
    return defined(k) ? (*this)[k] : v;
  }

  /// an iterator pointing to the beginning of the parameters
  iterator begin() { return list_.begin(); }
  /// a const iterator pointing to the beginning of the parameters
  const_iterator begin() const { return list_.begin(); }
  /// an iterator pointing past the of the parameters
  iterator end() { return list_.end(); }
  /// a const iterator pointing past the of the parameters
  const_iterator end() const { return list_.end(); }

  /// \brief appends a new parameter to the container
  /// \param p the parameter
  /// \param allow_overwrite indicates whether existing parameters may be overwritten
  /// \throw a std::runtime_error if the parameter key is empty or if it exists already and \a allow_overwrite is false
  void push_back(const parameter_type& p, bool allow_overwrite=false);

  /// \brief appends a new parameter to the container
  /// \param k the parameter key (name)
  /// \param v the parameter value
  /// \param allow_overwrite indicates whether existing parameters may be overwritten
  /// \throw a std::runtime_error if the parameter key \a k is empty or if it exists already and \a allow_overwrite is false
  void push_back(const key_type& k, const value_type& v,
                 bool allow_overwrite=false) {
    push_back(Parameter(k, v),allow_overwrite);
  }

  /// \brief set a parameter value, overwriting any existing value
  Parameters& operator<<(const parameter_type& p) {
    (*this)[p.key()] = p.value();
    return *this;
  }

  /// \brief set parameter values, overwriting any existing value
  Parameters& operator<<(const Parameters& params);
  /// \brief set parameter values, without overwriting existing value
  void copy_undefined(const Parameters& p);

  /// read from an XML file, using the ALPS XML parser
  void read_xml(XMLTag tag, std::istream& xml,bool ignore_duplicates=false);
  /// extractthe contents from the first <PARAMETERS> element in the XML stream
  void extract_from_xml(std::istream& xml);

  BOOST_SERIALIZATION_SPLIT_MEMBER();
  
  /// support for Boost serialization
  template<class Archive>
  inline void save(Archive & ar, const unsigned int)
  {
    boost::serialization::stl::save_collection<Archive,Parameters>(ar,*this); 
  }

  /// support for Boost serialization
  template<class Archive>
  inline void load(Archive & ar, const unsigned int)
  {
    boost::serialization::stl::load_collection<Archive,Parameters,
      boost::serialization::stl::archive_input_seq<Archive,Parameters>,
        boost::serialization::stl::no_reserve_imp<Parameters> >(ar, *this);
  }

private:
  list_type list_;
  map_type map_;
};

} // namespace alps

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
namespace alps {
#endif

/// write parameters in text-form to a std::ostream
std::ostream& operator<<(std::ostream& os, const alps::Parameters& p);

/// parse parameters in text-form from a std::istream
inline std::istream& operator>>(std::istream& is, alps::Parameters& p)
{
  p.parse(is);
  return is;
}

#ifndef ALPS_WITHOUT_OSIRIS

//
// OSIRIS support
//

/// ALPS serialization of a parameter value
inline alps::ODump& operator<<(alps::ODump& od, const alps::Parameter& p)
{ return od << p.key() << static_cast<std::string>(p.value()); }

/// ALPS de-serialization of a parameter value
inline alps::IDump& operator>>(alps::IDump& id, alps::Parameter& p)
{
  std::string k, v;
  id >> k >> v;
  p = alps::Parameter(k, v);
  return id;
}

/// ALPS serialization of parameters
inline alps::ODump& operator<<(alps::ODump& od, const alps::Parameters& p)
{
  od << uint32_t(p.size());
  for (alps::Parameters::const_iterator it = p.begin(); it != p.end(); ++it)
    od << *it;
  return od;
}

/// ALPS de-serialization of parameters
inline alps::IDump& operator>>(alps::IDump& id, alps::Parameters& p)
{
  p.clear();
  uint32_t n(id);
  for (std::size_t i = 0; i < n; ++i) {
    Parameter m;
    id >> m;
    p.push_back(m);
  }
  return id;
}

#endif

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
} // end namespace alps
#endif


//
// XML support
//

namespace alps {

/// \brief Implementation handler of the ALPS XML parser for the Parameter class  
class ParameterXMLHandler : public XMLHandlerBase
{
public:
  ParameterXMLHandler(Parameter& p);

  void start_element(const std::string& name,
                     const XMLAttributes& attributes,
                     xml::tag_type type);
  void end_element(const std::string& name, xml::tag_type type);
  void text(const std::string& text);

private:
  Parameter& parameter_;
};

/// \brief Implementation handler of the ALPS XML parser for the Parameters class  
class ParametersXMLHandler : public CompositeXMLHandler
{
public:
  ParametersXMLHandler(Parameters& p);

protected:
  void start_child(const std::string& name,
                   const XMLAttributes& attributes,
                   xml::tag_type type);
  void end_child(const std::string& name, xml::tag_type type);

private:
  Parameters& parameters_;
  Parameter parameter_;
  ParameterXMLHandler handler_;
};

} // namespace alps


#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
namespace alps {
#endif

/// \brief XML output of a parameter value 
///
/// follows the schema on http://xml.comp-phys.org/
inline alps::oxstream& operator<<(alps::oxstream& oxs,
                                  const alps::Parameter& parameter)
{
  oxs << alps::start_tag("PARAMETER")
      << alps::attribute("name", parameter.key()) << alps::no_linebreak
      << parameter.value().c_str()
      << alps::end_tag("PARAMETER");
  return oxs;
}

/// \brief XML output of parameters 
///
/// follows the schema on http://xml.comp-phys.org/
inline alps::oxstream& operator<<(alps::oxstream& oxs,
                                  const alps::Parameters& parameters)
{
  oxs << alps::start_tag("PARAMETERS");
  alps::Parameters::const_iterator p_end = parameters.end();
  for (alps::Parameters::const_iterator p = parameters.begin(); p != p_end;
       ++p) oxs << *p;
  oxs << alps::end_tag("PARAMETERS");
  return oxs;
}
#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
} // end namespace alps
#endif

#endif // ALPS_PARSER_PARAMETERS_H
