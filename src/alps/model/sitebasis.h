/***************************************************************************
* ALPS++/model library
*
* model/sitebasis.h    the basis classes
*
* $Id$
*
* Copyright (C) 2003-2003 by Matthias Troyer <troyer@comp-phys.org>,
*                            Axel Grzesik <axel@th.physik.uni-bonn.de>
*
* Permission is hereby granted, free of charge, to any person or organization 
* obtaining a copy of the software covered by this license (the "Software") 
* to use, reproduce, display, distribute, execute, and transmit the Software, 
* and to prepare derivative works of the Software, and to permit others
* to do so for non-commerical academic use, all subject to the following:
*
* The copyright notice in the Software and this entire statement, including 
* the above license grant, this restriction and the following disclaimer, 
* must be included in all copies of the Software, in whole or in part, and 
* all derivative works of the Software, unless such copies or derivative 
* works are solely in the form of machine-executable object code generated by 
* a source language processor.

* In any scientific publication based in part or wholly on the Software, the
* use of the Software has to be acknowledged and the publications quoted
* on the web page http://www.alps.org/license/ have to be referenced.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
* FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT 
* SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE 
* FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, 
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
* DEALINGS IN THE SOFTWARE.
*
**************************************************************************/

#ifndef ALPS_MODEL_SITEBASIS_H
#define ALPS_MODEL_SITEBASIS_H

#include <alps/model/quantumnumber.h>
#include <cstddef>
#include <stack>
#include <utility>
#include <vector>

#include <iostream>

namespace alps {

template<class I>
class SiteBasisDescriptor : public std::vector<QuantumNumber<I> >
{
public:
  typedef typename std::vector<QuantumNumber<I> >::const_iterator const_iterator;
  
  SiteBasisDescriptor() : num_states_(0) { }
#ifndef ALPS_WITHOUT_XML
  SiteBasisDescriptor(const XMLTag&, std::istream&);
  void write_xml(std::ostream&, const std::string& = "") const;
#endif

  const std::string& name() const { return name_;}
  bool valid(const std::vector<half_integer<I> >&) const;
  std::size_t num_states() const { if (!valid_ && !evaluate()) boost::throw_exception(std::runtime_error("Cannot evaluate quantum numbers in site basis " +name()));  return num_states_;}
  bool set_parameters(const Parameters&);
  const Parameters& get_parameters() const {  return parms_;}
private:
  mutable bool valid_;
  bool evaluate() const;
  Parameters parms_;
  std::string name_;
  mutable std::size_t num_states_;
};

template <class I>
class StateDescriptor : public std::vector<half_integer<I> > {
public:
  typename std::vector<half_integer<I> >::const_iterator const_iterator;
  StateDescriptor() {}
  StateDescriptor(const std::vector<half_integer<I> >& x) : std::vector<half_integer<I> >(x)  {}
};


template <class I>
class SiteBasisStates : public std::vector<StateDescriptor<I> >
{
public:
  typedef std::vector<StateDescriptor<I> > base_type;
  typedef typename base_type::const_iterator const_iterator;
  typedef typename base_type::value_type value_type;
  typedef typename base_type::size_type size_type;
  SiteBasisStates(const SiteBasisDescriptor<I>& b);
    
  size_type index(const value_type& x) const;
  const SiteBasisDescriptor<I>& basis() const { return basis_;}
  bool check_sort() const;

private:
  SiteBasisDescriptor<I> basis_;
};


// ------------------------------- implementation ----------------------------------

template <class I>
bool SiteBasisDescriptor<I>::valid(const std::vector<half_integer<I> >& x) const
{
  if(!valid_ && !evaluate()) 
    boost::throw_exception(std::runtime_error("Cannot evaluate quantum numbers in site basis " +name()));
  if (size() != x.size())
    return false;
  for (int i=0;i<size();++i)
    if (!(*this)[i].valid(x[i]))
      return false;
  return true;
}

template <class I>
bool SiteBasisDescriptor<I>::set_parameters(const Parameters& p)
{ 
  for (Parameters::iterator it=parms_.begin();it!=parms_.end();++it)
    if (p.defined(it->key())) 
      it->value() = p[it->key()];
  evaluate();
  return valid_;
}

template <class I>
bool SiteBasisDescriptor<I>::evaluate() const
{
  valid_=true;
  Parameters q_parms_=get_parameters();
  for (const_iterator it=begin();it!=end();++it) {
    valid_ = valid_ && const_cast<QuantumNumber<I>&>(*it).set_parameters(q_parms_);
    if(!valid_) break;
    q_parms_[it->name()]=it->min();
  }
  if (valid_ && begin()!=end()) {
    num_states_=1;
    const_iterator rit=end()-1;
    while(const_cast<QuantumNumber<I>&>(*rit).set_parameters(parms_)) {
      if(rit->levels()==half_integer<I>::max()) {
	num_states_=std::numeric_limits<I>::max();
	return true;
      }
      num_states_ *= rit->levels();
      if(rit==begin()) break;
      --rit;
    }
    if( rit!=begin() ) {
      unsigned int n=0;
      typedef std::pair<const_iterator,Parameters> q_pair;
      std::stack<q_pair> s;
      const_iterator it=begin();
      Parameters p=q_parms_;
      const_cast<QuantumNumber<I>&>(*it).set_parameters(p);
      if(it->levels()==std::numeric_limits<I>::max()) {
	num_states_=std::numeric_limits<I>::max();
	return true;
      }      
      for(half_integer<I> q=it->min();q<=it->max();++q) {
	p[it->name()]=q;
	s.push(q_pair(it,p));
      }
      while(!s.empty()) {
	const_iterator it=s.top().first;
	Parameters      p=s.top().second;
	s.pop();
	const_iterator itt=it+1;
	if(itt==rit) {
	  const_cast<QuantumNumber<I>&>(*itt).set_parameters(p);
	  if(itt->levels()==std::numeric_limits<I>::max()) {
	    num_states_=std::numeric_limits<I>::max();
	    return true;
	  }
	  n+=itt->levels();
	}
	else {
	  ++it;
	  const_cast<QuantumNumber<I>&>(*it).set_parameters(p);
	  if(it->levels()==std::numeric_limits<I>::max()) {
	    num_states_=std::numeric_limits<I>::max();
	    return true;
	  }
	  for(half_integer<I> q=it->min();q<=it->max();++q) {
	    p[it->name()]=q;
	    s.push(q_pair(it,p));
	  }
	}
      }
      num_states_ *= n;
    }
  }
  return valid_;
}

template <class I>
typename SiteBasisStates<I>::size_type SiteBasisStates<I>::index(const value_type& x) const
{
  return std::find(begin(),end(),x)-begin();
}

template <class I>
bool SiteBasisStates<I>::check_sort() const
{
  for (int i=0;i<size()-1;++i)
    if ((*this)[i]>=(*this)[i+1])
      return false;
  return true;
}

template <class I>
SiteBasisStates<I>::SiteBasisStates(const SiteBasisDescriptor<I>& b)
 : basis_(b)
{
  if (b.num_states()==std::numeric_limits<I>::max())
    boost::throw_exception(std::runtime_error("Cannot build infinite set of basis states\n"));
  typedef std::pair<typename SiteBasisDescriptor<I>::const_iterator,half_integer<I> > q_pair;
  std::stack<q_pair> s;
  typename SiteBasisDescriptor<I>::const_iterator it=b.begin();
  std::vector<half_integer<I> > quantumnumbers(basis_.size());
  const_cast<QuantumNumber<I>&>(*it).set_parameters(b.get_parameters());
  for(half_integer<I> q=it->max();q>=it->min();--q) 
    s.push(q_pair(it,q));
  while(!s.empty()) {
    it=s.top().first;
    quantumnumbers[it-b.begin()]=s.top().second;
    s.pop();
    if(it==b.end()-1) 
      push_back(quantumnumbers);
    else {
      ++it;
      Parameters p=b.get_parameters();
      for(typename SiteBasisDescriptor<I>::const_iterator qit=b.begin();qit!=it;++qit)
	p[qit->name()]=quantumnumbers[qit-b.begin()];
      const_cast<QuantumNumber<I>&>(*it).set_parameters(p);
      for(half_integer<I> q=it->max();q>=it->min();--q)
	s.push(q_pair(it,q));
    }
  }
  if(!check_sort())
    boost::throw_exception(std::logic_error("Site basis not sorted correctly"));
}

/* template <class I> */
/* SiteBasisStates<I>::SiteBasisStates(const SiteBasisDescriptor<I>& b,int) */
/*  : basis_(b) */
/* { */
/*   if (b.num_states()==std::numeric_limits<I>::max()) */
/*     boost::throw_exception(std::runtime_error("Cannot build infinite set of basis states\n")); */
/*   std::vector<half_integer<I> > quantumnumbers; */
/*   for (int i=0;i<basis_.size();++i) */
/*     quantumnumbers.push_back(basis_[i].min()); */
/*   int i=0; */
/*   if (basis_.valid(quantumnumbers))  */
/*     do { */
/*       push_back(quantumnumbers); */
/*       i=basis_.size()-1; */
/*       while (i>=0) { */
/* 	if(basis_[i].valid(++quantumnumbers[i])) */
/* 	  break; */
/* 	quantumnumbers[i]=basis_[i].min(); */
/* 	--i; */
/*       } */
/*     } while (i<basis_.size()); */
/*   if (!check_sort()) */
/*     boost::throw_exception(std::logic_error("Site basis not sorted correctly")); */
/* } */

#ifndef ALPS_WITHOUT_XML

template <class I>
SiteBasisDescriptor<I>::SiteBasisDescriptor(const XMLTag& intag, std::istream& is)
 : valid_(false)
{
  XMLTag tag(intag);
  name_ = tag.attributes["name"];
  if (tag.type!=XMLTag::SINGLE) {
    tag = parse_tag(is);
    while (tag.name!="/SITEBASIS") {
      if (tag.name=="QUANTUMNUMBER") 
        push_back(QuantumNumber<I>(tag,is));
      else if (tag.name=="PARAMETER") 
        parms_[tag.attributes["name"]]=tag.attributes["default"];
      if (tag.type!=XMLTag::SINGLE)
        tag = parse_tag(is);
      tag = parse_tag(is);
    }
    if (tag.name !="/SITEBASIS")
      boost::throw_exception(std::runtime_error("Illegal tag <" + tag.name + "> in <SITEBASIS> element"));
  }
}

template <class I>
void SiteBasisDescriptor<I>::write_xml(std::ostream& os,  const std::string& prefix) const
{
  os << prefix << "<SITEBASIS name=\"" << name() <<"\">\n";
  for (Parameters::const_iterator it=parms_.begin();it!=parms_.end();++it)
    os << prefix << "  <PARAMETER name=\"" << it->key() << "\" default=\"" << it->value() << "\"/>\n";
  for (const_iterator it=begin();it!=end();++it)
    it->write_xml(os,prefix+"  ");
  os << prefix << "</SITEBASIS>\n";
}

#endif

}

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
namespace alps {
#endif

#ifndef ALPS_WITHOUT_XML

template <class I>
inline std::ostream& operator<<(std::ostream& out, const alps::SiteBasisDescriptor<I>& q)
{
  q.write_xml(out);
  return out;	
}

#endif

template <class I>
std::ostream& operator<<(std::ostream& out, const alps::StateDescriptor<I>& s)
{
  out << "|";
  for (typename alps::StateDescriptor<I>::const_iterator it=s.begin();it!=s.end();++it)
    out << *it << " ";
  out << ">\n";
  return out;	
}

template <class I>
std::ostream& operator<<(std::ostream& out, const alps::SiteBasisStates<I>& s)
{ 
  out << "{\n";
  for (typename alps::SiteBasisStates<I>::const_iterator it=s.begin();it!=s.end();++it) {
    out << "  |";
    for (int i=0;i<s.basis().size();++i)
      out << " " << s.basis()[i].name() << "=" << (*it)[i];
    out << " >\n";
  }
  out << "}\n";
  return out;	
}

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
} // namespace alps
#endif

#endif
