/*****************************************************************************
*
* ALPS Project: Algorithms and Libraries for Physics Simulations
*
* ALPS Libraries
*
* Copyright (C) 2001-2005 by Matthias Troyer <troyer@comp-phys.org>,
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

#ifndef ALPS_LATTICE_PROPERTYMAP_H
#define ALPS_LATTICE_PROPERTYMAP_H

#include <alps/config.h>
#include <boost/limits.hpp>
#include <boost/pending/property.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/any.hpp>
#include <boost/version.hpp>
#include <boost/mpl/if.hpp>

#include <string>
#include <vector>

namespace alps {

template <class V, class K=boost::any> // singleton_property_map and constant_property_map
class singleton_property_map {
public:
  typedef K key_type;
  typedef V value_type;
  typedef V& reference;
  typedef boost::lvalue_property_map_tag category;

  singleton_property_map(V v=V()) : v_(v) {}

  operator V () const { return v_;}
  V value() const { return v_;}

  const singleton_property_map<V,K>& operator=(const V& v) { v_=v; return *this;}

  template <class T>
  V& operator[](T ) { return v_;}

  template <class T>
  const V& operator[](T ) const { return v_;}
private:
  V v_;
};

} // end namespace

namespace boost {
template <class V, class K>
V get(const alps::singleton_property_map<V,K>& m, const K&) { return m.value();}

template <class V, class K>
void put(alps::singleton_property_map<V,K>& m, const K& k, const V& v) { m[k]=v;}
}

namespace alps {
namespace detail {

// helper functions to probe for graph properties
/*
template <class T, class DEFAULT=int>
struct existing_property {
  BOOST_STATIC_CONSTANT(bool, result=true);
  typedef T type;
};

template <class DEFAULT>
struct existing_property<boost::detail::error_property_not_found,DEFAULT> {
  BOOST_STATIC_CONSTANT(bool, result=false);
  typedef DEFAULT type;
};
*/

// boost 1.51 changed the boost::property_value interface
#if BOOST_VERSION < 105100

template <class T, class DEFAULT=int>
struct existing_property_internal {
  BOOST_STATIC_CONSTANT(bool, result=true);
  typedef T type;
};

template <class DEFAULT>
struct existing_property_internal<boost::detail::error_property_not_found,DEFAULT> {
  BOOST_STATIC_CONSTANT(bool, result=false);
  typedef DEFAULT type;
};

template <class PropertyList, class Tag, class Default=int> struct existing_property
    : public existing_property_internal<typename boost::property_value<PropertyList, Tag>::type, Default>
{};

#else

}
}

namespace boost {

  template <typename Tag, typename T, typename PropName>
  struct lookup_one_property_internal<boost::property<Tag, T, boost::no_property>, PropName>: lookup_one_property_internal<boost::no_property, PropName> {
    typedef T type;
  };

  template <typename Tag, typename T>
  struct lookup_one_property_internal<boost::property<Tag, T, boost::no_property>, Tag> {
    BOOST_STATIC_CONSTANT(bool, found = true);
    typedef property<Tag, T, boost::no_property> prop;
    typedef T type;
    template <typename U>
    static typename enable_if<is_same<prop, U>, T&>::type
    lookup(U& prop, const Tag&) {return prop.m_value;}
    template <typename U>
    static typename enable_if<is_same<prop, U>, const T&>::type
    lookup(const U& prop, const Tag&) {return prop.m_value;}
  };
  
#define BGL_PATCH_ALL_PROP(tag) \
  template <typename Tag, typename T> \
  struct lookup_one_property_internal<property<Tag, T, boost::no_property>, tag> { /* Avoid ambiguity */ \
    BOOST_STATIC_CONSTANT(bool, found = false); \
    typedef property<Tag, T, boost::no_property> type; \
    static type& lookup(type& x, tag) {return x;} \
    static const type& lookup(const type& x, tag) {return x;} \
  };
BGL_PATCH_ALL_PROP(vertex_all_t)
BGL_PATCH_ALL_PROP(edge_all_t)
BGL_PATCH_ALL_PROP(graph_all_t)
BGL_PATCH_ALL_PROP(vertex_bundle_t)
BGL_PATCH_ALL_PROP(edge_bundle_t)
BGL_PATCH_ALL_PROP(graph_bundle_t)
#undef BGL_PATCH_ALL_PROP

  template <typename Tag> struct property_value<boost::no_property, Tag> {
    BOOST_STATIC_CONSTANT(bool, found = false);
    typedef boost::no_property type;
  };

}

namespace alps {
namespace detail {

template <bool, class PropertyList, class Tag, class Default> struct existing_property_internal {
    typedef typename boost::property_value<PropertyList, Tag>::type type;
};

template <class PropertyList, class Tag, class Default> struct existing_property_internal<false, PropertyList, Tag, Default> {
    typedef Default type;
};

template <class PropertyList, class Tag, class Default=int> struct existing_property {
    BOOST_STATIC_CONSTANT(bool, result = (boost::property_value<PropertyList, Tag>::found));
    typedef typename existing_property_internal<boost::property_value<PropertyList, Tag>::found, PropertyList, Tag, Default>::type type;
};

#endif

} // end namespace detail



template <class Property, class Graph, class Default=int>
struct has_property {
  BOOST_STATIC_CONSTANT(bool, vertex_property=false);
  BOOST_STATIC_CONSTANT(bool, edge_property=false);
  BOOST_STATIC_CONSTANT(bool, graph_property=false);
  BOOST_STATIC_CONSTANT(bool, any_property=false);
  BOOST_STATIC_CONSTANT(bool, site_property = vertex_property);
  BOOST_STATIC_CONSTANT(bool, bond_property = edge_property);
  typedef Default vertex_property_type;
  typedef Default edge_property_type;
  typedef Default graph_property_type;
  typedef Default property_type;
  typedef property_type type;
  typedef vertex_property_type site_property_type;
  typedef edge_property_type bond_property_type;
};

template <class s1, class s2, class s3, class VP, class EP, class GP, class s4, class P, class D>
struct has_property<P, boost::adjacency_list<s1,s2,s3,VP,EP,GP,s4>, D>
{
  typedef boost::adjacency_list<s1,s2,s3,VP,EP,GP,s4> Graph;
  BOOST_STATIC_CONSTANT(bool, vertex_property = (detail::existing_property<VP,P,D>::result));
  BOOST_STATIC_CONSTANT(bool, edge_property = (detail::existing_property<EP,P,D>::result));
  BOOST_STATIC_CONSTANT(bool, graph_property = (detail::existing_property<GP,P,D>::result));
  BOOST_STATIC_CONSTANT(bool, any_property = (edge_property || vertex_property || graph_property));
  BOOST_STATIC_CONSTANT(bool, site_property = vertex_property);
  BOOST_STATIC_CONSTANT(bool, bond_property = edge_property);
  typedef typename detail::existing_property<VP,P,D>::type vertex_property_type;
  typedef typename detail::existing_property<EP,P,D>::type edge_property_type;
  typedef typename detail::existing_property<GP,P,D>::type graph_property_type;
  typedef typename boost::mpl::if_c<edge_property,edge_property_type,
    typename boost::mpl::if_c<vertex_property,vertex_property_type,
    graph_property_type>::type>::type property_type;
  typedef property_type type;
  typedef vertex_property_type site_property_type;
  typedef edge_property_type bond_property_type;
};

#ifndef BOOST_NO_INCLASS_MEMBER_INITIALIZATION
template <class s1, class s2, class s3, class VP, class EP, class GP, class s4, class P, class D>
const bool has_property<P, boost::adjacency_list<s1,s2,s3,VP,EP,GP,s4>,D>::vertex_property;
template <class s1, class s2, class s3, class VP, class EP, class GP, class s4, class P, class D>
const bool has_property<P, boost::adjacency_list<s1,s2,s3,VP,EP,GP,s4>,D>::edge_property;
template <class s1, class s2, class s3, class VP, class EP, class GP, class s4, class P, class D>
const bool has_property<P, boost::adjacency_list<s1,s2,s3,VP,EP,GP,s4>,D>::graph_property;
template <class s1, class s2, class s3, class VP, class EP, class GP, class s4, class P, class D>
const bool has_property<P, boost::adjacency_list<s1,s2,s3,VP,EP,GP,s4>,D>::any_property;
template <class s1, class s2, class s3, class VP, class EP, class GP, class s4, class P, class D>
const bool has_property<P, boost::adjacency_list<s1,s2,s3,VP,EP,GP,s4>,D>::site_property;
template <class s1, class s2, class s3, class VP, class EP, class GP, class s4, class P, class D>
const bool has_property<P, boost::adjacency_list<s1,s2,s3,VP,EP,GP,s4>,D>::bond_property;
#endif

template <class s1, class s2, class s3, class VP, class EP, class GP, class s4, class P, class D>
struct has_property<P, const boost::adjacency_list<s1,s2,s3,VP,EP,GP,s4>, D>
{
  typedef boost::adjacency_list<s1,s2,s3,VP,EP,GP,s4> Graph;
  BOOST_STATIC_CONSTANT(bool, vertex_property = (detail::existing_property<VP,P,D>::result));
  BOOST_STATIC_CONSTANT(bool, edge_property = (detail::existing_property<EP,P,D>::result));
  BOOST_STATIC_CONSTANT(bool, graph_property = (detail::existing_property<GP,P,D>::result));
  BOOST_STATIC_CONSTANT(bool, any_property =
    (edge_property || vertex_property || graph_property));
  BOOST_STATIC_CONSTANT(bool, site_property = vertex_property);
  BOOST_STATIC_CONSTANT(bool, bond_property = edge_property);
  typedef typename detail::existing_property<VP,P,D>::type vertex_property_type;
  typedef typename detail::existing_property<EP,P,D>::type edge_property_type;
  typedef typename detail::existing_property<GP,P,D>::type graph_property_type;
  typedef typename boost::mpl::if_c<edge_property,edge_property_type,
    typename boost::mpl::if_c<vertex_property,vertex_property_type,
    graph_property_type>::type>::type property_type;
  typedef property_type type;
  typedef vertex_property_type site_property_type;
  typedef edge_property_type bond_property_type;
};

#ifndef BOOST_NO_INCLASS_MEMBER_INITIALIZATION
template <class s1, class s2, class s3, class VP, class EP, class GP, class s4, class P, class D>
const bool has_property<P, const boost::adjacency_list<s1,s2,s3,VP,EP,GP,s4>,D>::vertex_property;
template <class s1, class s2, class s3, class VP, class EP, class GP, class s4, class P, class D>
const bool has_property<P, const boost::adjacency_list<s1,s2,s3,VP,EP,GP,s4>,D>::edge_property;
template <class s1, class s2, class s3, class VP, class EP, class GP, class s4, class P, class D>
const bool has_property<P, const boost::adjacency_list<s1,s2,s3,VP,EP,GP,s4>,D>::graph_property;
template <class s1, class s2, class s3, class VP, class EP, class GP, class s4, class P, class D>
const bool has_property<P, const boost::adjacency_list<s1,s2,s3,VP,EP,GP,s4>,D>::any_property;
template <class s1, class s2, class s3, class VP, class EP, class GP, class s4, class P, class D>
const bool has_property<P, const boost::adjacency_list<s1,s2,s3,VP,EP,GP,s4>,D>::site_property;
template <class s1, class s2, class s3, class VP, class EP, class GP, class s4, class P, class D>
const bool has_property<P, const boost::adjacency_list<s1,s2,s3,VP,EP,GP,s4>,D>::bond_property;
#endif

template <class P, class G, class Default>
struct property_map
{
  typedef 
    typename boost::mpl::if_c<has_property<P,G>::graph_property,
      typename boost::graph_property<G,P>::type&,
      typename boost::mpl::if_c<has_property<P,G>::any_property,
        typename boost::property_map<G,P>::type,
        singleton_property_map<Default> 
      >::type
    >::type type;

  typedef
    typename boost::mpl::if_c<has_property<P,G>::graph_property,
      typename boost::graph_property<G,P>::type const&,
      typename boost::mpl::if_c<has_property<P,G>::any_property,
        typename boost::property_map<G,P>::const_type,
        singleton_property_map<Default>
      >::type
    >::type const_type;
};

template <class P, class G, class Default>
struct property_map<P, const G, Default>
{
  typedef
    typename boost::mpl::if_c<has_property<P,G>::graph_property,
      typename boost::graph_property<G,P>::type const&,
      typename boost::mpl::if_c<has_property<P,G>::any_property,
        typename boost::property_map<G,P>::const_type,
        singleton_property_map<Default>
      >::type
    >::type type;

  typedef type const_type;
};


namespace detail {

template <bool F>
struct put_get_helper {};

template <>
struct put_get_helper<true>
{
  template <class P, class G, class T>
  static typename property_map<P,G,T>::type get (P p, G& g, T) {
    return put_get_helper<has_property<P,G>::graph_property>::get_property(p,g);
  }

  template <class P, class G>
  static typename property_map<P,G,int>::type get_property (P p, G& g) 
  { 
    BOOST_STATIC_ASSERT((has_property<P,G>::graph_property));
    return boost::get_property(g,p);
  }
};

template <>
struct put_get_helper<false>
{
  template <class P, class G, class V>
  static singleton_property_map<V> get (P, const G&, const V& v)
  { 
    BOOST_STATIC_ASSERT((!has_property<P,G>::any_property));
    return singleton_property_map<V>(v);
  }


  template <class P, class G>
  static typename property_map<P,G,int>::type get_property (P p, G& g) 
  { 
    BOOST_STATIC_ASSERT((has_property<P,G>::any_property));
    BOOST_STATIC_ASSERT((!has_property<P,G>::graph_property));
    return boost::get(p,g);
  }
};

} // end namespace detail

template <class P, class G, class V>
inline typename property_map<P,G,V>::type
get_or_default(P p, G& g, const V& v)
{
  return detail::put_get_helper<has_property<P,G>::any_property>::get(p,g,v);
}

template <class SRC, class DST, class PROPERTY, bool has_property>
struct copy_property_helper
{
  template <class SRCREF, class DSTREF>
  static void copy(const SRC&, const SRCREF&, DST&, const DSTREF&) {}
};

template <class SRC, class DST, class PROPERTY>
struct copy_property_helper<SRC,DST,PROPERTY,true>
{
  template <class SRCREF, class DSTREF>
  static void copy(const SRC& s, const SRCREF& sr, DST& d, const DSTREF& dr) {
    boost::put(PROPERTY(),d,dr, boost::get(PROPERTY(),s,sr));
  }
};

} // end namespace alps

#endif // ALPS_LATTICE_GRAPH_PROPERTIES_H
