/*****************************************************************************
*
* ALPS Project: Algorithms and Libraries for Physics Simulations
*
* ALPS Libraries
*
* Copyright (C) 2003-2004 by Matthias Troyer <troyer@comp-phys.org>,
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

#ifndef ALPS_MODEL_SITETERM_H
#define ALPS_MODEL_SITETERM_H

#include <alps/model/operator.h>
#include <alps/expression.h>
#include <alps/multi_array.hpp>
#include <alps/parameters.h>

namespace alps {

template<class I>
class SiteTermDescriptor
{
public:
  typedef std::map<std::string,OperatorDescriptor<I> > operator_map;

  SiteTermDescriptor() : type_(-2) {}
  SiteTermDescriptor(const std::string& t) : type_(-2), term_(t) {}
  SiteTermDescriptor(const Term& t) : type_(-2),
    term_(boost::lexical_cast<std::string>(t)) {}
  SiteTermDescriptor(const XMLTag&, std::istream&);

  void write_xml(oxstream&) const;

  bool match_type(int type) const { return type_==-1 || type==type_;}
  const std::string& term() const { return term_;}
  template <class T>
  boost::multi_array<T,2> matrix(const SiteBasisDescriptor<I>&, const operator_map&,
                                          const Parameters& =Parameters()) const;
private:
  int type_;
  std::string term_;
};


template <class I, class STATE=site_state<I> >
class SiteOperatorEvaluator : public OperatorEvaluator<I>
{
public:
  typedef typename OperatorEvaluator<I>::operator_map operator_map;
  typedef STATE state_type;

  SiteOperatorEvaluator(const state_type& s, const SiteBasisDescriptor<I>& b,
                        const Parameters& p, const operator_map& o)
    : OperatorEvaluator<I>(p,o), state_(s), basis_(b) {}
  bool can_evaluate(const std::string&) const;
  Expression partial_evaluate(const std::string& name) const;
  const state_type& state() const { return state_;}
private:
  mutable state_type state_;
  const SiteBasisDescriptor<I>& basis_;
};


template <class I, class STATE>
bool SiteOperatorEvaluator<I,STATE>::can_evaluate(const std::string& name) const
{
  if (ops_.find(name) != ops_.end()) {
    SiteOperatorEvaluator<I,STATE> eval(*this);
    return eval.partial_evaluate(name).can_evaluate(ParameterEvaluator(*this));
  }
  else
    return ParameterEvaluator::can_evaluate(name);
}

template <class I,class STATE>
Expression SiteOperatorEvaluator<I,STATE>::partial_evaluate(const std::string& name) const
{
  typename operator_map::const_iterator op = ops_.find(name);
  if (op!=ops_.end()) {  // evaluate operator
    Expression e;
    boost::tie(state_,e) = op->second.apply(state_,basis_,ParameterEvaluator(*this));
    return e;
  }
  else
    return OperatorEvaluator<I>::partial_evaluate(name);
}


template <class I, class T>
boost::multi_array<T,2> get_matrix(T,const SiteTermDescriptor<I>& m, const SiteBasisDescriptor<I>& basis1, const typename SiteTermDescriptor<I>::operator_map& ops, const Parameters& p=Parameters())
{
  return m.template matrix<T>(basis1,ops,p);
}


template <class I> template <class T> boost::multi_array<T,2>
SiteTermDescriptor<I>::matrix(const SiteBasisDescriptor<I>& b, const operator_map& ops, const Parameters& p) const
{
  SiteBasisDescriptor<I> basis(b);
  basis.set_parameters(p);
  Parameters parms(p);
  parms.copy_undefined(basis.get_parameters());
  std::size_t dim=basis.num_states();
  boost::multi_array<T,2> mat(boost::extents[dim][dim]);
  // parse expression and store it as sum of terms
  alps::Expression ex(term());
  ex.flatten();

  // fill the matrix
  if (basis.size()==1) {
    typedef single_qn_site_state<I> state_type;
    site_basis<I,state_type> states(basis);
    for (int i=0;i<states.size();++i) {
      //calculate expression applied to state *it and store it into matrix
      for (alps::Expression::term_iterator tit = ex.terms().first; tit !=ex.terms().second; ++tit) {
        SiteOperatorEvaluator<I,state_type> evaluator(states[i],basis,parms,ops);
        Term term(*tit);
        term.partial_evaluate(evaluator);
        int j=states.index(evaluator.state());
        if (boost::lexical_cast<std::string,Term>(term)!="0")
          mat[i][j]+=term;
      }
    }
  }
  else {
    site_basis<I> states(basis);
    for (int i=0;i<states.size();++i) {
    //calculate expression applied to state *it and store it into matrix
      for (alps::Expression::term_iterator tit = ex.terms().first; tit !=ex.terms().second; ++tit) {
        SiteOperatorEvaluator<I> evaluator(states[i],basis,parms,ops);
        Term term(*tit);
        term.partial_evaluate(evaluator);
        int j=states.index(evaluator.state());
        if (boost::lexical_cast<std::string,Term>(term)!="0")
          mat[i][j]+=term;
      }
    }
  }
  return mat;
}

#ifndef ALPS_WITHOUT_XML

template <class I>
SiteTermDescriptor<I>::SiteTermDescriptor(const XMLTag& intag, std::istream& is)
{
  XMLTag tag(intag);
  type_ = tag.attributes["type"]=="" ? -1 : boost::lexical_cast<int,std::string>(tag.attributes["type"]);
  if (tag.type!=XMLTag::SINGLE) {
    term_=parse_content(is);
    tag = parse_tag(is);
    if (tag.name !="/SITETERM")
      boost::throw_exception(std::runtime_error("Illegal tag <" + tag.name + "> in <SITETERM> element"));
  }
}

template <class I>
void SiteTermDescriptor<I>::write_xml(oxstream& os) const
{
  os << start_tag("SITETERM");
  if (type_>=0)
    os << attribute("type", type_);
  if (term()!="")
    os << term();
  os << end_tag("SITETERM");
}

#endif

} // namespace alps

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
namespace alps {
#endif

template <class I>
inline alps::oxstream& operator<<(alps::oxstream& out, const alps::SiteTermDescriptor<I>& q)
{
  q.write_xml(out);
  return out;
}

template <class I>
inline std::ostream& operator<<(std::ostream& out, const alps::SiteTermDescriptor<I>& q)
{
  alps::oxstream xml(out);
  xml << q;
  return out;
}

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
} // namespace alps
#endif

#endif
