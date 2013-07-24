/*****************************************************************************
 *
 * ALPS DMFT Project - BLAS Compatibility headers
 *  Square Matrix Class
 *
 * Copyright (C) 2005 - 2012 by 
 *                              Emanuel Gull <gull@phys.columbia.edu>
 *                              Brigitte Surer <surerb@phys.ethz.ch>
 *                              Andreas Hehn <hehn@phys.ethz.ch>
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

#ifndef ALPS_VECTOR_HPP
#define ALPS_VECTOR_HPP

#include <alps/numeric/matrix/detail/vector_adaptor.hpp>
#include <alps/numeric/matrix/matrix_traits.hpp>

#include <alps/numeric/matrix/detail/blasmacros.hpp>
#include <boost/numeric/bindings/blas/level1/dot.hpp>
#include <boost/numeric/bindings/blas/level1/scal.hpp>
#include <boost/numeric/bindings/blas/level1/axpy.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <alps/numeric/scalar_product.hpp>

#ifdef HAVE_ALPS_HDF5
#include <alps/hdf5.hpp>
#endif

namespace alps {
  namespace numeric {
    namespace detail {
        template <typename T, typename T2>
        struct multiplies : public std::binary_function<T,T2,T>
        {
            inline T operator()(T t, T2 const& t2) const
            {
                return t*t2;
            }
        };
    } // end namespace detail

    template <typename T1, typename T2, class MemoryBlock1, class MemoryBlock2>
    void plus_assign(vector<T1, MemoryBlock1> & lhs, const vector<T2, MemoryBlock2> & rhs)
    {
        std::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(), std::plus<typename vector<T2, MemoryBlock2>::value_type>());
    }

    template <typename T1, typename T2, class MemoryBlock1, class MemoryBlock2>
    void minus_assign(vector<T1, MemoryBlock1> & lhs, const vector<T2, MemoryBlock2> & rhs)
    {
        std::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(), std::minus<typename vector<T2, MemoryBlock2>::value_type>());
    }

    template <typename T1, typename T2, class MemoryBlock>
    void multiplies_assign(vector<T1, MemoryBlock> & lhs, T2 lambda)
    {
        using detail::multiplies;
        std::transform(lhs.begin(), lhs.end(), lhs.begin(), std::bind2nd(multiplies<typename vector<T1, MemoryBlock>::value_type, T2>(), lambda));
    }

  template<typename T, typename MemoryBlock = std::vector<T> >
  class vector : public MemoryBlock
  {
    public:
      vector(std::size_t size=0, T const& initial_value = T())
      : MemoryBlock(size, initial_value)
      {
      }

      vector(vector const& v)
      : MemoryBlock(v)
      {
      }

      template <typename OtherMemoryBlock>
      vector(vector<T,OtherMemoryBlock> const& v)
      : MemoryBlock( v.begin(), v.end() )
      {
      }

      template <class InputIterator>
      vector (InputIterator first, InputIterator last)
      : MemoryBlock( first, last )
      {
      }

      friend void swap(vector& x,vector& y)
      {
          std::swap(x, y);
      }

      inline T &operator()(const std::size_t i)
      {
          assert((i < this->size()));
          return this->operator[](i);
      }

      inline const T &operator()(std::size_t i) const
      {
          assert((i < this->size()));
          return this->operator[](i);
      }

      vector& operator+=(const vector& rhs)
      {
          assert(rhs.size() == this->size());
          plus_assign(*this, rhs);
          return *this;
      }

      vector& operator-=(const vector& rhs)
      {
          assert(rhs.size() == this->size());
          minus_assign(*this, rhs);
          return *this;
      }

      template <typename T2>
      vector& operator *= (T2 const& lambda)
      {
          multiplies_assign(*this, lambda);
          return *this;
      }
  };

    template<typename T, typename MemoryBlock>
    void insert(vector<T,MemoryBlock>& v, T value, std::size_t i)
    {
        assert((i <= v.size()));
        v.insert(v.begin()+i,value);
    }

    template<typename T, typename MemoryBlock>
    vector<T,MemoryBlock> operator+(vector<T,MemoryBlock> v1, const vector<T,MemoryBlock>& v2)
    {
        assert(v1.size() == v2.size());
        v1 += v2;
        return v1;
    }

    template<typename T, typename MemoryBlock>
    vector<T,MemoryBlock> operator-(vector<T,MemoryBlock> v1, const vector<T,MemoryBlock>& v2)
    {
        assert(v1.size() == v2.size());
        v1 -= v2;
        return v1;
    }

    template <typename T1, typename T2, typename MemoryBlock>
    vector<T2,MemoryBlock> operator * (T1 const& t, vector<T2,MemoryBlock> v)
    {
        return v *= t;
    }

    template <typename T1, typename T2, typename MemoryBlock>
    vector<T1,MemoryBlock> operator * (vector<T1,MemoryBlock> v, T2 const& t)
    {
        return v *= t;
    }


    template <typename T, typename MemoryBlock>
    inline vector<T,MemoryBlock> exp(T c, vector<T,MemoryBlock> v)
    {
        vector<T,MemoryBlock> result(v.size());
        v*=c;
        std::transform(v.begin(), v.end(), result.begin(), static_cast<T(*)(T)> (&std::exp));
        return result;
    }

    template <typename MemoryBlock>
    inline vector<double,MemoryBlock> exp(double c, vector<double,MemoryBlock> v)
    {
        fortran_int_t s=v.size();
        vector<double,MemoryBlock> result(s);
        v*=c;
#ifdef VECLIB
        vecLib::vvexp(&result[0], &v[0], &s);
#else
#ifdef ACML
        acml::vrda_exp(s, &v[0], &result[0]);
#else
#ifdef MKL
        mkl::vdExp(s,  &v[0], &result[0]);
#else
        using std::exp;
        std::transform(v.begin(), v.end(), result.begin(), static_cast<double(*)(double)> (&exp));
#endif
#endif
#endif
        return result;
    }

  template <typename T, typename MemoryBlock>
  inline std::ostream &operator<<(std::ostream &os, const vector<T,MemoryBlock> &v)
  {
    os<<"[ ";
    for(unsigned int i=0;i<v.size()-1;++i){
      os<<v(i)<<", ";
    }
      os<< v(v.size()-1) << "]"<<std::endl;
    return os;
  }

#define PLUS_ASSIGN(T) \
template <typename MemoryBlock> \
void plus_assign(vector<T,MemoryBlock> & lhs, const vector<T,MemoryBlock> & rhs) \
{ boost::numeric::bindings::blas::detail::axpy(lhs.end()-lhs.begin(), 1., &*rhs.begin(), 1, &*lhs.begin(), 1);}
    ALPS_IMPLEMENT_FOR_ALL_BLAS_TYPES(PLUS_ASSIGN)
#undef PLUS_ASSIGN


#define MINUS_ASSIGN(T) \
template <typename MemoryBlock> \
void minus_assign(vector<T, MemoryBlock> & lhs, const vector<T,MemoryBlock> & rhs) \
{ boost::numeric::bindings::blas::detail::axpy(lhs.end()-lhs.begin(), -1., &*rhs.begin(), 1, &*lhs.begin(), 1);}
    ALPS_IMPLEMENT_FOR_ALL_BLAS_TYPES(MINUS_ASSIGN)
#undef MINUS_ASSIGN

#define MULTIPLIES_ASSIGN(T) \
template <typename MemoryBlock, typename T1> \
void multiplies_assign(vector<T, MemoryBlock> & rhs, T1 lambda) \
    { boost::numeric::bindings::blas::detail::scal(rhs.end()-rhs.begin(), lambda, &*rhs.begin(), 1);}
    ALPS_IMPLEMENT_FOR_ALL_BLAS_TYPES(MULTIPLIES_ASSIGN)
#undef MULTIPLIES_ASSIGN

#define SCALAR_PRODUCT(T) \
template <typename MemoryBlock> \
inline T scalar_product(const vector<T,MemoryBlock> v1, const vector<T,MemoryBlock> v2) \
    { return boost::numeric::bindings::blas::detail::dot(v1.size(), &v1[0],1,&v2[0],1);}
    ALPS_IMPLEMENT_FOR_ALL_BLAS_TYPES(SCALAR_PRODUCT)
#undef SCALAR_PRODUCT
    
    
    template <typename Matrix, typename T, typename MemoryBlock>
    struct is_matrix_scalar_multiplication<Matrix,vector<T,MemoryBlock> > {
        static bool const value = false;
    };
    
   } //namespace numeric 
} //namespace alps


#ifdef HAVE_ALPS_HDF5
namespace alps {
    namespace hdf5 {

        template <typename T, typename MemoryBlock>
        void save(
                  alps::hdf5::archive & ar
                  , std::string const & path
                  , alps::numeric::vector<T, MemoryBlock> const & value
                  , std::vector<std::size_t> size = std::vector<std::size_t>()
                  , std::vector<std::size_t> chunk = std::vector<std::size_t>()
                  , std::vector<std::size_t> offset = std::vector<std::size_t>()
                  ) {
            ar[path] << MemoryBlock(value.begin(), value.end());
        }
        template <typename T, typename MemoryBlock>
        void load(
                  alps::hdf5::archive & ar
                  , std::string const & path
                  , alps::numeric::vector<T, MemoryBlock> & value
                  , std::vector<std::size_t> chunk = std::vector<std::size_t>()
                  , std::vector<std::size_t> offset = std::vector<std::size_t>()
                  ) {
            MemoryBlock tmp;
            ar[path] >> tmp;
            value = alps::numeric::vector<T, MemoryBlock>(tmp.begin(), tmp.end());
        }

    }
}
#endif //HAVE_ALPS_HDF5 


#endif //ALPS_VECTOR_HPP
