/*****************************************************************************
*
* ALPS Project: Algorithms and Libraries for Physics Simulations
*
* ALPS Libraries
*
* Copyright (C) 2001-2004 by Matthias Troyer <troyer@comp-phys.org>,
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

#ifndef ALPS_LATTICE_HYPERCUBIC_H
#define ALPS_LATTICE_HYPERCUBIC_H

#include <alps/config.h>
#include <alps/lattice/lattice.h>
#include <alps/lattice/coordinate_traits.h>
#include <alps/vectorio.h>
#include <alps/vectormath.h>
#include <boost/utility.hpp>

#include <vector>

namespace alps {

template <class BASE, class EX = typename lattice_traits<BASE>::offset_type>
class hypercubic_lattice;

template <class BASE, class EX>
inline std::size_t dimension(const hypercubic_lattice<BASE,EX>& l)
{
  return l.dimension();
}


template <class BASE, class EX>
class hypercubic_lattice : public BASE {
public:
  typedef hypercubic_lattice<BASE> lattice_type;
  typedef BASE parent_lattice_type;

  typedef typename lattice_traits<parent_lattice_type>::unit_cell_type unit_cell_type;
  typedef typename lattice_traits<parent_lattice_type>::cell_descriptor cell_descriptor;
  typedef typename lattice_traits<parent_lattice_type>::offset_type offset_type;
  typedef EX extent_type;
  typedef typename lattice_traits<parent_lattice_type>::basis_vector_iterator basis_vector_iterator;
  typedef typename lattice_traits<parent_lattice_type>::vector_type vector_type;
  typedef boundary_crossing boundary_crossing_type;
  typedef std::vector<unsigned int> distance_type;

  typedef int size_type;

  hypercubic_lattice() {}

  template <class BASE2, class EX2>
  hypercubic_lattice(const hypercubic_lattice<BASE2,EX2>& l)
   : parent_lattice_type(l), extent_(l.extent().begin(), l.extent().end()),
     bc_(l.boundary())
   { fill_extent(); }

  hypercubic_lattice(const parent_lattice_type& p, size_type length,
                     const std::string& bc = "periodic")
    : parent_lattice_type(p), extent_(BASE::dimension(), length),
      bc_(BASE::dimension(), bc) {}

  template <class InputIterator>
  hypercubic_lattice(const parent_lattice_type& p,
                     InputIterator first, InputIterator last,
                     const std::string& bc = "periodic")
    : parent_lattice_type(p), extent_(first, last), bc_(BASE::dimension(), bc)
  { fill_extent(); }

  template <class InputIterator2>
  hypercubic_lattice(const parent_lattice_type& p, size_type length,
                     InputIterator2 first2, InputIterator2 last2)
    : parent_lattice_type(p), extent_(BASE::dimension(), length),
      bc_(first2, last2)
  { fill_extent(); }

     template <class InputIterator, class InputIterator2>
  hypercubic_lattice(const parent_lattice_type& p,
                     InputIterator first, InputIterator last,
                     InputIterator2 first2, InputIterator2 last2)
   : parent_lattice_type(p), extent_(first, last), bc_(first2, last2)
  { fill_extent(); }

  template <class BASE2, class EX2>
  const hypercubic_lattice& operator=(const hypercubic_lattice<BASE2,EX2>& l)
  {
    static_cast<BASE&>(*this)=l;
    extent_=l.extent();
    bc_ = l.boundary();
    return *this;
  }

  class cell_iterator {
  public:

    cell_iterator() {}
    cell_iterator(const lattice_type& l, const offset_type& o)
     : lattice_(&l), offset_(o) {}

    const cell_iterator& operator++() {
      typedef typename coordinate_traits<offset_type>::iterator offset_iterator;
      typedef typename coordinate_traits<offset_type>::const_iterator const_offset_iterator;
      offset_iterator offit, offend;
      boost::tie(offit,offend)=coordinates(offset_);
      int d=0;
      (*offit)++;
      while (*offit == lattice_->extent(d) && (offit+1) != offend)
        {
          *offit =0;
          ++d;
          ++offit;
          (*offit)++;
        }
      return *this;
    }

    cell_iterator operator++(int) {
      cell_iterator tmp(*this);
      operator++();
      return tmp;
    }

    bool operator ==(const cell_iterator& it)
    {
      return (lattice_ == it.lattice_ && offset_ == it.offset_);
    }

    bool operator !=(const cell_iterator& it)
    {
      return (lattice_ != it.lattice_ || offset_ != it.offset_);
    }

    cell_descriptor operator*() const { return lattice_->cell(offset_);}
    // operator-> looks harder to implement. any good ideas?

  protected:
    const lattice_type* lattice_;
    offset_type offset_;
  };

  std::pair<cell_iterator,cell_iterator> cells() const
  {
    offset_type begin(extent_);
    offset_type end(extent_);
    std::fill(coordinates(begin).first,coordinates(begin).second,0);
    std::fill(coordinates(end).first,coordinates(end).second-1,0);
    return std::make_pair(cell_iterator(*this,begin),cell_iterator(*this,end));
  }

  size_type volume() const {
    return std::accumulate(extent_.begin(),extent_.end(),size_type(1),
           std::multiplies<size_type>());
  }

  size_type index(const cell_descriptor& c) const
  {
    size_type ind=0;
    size_type factor=1;
    offset_type o=offset(c,*this);
    typedef typename coordinate_traits<offset_type>::const_iterator CI;
    CI exit = coordinates(extent_).first;
    CI offit = coordinates(o).first;

    for (;exit!=coordinates(extent_).second;++exit,++offit)
    {
      ind += factor* (*offit);
      factor *= (*exit);
    }
    return ind;
  }

  bool on_lattice(const cell_descriptor& c) const {
    typedef typename coordinate_traits<offset_type>::const_iterator CI;
    CI exit = coordinates(extent_).first;
    CI offit = coordinates(offset(c,*this)).first;

    for (;exit!=coordinates(extent_).last;++exit,++offit)
      if(*offit<0 || *offit>=*exit)
        return false;
    return true;
  }

  cell_descriptor cell(size_type i)  const{
    offset_type o;

    typedef typename coordinate_traits<offset_type>::const_iterator CIT;
    typedef typename coordinate_traits<offset_type>::iterator IT;
    CIT ex = coordinates(extent_).first;
    IT offit = coordinates(o).first;

    for (;exit!=coordinates(extent_).last;++ex,++offit)
    {
      *offit=i%(*ex);
      i/=(*ex);
    }
    return cell(o);
  }

  cell_descriptor cell(offset_type o) const
  {
    return alps::cell(o,static_cast<const parent_lattice_type&>(*this));
  }

  std::pair<bool,boundary_crossing_type> shift(offset_type& o,const offset_type& s) const
  {
    o=o+s;
    bool ison=true;
    typedef typename coordinate_traits<offset_type>::iterator IT;
    typedef typename coordinate_traits<offset_type>::const_iterator CIT;
    IT offit=alps::coordinates(o).first;
    CIT exit=alps::coordinates(extent_).first;
    std::vector<std::string>::const_iterator bit=bc_.begin();
    boundary_crossing_type crossing;
    for (int dim=0; exit!=alps::coordinates(extent_).second;++dim, ++bit, ++offit, ++exit)
    {
      if (*offit<0)
      while (*offit<0)
      {
              if (*bit=="periodic") {
                *offit+=*exit; // need to check % for negative numbers
          crossing.set_crossing(dim,-1);
        }
              else
                ison=false;
      }
      else if (*offit >= *exit)
      {
              if (*bit=="periodic") {
                *offit %= *exit;
          crossing.set_crossing(dim,1);
        }
              else
                ison=false;
      }
    }
    return std::make_pair(ison,crossing);
  }

  const std::string& boundary(unsigned int dim) const{ return bc_[dim];}
  const std::vector<std::string>& boundary() const { return bc_;}
  typename extent_type::value_type extent(unsigned int dim) const {return extent_[dim];}
  const extent_type& extent() const { return extent_;}

  distance_type distance_sizes() const
  {
    distance_type sizes;
    for (int i=0;i<BASE::dimension();++i) {
      sizes.push_back(extent(i));
      if(boundary(i)!="periodic")
         sizes.push_back(extent(i));
    }
    return sizes;
  }

  distance_type distance_vector(const offset_type& x, const offset_type& y) const
  {
    distance_type d;
    for (int i=0;i<BASE::dimension();++i) {
      if(boundary(i)=="periodic") {
        d.push_back(x[i]<y[i] ? y[i]-x[i] : x[i]-y[i]);
      }
      else {
        d.push_back(x[i]);
        d.push_back(y[i]);
      }
    }
    return d;
  }

  class momentum_iterator : public cell_iterator {
  public:
    momentum_iterator(cell_iterator it=cell_iterator()) : cell_iterator(it) {}
    const vector_type& operator*() const { set_k(); return k_; }
    const vector_type* operator->() const { set_k(); return &k_; }
  private:
    mutable vector_type k_;
    void set_k()
    {
      k_=*basis_vectors(*cell_iterator::lattice_).first;
      for (int i=0;i<dimension(*cell_iterator::lattice_);++i)
        k_[i]=cell_iterator::offset_[i]*2.*M_PI/double(cell_iterator::lattice_->extent()[i]);
    }
  };

  std::pair<momentum_iterator,momentum_iterator> momenta() const
  {
    return std::make_pair(momentum_iterator(cells().first),momentum_iterator(cells().second));
  }


protected:
  extent_type extent_;
  std::vector<std::string> bc_;
private:
  void fill_extent() {
    extent_.resize(BASE::dimension(),1);
    bc_.resize(BASE::dimension(),"open");
  }
};

template <class BASE, class EX>
struct lattice_traits<hypercubic_lattice<BASE,EX> >
{
  typedef typename hypercubic_lattice<BASE,EX>::unit_cell_type unit_cell_type;
  typedef typename hypercubic_lattice<BASE,EX>::cell_descriptor cell_descriptor;
  typedef typename hypercubic_lattice<BASE,EX>::offset_type offset_type;
  typedef typename hypercubic_lattice<BASE,EX>::extent_type extent_type;
  typedef typename hypercubic_lattice<BASE,EX>::basis_vector_iterator basis_vector_iterator;
  typedef typename hypercubic_lattice<BASE,EX>::momentum_iterator momentum_iterator;
  typedef typename hypercubic_lattice<BASE,EX>::cell_iterator cell_iterator;
  typedef typename hypercubic_lattice<BASE,EX>::size_type size_type;
  typedef typename hypercubic_lattice<BASE,EX>::vector_type vector_type;
  typedef typename hypercubic_lattice<BASE,EX>::boundary_crossing_type boundary_crossing_type;
};

} // end namespace alps

#endif // ALPS_LATTICE_HYPERCUBIC_H
