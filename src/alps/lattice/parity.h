/***************************************************************************
* ALPS++/lattice library
*
* lattice/parity.h   setting parity for bipartite graphs
*
* $Id$
*
* Copyright (C) 2001-2003 by Matthias Troyer <troyer@itp.phys.ethz.ch>
*                            Synge Todo <wistaria@comp-phys.org>
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

#ifndef ALPS_LATTICE_PARITY_H
#define ALPS_LATTICE_PARITY_H

#include <alps/config.h>
#include <alps/lattice/graphproperties.h>

#define ALPS_USE_DFS2 // define this if you want non-recursive version of DFS

#ifndef ALPS_USE_DFS2
# include <boost/graph/depth_first_search.hpp>
#else
# include <boost/depth_first_search_2.hpp> // non-recursive version of DFS
#endif
#include <boost/graph/visitors.hpp>
#include <boost/throw_exception.hpp>
#include <stdexcept>

namespace alps {

namespace parity {

typedef int8_t parity_type;
BOOST_STATIC_CONSTANT(parity_type, white = 0);
BOOST_STATIC_CONSTANT(parity_type, black = 1);
BOOST_STATIC_CONSTANT(parity_type, undefined = 2);

template<class Graph, class PropertyMap>
class ParityVisitor
{
public:
  typedef typename boost::graph_traits<Graph>::edge_descriptor
    edge_descriptor;
  typedef typename boost::graph_traits<Graph>::vertex_descriptor
    vertex_descriptor;

  // constructor
  ParityVisitor(PropertyMap& map, bool* check) :
    p_(white), map_(map), check_(check) { *check_ = true; }

  // callback member functions
  void initialize_vertex(vertex_descriptor s, const Graph&) {
    map_[s]=undefined;
  }
  void start_vertex(vertex_descriptor, const Graph&) {}
  void discover_vertex(vertex_descriptor s, const Graph&) {
    flip();
    map_[s]=p_;
  }
  void examine_edge(edge_descriptor, const Graph&) {}
  void tree_edge(edge_descriptor, const Graph&) {}
  void back_edge(edge_descriptor e, const Graph& g) { check(e, g); }
  void forward_or_cross_edge(edge_descriptor e, const Graph& g) {
    check(e, g);
  }
  void finish_vertex(vertex_descriptor, const Graph&) { flip(); }

protected:
  ParityVisitor();

  void flip() { p_ = (p_ == white ? black : white); }
  void check(edge_descriptor e, const Graph& g) {
    if (map_[boost::source(e, g)] == undefined ||
 	map_[boost::target(e, g)] == undefined) {
      boost::throw_exception(std::runtime_error("unvisited vertex found"));
    }
    if (map_[boost::source(e, g)] == map_[boost::target(e, g)]) 
      *check_ = false;
  }

private:
  parity_type p_;
  PropertyMap map_;
  bool* check_;
};

} // end namespace parity

template <class Graph, class Map>
bool set_parity(Map map, const Graph& g)
{
  typedef typename boost::graph_traits<Graph>::vertex_iterator
    vertex_iterator;
  typedef typename parity::ParityVisitor<Graph, Map> visitor_type;
  bool check = true;
  visitor_type v(map, &check);
#ifndef ALPS_USE_DFS2
  boost::depth_first_search(g, boost::visitor(v));
#else
  boost::depth_first_search_2(g, boost::visitor(v));
#endif
  if (!check) {
    for (vertex_iterator itr = boost::vertices(g).first;
	  itr != boost::vertices(g).second; ++itr) {
      boost::put(map, *itr, parity::undefined);
    }
  }
  return check;
}

namespace parity {

template<bool HasParity>
struct helper
{
  template<class Graph>
  static bool set_parity(Graph&) { return false; }
};

template<>
struct helper<true>
{
  template<class Graph>
  static bool set_parity(Graph& g) {
    typedef typename property_map<parity_t, Graph, int>::type map_type;
    map_type map = boost::get(parity_t(), g);
    return alps::set_parity(map,g);
  }
};

} // end namespace parity


template<class Graph>
bool set_parity(Graph& g)
{
  return parity::helper<has_property<parity_t, Graph>::vertex_property>
    ::set_parity(g);
}

} // end namespace alps

#endif // ALPS_LATTICE_PARITY_H
