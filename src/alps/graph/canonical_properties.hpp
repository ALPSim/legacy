/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                 *
 * ALPS Project: Algorithms and Libraries for Physics Simulations                  *
 *                                                                                 *
 * ALPS Libraries                                                                  *
 *                                                                                 *
 * Copyright (C) 2010 - 2013 by Lukas Gamper <gamperl@gmail.com>                   *
 *                              Andreas Hehn <hehn@phys.ethz.ch>                   *
 *                                                                                 *
 * This software is part of the ALPS libraries, published under the ALPS           *
 * Library License; you can use, redistribute it and/or modify it under            *
 * the terms of the license, either version 1 or (at your option) any later        *
 * version.                                                                        *
 *                                                                                 *
 * You should have received a copy of the ALPS Library License along with          *
 * the ALPS Libraries; see the file LICENSE.txt. If not, the license is also       *
 * available from http://alps.comp-phys.org/.                                      *
 *                                                                                 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT       *
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE       *
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,     *
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER     *
 * DEALINGS IN THE SOFTWARE.                                                       *
 *                                                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Resources:                                                                      *
 *                                                                                 *
 * original paper: http://cs.anu.edu.au/~bdm/nauty/pgi.pdf                         *
 * the algorithm : http://www.math.unl.edu/~shartke2/math/papers/canonical.pdf     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#ifndef ALPS_GRAPH_CANONICAL_PROPERTIES_HPP
#define ALPS_GRAPH_CANONICAL_PROPERTIES_HPP

#include <boost/mpl/not.hpp>
#include <boost/tuple/tuple_io.hpp>
#include <boost/graph/properties.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/multi_array.hpp>

#include <boost/mpl/if.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>

#include <alps/graph/canonical_properties_traits.hpp>

#include <map>
#include <set>
#include <vector>
#include <algorithm>

namespace alps {
    namespace graph {
        namespace detail {

            // Input: pi = (V1, V2, ..., Vr)
            // Output: I = {(ni, j) : ni element of Vj
            template<typename Graph> void partition_indeces(
                  std::map<typename boost::graph_traits<Graph>::vertex_descriptor, std::size_t> & I
                , typename partition_type<Graph>::type const & pi
                , Graph const & G
            ) {
                for (typename partition_type<Graph>::type::const_iterator it = pi.begin(); it != pi.end(); ++it)
                    for (typename partition_type<Graph>::type::value_type::const_iterator jt = it->begin(); jt != it->end(); ++jt)
                        I[*jt] = it - pi.begin();
            }

            // Given an inequitable ordered partition pi = (V1, V2, . . . , Vr ), we say that Vj shatters Vi 
            // if there exist two vertices v, w element of Vi such that deg(v, Vj ) != deg(w, Vj ).
            // Input: pi = (V1, V2, ..., Vr)
            // Output: i, j; Vj shatters Vi and (i, j) is the minimum element of B under the lexicographic order
            template<typename Graph> std::pair<std::size_t, std::size_t> shattering(
                  typename partition_type<Graph>::type const & pi
                , std::map<typename boost::graph_traits<Graph>::vertex_descriptor, std::size_t> const & I
                , Graph const & G
            ) {
                using boost::make_tuple;
                using std::make_pair;
                // B = {(i, j): Vj shatters Vi}
                std::size_t maxsize = 0;
                for (typename partition_type<Graph>::type::const_iterator it = pi.begin(); it != pi.end(); ++it)
                    maxsize = std::max(maxsize,it->size());
                boost::multi_array<std::size_t,2> adjacent_numbers(boost::extents[pi.size()][maxsize]);
                for (typename partition_type<Graph>::type::const_iterator it = pi.begin(); it != pi.end(); ++it) {
                    std::fill_n(adjacent_numbers.data(),adjacent_numbers.num_elements(),0);
                    for (typename partition_type<Graph>::type::value_type::const_iterator jt = it->begin(); jt != it->end(); ++jt) {
                        typename boost::graph_traits<Graph>::adjacency_iterator ai, ae;
                        for (boost::tie(ai, ae) = adjacent_vertices(*jt, G); ai != ae; ++ai)
                            ++adjacent_numbers[I.find(*ai)->second][jt - it->begin()];
                    }
                    for (boost::multi_array<std::size_t,2>::const_iterator jt = adjacent_numbers.begin(); jt != adjacent_numbers.end(); ++jt)
                    {
                        for (std::size_t k = 0; k < it->size(); ++k)
                            if( *(jt->begin() + k) != *jt->begin() )
                                // Return the minimum element of B under the lexicographic order
                                return make_pair(it - pi.begin(), jt - adjacent_numbers.begin());
                    }
                }
                
                // no shattering found
                return std::make_pair(pi.size(), 0);
            }
            
            // Input: pi = (V1, V2, ..., Vr)
            // Output: An ordered partition R(pi)
            template<typename Graph> void equitable_refinement(
                  typename partition_type<Graph>::type & pi
                , Graph const & G
            ) {
                using boost::tie;
                using std::make_pair;
                if (pi.size() < num_vertices(G))
                    while(true) {
                        std::map<typename boost::graph_traits<Graph>::vertex_descriptor, std::size_t> I;
                        // I = {(ni, j) : ni element of Vj
                        partition_indeces(I, pi, G);
                        std::size_t i, j;
                        boost::tie(i, j) = shattering(pi, I, G);
                        // If B is empty, then stop, reporting pi as the output R(pi)
                        if (i == pi.size())
                            break;
                        // Let (X1, X2, . . . , Xt) be the shattering of Vi by Vj
                        // Replace pi by the ordered partition where Vi is replaced by X1, X2, ..., Xt;
                        // that is, replace pi = (V1, V2, ..., Vr) with (V1, V2, ..., Vi−1, X1, X2, ..., Xt, Vi+1, ..., Vr).
                        typename partition_type<Graph>::type tau(pi.begin(), pi.begin() + i);
                        // O = {(k, l) : nl element of Vi with k adjacents in Vj}
                        std::vector<std::pair<std::size_t, std::size_t> > O;
                        for (typename partition_type<Graph>::type::value_type::const_iterator it = pi[i].begin(); it != pi[i].end(); ++it) {
                            O.push_back(make_pair(0, *it));
                            typename boost::graph_traits<Graph>::adjacency_iterator ai, ae;
                            for (boost::tie(ai, ae) = adjacent_vertices(*it, G); ai != ae; ++ai)
                                if (I[*ai] == j)
                                    ++O.back().first;
                        }
                        std::sort(O.begin(), O.end());
                        // The shattering of Vi by Vj is the ordered partition (X1, X2, . . . , Xt) of
                        // Vi such that if v in Xk and w in Xl then k < l if and only if deg(v, Vj) < deg(w, Vj).
                        // Thus, (X1, X2, . . . , Xt) sorts the vertices of Vi by their degree to Vj
                        tau.push_back(typename partition_type<Graph>::type::value_type(1, O.front().second));
                        std::size_t counter = O.front().first;
                        for (std::vector<std::pair<std::size_t, std::size_t> >::const_iterator it = O.begin() + 1; it != O.end(); ++it)
                            if (counter < it->first) {
                                tau.push_back(typename partition_type<Graph>::type::value_type(1, it->second));
                                counter = it->first;
                            } else
                                tau.back().push_back(it->second);
                        // pi = (V1, V2, ..., Vi−1, X1, X2, ..., Xt, Vi+1, ..., Vr).
                        if (i + 1 < pi.size())
                            std::copy(pi.begin() + i + 1, pi.end(), std::back_inserter(tau));
                        pi = tau;
                    }
            }

            // The search tree T(G) is the rooted tree whose nodes
            // {(pi, u) : pi is an ordered partition of [n]; u = (u1, u2, ..., uk) is a sequence of distinct vertices;
            //   pi = (...     (R(mu) ⊥ u1) ⊥ u2) ... ) ⊥ uk;
            //   ui is in the ﬁrst non-trivial part of (... (R(mu) ⊥ u1) ⊥ u2) ...) ⊥ ui−1 for each i.
            // Input: incomplete trace of T(G)
            //        remaining arches
            // Output: trace from root to a terminal node of T(G)
            template<typename Graph> void terminal_node(
                  std::vector<boost::tuple<typename partition_type<Graph>::type, std::size_t, std::size_t> > & T
                , Graph const & G
            ) {
                using boost::get;
                using boost::make_tuple;
                typename partition_type<Graph>::type & tau = get<0>(*(T.rbegin() + 1));
                typename partition_type<Graph>::type & pi = get<0>(T.back());
                std::size_t i = get<1>(T.back());
                // u = nj, nj is j-th element of Vi
                std::size_t j = get<2>(T.back());
                // Let pi be an equitable ordered partition of [n] with a nontrivial
                // part Vi, and let u be an elemeent of Vi. The splitting of pi by u, 
                // denoted by pi ⊥ u, is the equitable reﬁnement R(pi) of the ordered partition 
                // pi = (V1, V2, ..., {u}, Vi \ {u}, Vi+1, ..., Vr).
                std::copy(tau.begin(), tau.begin() + i, std::back_inserter(pi));
                pi.push_back(typename partition_type<Graph>::type::value_type(1, tau[i][j]));
                pi.push_back(typename partition_type<Graph>::type::value_type(tau[i].begin(), tau[i].begin() + j));
                if (j + 1 < tau[i].size())
                    std::copy(tau[i].begin() + j + 1, tau[i].end(), std::back_inserter(pi.back()));
                if (!pi.back().size())
                    pi.pop_back();
                if (i + 1 < tau.size())
                    std::copy(tau.begin() + i + 1, tau.end(), std::back_inserter(pi));
                equitable_refinement(pi, G);
                // To reduce the branching factor, McKay actually chooses the ﬁrst smallest part of pi. The
                // method of choosing the part for splitting pi is irrelevant as long as it is an isomorphism invariant
                // of unordered partitions. That is to say, that if the i-th part of pi is chosen, then also the i-th part
                // of pi^ny is chosen for any nu element of Sigma_n.
                std::size_t n = num_vertices(G) + 1;
                i = 0;
                for (typename partition_type<Graph>::type::iterator it = pi.begin(); it != pi.end(); ++it)
                    if (it->size() > 1 && n > it->size()) {
                        n = it->size();
                        i = it - pi.begin();
                    }
                if (n < num_vertices(G) + 1u) {
                    T.push_back(boost::make_tuple(typename partition_type<Graph>::type(), i, 0));
                    terminal_node(T, G);
                }
            }
            
            // The not colored graph label is a triangular bit matrix
            // Input: pi = (V1, V2, ..., Vr)
            // Output: comparable graph label l(pi)
            template<typename Graph> void apply_label_no_coloring (
                  typename graph_label<Graph>::type & l
                , typename partition_type<Graph>::type const & pi
                , Graph const & G
            ) {
                using boost::get;
                // N = #of parts of pi
                get<0>(l).clear();
                get<0>(l).resize(pi.size() * (pi.size() + 1) / 2);
                typename boost::graph_traits<Graph>::adjacency_iterator ai, ae;
                std::map<typename boost::graph_traits<Graph>::vertex_descriptor, std::size_t> I;
                // I = {(ni, j) : ni element of Vj
                partition_indeces(I, pi, G);
                for (typename std::map<typename boost::graph_traits<Graph>::vertex_descriptor, std::size_t>::const_iterator it = I.begin(); it != I.end(); ++it)
                    for (boost::tie(ai, ae) = adjacent_vertices(it->first, G); ai != ae; ++ai)
                        if (I[*ai] <= I[it->first])
                            get<0>(l)[I[*ai] * pi.size() - (I[*ai] - 1) * I[*ai] / 2 + I[it->first] - I[*ai]] = true;
            }
            


            namespace label_helpers {
                struct label_no_coloring_helper
                {
                    label_no_coloring_helper() {};
                    template <typename T>
                    label_no_coloring_helper(T const&) {};
                    template <typename T1, typename T2, typename T3>
                    inline void operator()(T1 const&, T2 const&, T3 const&) const {}
                };

                template <typename Graph>
                struct label_vertex_coloring_helper
                {
                    // Vertex colored graph label
                    // Input: pi = (V1, V2, ..., Vr)
                    // Output: comparable graph label l(pi)
                    // TODO: only one vertex per orbit in matrix
                    void operator() (
                          typename graph_label<Graph>::type & l
                        , typename partition_type<Graph>::type const & pi
                        , Graph const & G
                    ) const {
                        using boost::get;
                        std::set<typename boost::property_map<Graph, alps::vertex_type_t>::type::value_type> colors;
                        typename boost::graph_traits<Graph>::vertex_iterator it, end;
                        for (boost::tie(it, end) = vertices(G); it != end; ++it)
                            colors.insert(get(alps::vertex_type_t(), G)[*it]);
                        get<2>(l).clear();
                        for (
                              typename std::set<typename boost::property_map<Graph, alps::vertex_type_t>::type::value_type>::const_iterator jt = colors.begin()
                            ; jt != colors.end()
                            ; ++jt
                        )
                            get<2>(l).push_back(*jt);
                        get<1>(l).clear();
                        get<1>(l).resize(num_vertices(G) * get<2>(l).size());
                        // TODO: just make one row per orbit, not per vertex
                        std::size_t index = 0;
                        for (typename partition_type<Graph>::type::const_iterator jt = pi.begin(); jt != pi.end(); ++jt)
                            for (typename partition_type<Graph>::type::value_type::const_iterator kt = jt->begin(); kt != jt->end(); ++kt)
                                get<1>(l)[(std::find(get<2>(l).begin(), get<2>(l).end(), get(alps::vertex_type_t(), G)[*kt]) - get<2>(l).begin()) * num_vertices(G) + index++] = true;
                    }
                };


                template <typename Graph>
                struct label_edge_coloring_helper
                {
                  private:
                    typedef typename boost::property_map<Graph,alps::edge_type_t>::type::value_type color_type;
                    typedef typename color_partition<Graph>::type                                   color_partition_type;
                    typedef typename boost::graph_traits<Graph>::edge_descriptor                    edge_descriptor;

                    // Sort edges acoring to the given partition
                    struct apply_label_edge_comp {
                        // Input: pi = (V1, V2, ..., Vr)
                        apply_label_edge_comp (typename partition_type<Graph>::type const & pi, Graph const & g)
                            : G(g)
                        {
                            for (typename partition_type<Graph>::type::const_iterator it = pi.begin(); it != pi.end(); ++it)
                                for (typename partition_type<Graph>::type::value_type::const_iterator jt = it->begin(); jt != it->end(); ++jt)
                                    ordering.push_back(*jt);
                        }

                        // Comparison operator
                        bool operator() (
                              edge_descriptor const & i
                            , edge_descriptor const & j
                        ) {
                            std::size_t Ai, Bi, Aj, Bj;
                            Ai = std::find(ordering.begin(), ordering.end(), source(i, G)) - ordering.begin();
                            Bi = std::find(ordering.begin(), ordering.end(), target(i, G)) - ordering.begin();
                            Aj = std::find(ordering.begin(), ordering.end(), source(j, G)) - ordering.begin();
                            Bj = std::find(ordering.begin(), ordering.end(), target(j, G)) - ordering.begin();
                            return std::min(Ai, Bi) == std::min(Aj, Bj)
                                ? std::max(Ai, Bi) < std::max(Aj, Bj)
                                : std::min(Ai, Bi) < std::min(Aj, Bj)
                            ;
                        }

                        Graph const & G;
                        std::vector<typename boost::graph_traits<Graph>::vertex_descriptor> ordering;
                    };

                    void canonicalize_colors(
                          boost::container::flat_map<color_type, color_type> & color_map
                        , std::vector<edge_descriptor> & edge_list
                        , Graph const& G
                    ) const {
                        typedef boost::container::flat_map<color_type,color_type> color_map_type;

                        for(typename std::vector<edge_descriptor>::const_iterator jt = edge_list.begin(); jt != edge_list.end(); ++jt) {
                            typename color_map_type::iterator it = color_map.find(get(alps::edge_type_t(),G)[*jt]);
                            if(it == color_map.end()) {
                                color_type   const c  = get(alps::edge_type_t(),G)[*jt];
                                assert(color_partition_.find(c) != color_partition_.end());
                                unsigned int const cp = color_partition_.find(c)->second;
                                // Find a color of the same group which is not mapped to yet
                                bool found_mapping = false;
                                for(typename color_partition_type::const_iterator cit = color_partition_.begin(); cit != color_partition_.end(); ++cit) {
                                    if(cit->second == cp) {
                                        bool is_mapped_to = false;
                                        for(typename color_map_type::iterator cmit = color_map.begin(); cmit != color_map.end(); ++cmit) {
                                            if(cmit->second == cit->first)
                                                is_mapped_to = true;
                                        }
                                        if(!is_mapped_to) {
                                            color_map.insert(std::make_pair(c,cit->first));
                                            found_mapping = true;
                                            break;
                                        }
                                    }
                                }
                                assert(found_mapping);
                            }
                        }
                    }

                  public:
                    // Create a simple label_edge_coloring_helper
                    label_edge_coloring_helper(Graph const& G)
                    {
                        // Create an identity color symmetry partition...
                        typename boost::graph_traits<Graph>::edge_iterator it, end;
                        for (boost::tie(it, end) = edges(G); it != end; ++it)
                            color_partition_.insert(std::make_pair(get(alps::edge_type_t(), G)[*it], get(alps::edge_type_t(),G)[*it]));
                    }

                    // Create a label_edge_coloring_helper with a color symmetry partition
                    label_edge_coloring_helper(Graph const& G, color_partition_type const& color_partition)
                    : color_partition_(color_partition)
                    {
                        // Check if all edge colors occuring in the graph are also in the color_partition
                        typename boost::graph_traits<Graph>::edge_iterator it, end;
                        bool partitions_are_complete = true;
                        for (boost::tie(it, end) = edges(G); it != end; ++it)
                            partitions_are_complete = partitions_are_complete && color_partition_.find(get(alps::edge_type_t(),G)[*it]) != color_partition_.end();
                        assert(partitions_are_complete);
                    }

                    // Edge colored graph label
                    // Input: pi = (V1, V2, ..., Vr)
                    // Output: comparable graph label l(pi)
                    // TODO: only add one edge from orbit to orbit not all edges
                    void operator()(
                          typename graph_label<Graph>::type & l
                        , typename partition_type<Graph>::type const & pi
                        , Graph const & G
                    ) const {
                        using boost::get;
                        using std::copy;
                        static std::size_t const Base = boost::tuples::length<typename graph_label<Graph>::type>::value - 2;
                        typename boost::graph_traits<Graph>::edge_iterator it, end;
                        boost::tie(it, end) = edges(G);
                        std::vector<edge_descriptor> edge_list(it,end);

                        get<Base + 1>(l).clear();
                        get<Base + 1>(l).reserve(color_partition_.size());
                        typename color_partition_type::const_iterator cit(color_partition_.begin()), cend(color_partition_.end());
                        for(; cit != cend; ++cit)
                            get<Base + 1>(l).push_back(cit->first);

                        std::sort(edge_list.begin(), edge_list.end(), apply_label_edge_comp(pi, G));
                        boost::container::flat_map<color_type,color_type> color_map;
                        color_map.reserve(color_partition_.size());
                        canonicalize_colors(color_map, edge_list, G);
                        get<Base>(l).clear();
                        // TODO: just make one row per orbit, not per vertex
                        get<Base>(l).resize(num_edges(G) * get<Base + 1>(l).size());
                        for (typename std::vector<edge_descriptor>::const_iterator jt = edge_list.begin(); jt != edge_list.end(); ++jt)
                            get<Base>(l).set((std::find(get<Base + 1>(l).begin(), get<Base + 1>(l).end(), color_map[get(alps::edge_type_t(), G)[*jt]] ) - get<Base + 1>(l).begin()) * num_edges(G) + (jt - edge_list.begin()));
                    }

                  private:
                    color_partition_type color_partition_;
                };
            }

            // The graph label is a triangular bit matrix with a 
            // Input: pi = (V1, V2, ..., Vr)
            // Output: comparable graph label l(pi)
            template<typename Graph, typename LabelVertexColoringHelper, typename LabelEdgeColoringHelper> void assemble_label(
                  typename graph_label<Graph>::type & l
                , typename partition_type<Graph>::type const & pi
                , Graph const & G
                , LabelVertexColoringHelper const& apply_label_vertex_coloring
                , LabelEdgeColoringHelper const& apply_label_edge_coloring
            ) {
                apply_label_no_coloring(l, pi, G);
                apply_label_vertex_coloring(l, pi, G);
                apply_label_edge_coloring(l, pi, G);
            }

            // If an ni
            // Input: pi = (V1, V2, ..., Vr), Vi = (n1, n2, ..., nk), ni element of G
            //        tau = (W1, W2, ..., Wr), Wi = (m1, m2, ..., mk), mi element of G 
            //        orbit = (Q1, Q2, ..., Qr), Wi = (o1, o2, ..., ok), oi element of G 
            // Output: coarsed orbit
            template<typename Graph> void coarse_orbit(
                  typename partition_type<Graph>::type & orbit
                , std::map<typename boost::graph_traits<Graph>::vertex_descriptor, std::size_t> & I
                , typename partition_type<Graph>::type const & pi
                , typename partition_type<Graph>::type const & tau
                , Graph G
            ) {
                if (pi != tau)
                    for (typename partition_type<Graph>::type::const_iterator it = pi.begin(), jt = tau.begin(); it != pi.end(); ++it, ++jt)
                        // If Vi != Wi and {Qa: oj = n1, oj element of Qa, n1 element of Vi} != {Qb: oj = m1, oj element of Qb, m1 element of Wi} 
                        // merge Qa and Qb into Qa and remove Qb
                        if (*it != *jt && I[it->front()] != I[jt->front()]) {
                            std::size_t a = std::min(I[it->front()], I[jt->front()]);
                            std::size_t b = std::max(I[it->front()], I[jt->front()]);
                            std::copy(orbit[b].begin(), orbit[b].end(), std::back_inserter(orbit[std::min(I[it->front()], I[jt->front()])]));
                            std::sort(orbit[a].begin(), orbit[a].end());
                            orbit.erase(orbit.begin() + b);
                            detail::partition_indeces(I, orbit, G);
                        }
            }
            
            // uncolored initial partition
            template<typename Graph> void initial_partition(
                  Graph const & G
                , typename partition_type<Graph>::type & pi
                , boost::mpl::false_
            ) {
                pi.resize(1);
                typename boost::graph_traits<Graph>::vertex_iterator it, end;
                for (boost::tie(it, end) = vertices(G); it != end; ++it)
                    pi.front().push_back(*it);
            }

            // vertex colored initial partition
            template<typename Graph> void initial_partition(
                  Graph const & G
                , typename partition_type<Graph>::type & pi
                , boost::mpl::true_
            ) {
                std::set<typename boost::property_map<Graph, alps::vertex_type_t>::type::value_type> color_set;
                typename boost::graph_traits<Graph>::vertex_iterator it, end;
                for (boost::tie(it, end) = vertices(G); it != end; ++it)
                    color_set.insert(get(alps::vertex_type_t(), G)[*it]);
                std::vector<typename boost::property_map<Graph, alps::vertex_type_t>::type::value_type> color_vector;
                for (
                      typename std::set<typename boost::property_map<Graph, alps::vertex_type_t>::type::value_type>::const_iterator jt = color_set.begin()
                    ; jt != color_set.end()
                    ; ++jt
                )
                    color_vector.push_back(*jt);
                pi.resize(color_vector.size());    
                for (boost::tie(it, end) = vertices(G); it != end; ++it)
                    pi[std::find(color_vector.begin(), color_vector.end(), get(alps::vertex_type_t(), G)[*it]) - color_vector.begin()].push_back(*it);
            }

            // uncolored initial partition with symmetry breaking vertex v
            template<typename Graph> void initial_partition(
                  Graph const & G
                , typename partition_type<Graph>::type & pi
                , typename boost::graph_traits<Graph>::vertex_descriptor v
                , boost::mpl::false_
            ) {
                pi.resize(2);
                pi.back().push_back(v);
                typename boost::graph_traits<Graph>::vertex_iterator it, end;
                for (boost::tie(it, end) = vertices(G); it != end; ++it)
                    if (*it != v)
                        pi.front().push_back(*it);
            }

            // vertex colored initial partition with symmetry breaking vertex v
            template<typename Graph> void initial_partition(
                  Graph const & G
                , typename partition_type<Graph>::type & pi
                , typename boost::graph_traits<Graph>::vertex_descriptor v
                , boost::mpl::true_
            ) {
                std::set<typename boost::property_map<Graph, alps::vertex_type_t>::type::value_type> color_set;
                typename boost::graph_traits<Graph>::vertex_iterator it, end;
                for (boost::tie(it, end) = vertices(G); it != end; ++it)
                    color_set.insert(get(alps::vertex_type_t(), G)[*it]);
                std::vector<typename boost::property_map<Graph, alps::vertex_type_t>::type::value_type> color_vector;
                for (
                      typename std::set<typename boost::property_map<Graph, alps::vertex_type_t>::type::value_type>::const_iterator jt = color_set.begin()
                    ; jt != color_set.end()
                    ; ++jt
                )
                    color_vector.push_back(*jt);
                pi.resize(color_vector.size() + 1);    
                pi.back().push_back(v);
                for (boost::tie(it, end) = vertices(G); it != end; ++it)
                    if (*it != v)
                        pi[std::find(color_vector.begin(), color_vector.end(), get(alps::vertex_type_t(), G)[*it]) - color_vector.begin()].push_back(*it);
            }
        }

        namespace detail {

            // Input: graph G, inital partition pi
            // Output: canonical ordering, canonical label and orbit of G
            template<typename Graph, typename LabelVertexColoringHelper, typename LabelEdgeColoringHelper>
            typename canonical_properties_type<Graph>::type
            canonical_properties_impl(Graph const & G, typename partition_type<Graph>::type const pi, LabelVertexColoringHelper const& lvch, LabelEdgeColoringHelper const& lech) {
                using boost::get;
                using boost::make_tuple;
                typename partition_type<Graph>::type orbit, canonical_partition, first_partition;
                typename graph_label<Graph>::type canonical_label, first_label, current_label;
                std::map<typename boost::graph_traits<Graph>::vertex_descriptor, std::size_t> Io;
                // The orbit starts with a discrete partition (a partition with only trivial parts)
                // orbit = (W1, W2, ..., Wr), Wi = (m1, m2, ..., mk), mi element of G
                typename boost::graph_traits<Graph>::vertex_iterator vi, ve;
                for (boost::tie(vi, ve) = vertices(G); vi != ve; ++vi)
                    orbit.push_back(typename partition_type<Graph>::type::value_type(1, *vi));
                // Io = {(mi, j) : ni element of Vj
                detail::partition_indeces(Io, orbit, G);
                // Build first node of the search tree T(G)
                std::vector<boost::tuple<typename partition_type<Graph>::type, std::size_t, std::size_t> > T(1, boost::make_tuple(pi, 0, 0));
                detail::equitable_refinement(get<0>(T.back()), G);
                T.push_back(boost::make_tuple(typename partition_type<Graph>::type(), 0, 0));
                detail::terminal_node(T, G);
                // Initialize partitions and labels
                first_partition = canonical_partition = get<0>(T.back());
                detail::assemble_label(canonical_label, canonical_partition, G, lvch, lech);
                first_label = current_label = canonical_label;
                while(true) {
                    // Find next node in the search tree T(G). The last node is always a leaf.
                    while(T.size() > 1)
                        if (get<2>(T.back()) + 1 < get<0>(*(T.rbegin() + 1))[get<1>(T.back())].size()) {
                            ++get<2>(T.back());
                            // Prune the search tree: if we have already visited a branch strating at an element in the 
                            // same part of the orbit, skip the current element
                            typename partition_type<Graph>::type::value_type & part = get<0>(*(T.rbegin() + 1))[get<1>(T.back())];
                            bool visited = false;
                            for (
                                typename partition_type<Graph>::type::value_type::const_iterator it = part.begin(); 
                                it != part.begin() + get<2>(T.back()); 
                                ++it
                            )
                                visited = visited || (Io[*it] == Io[part[get<2>(T.back())]]);
                            if (!visited)
                                break;
                        } else
                            T.pop_back();
                    get<0>(T.back()).clear();
                    // all leafs have been checked
                    if (T.size() == 1)
                        break;
                    // TODO: add edge coloring to algorithym
                    detail::terminal_node(T, G);
                    detail::assemble_label(current_label, get<0>(T.back()), G, lvch, lech);
                    // If two labels are the same, coarse orbit
                    if(first_label == current_label)
                        detail::coarse_orbit(orbit, Io, first_partition, get<0>(T.back()), G);
                    else if(canonical_label == current_label)
                        detail::coarse_orbit(orbit, Io, canonical_partition, get<0>(T.back()), G);
                    // Cl = min{ Gl: Gl graph label of G }
                    if (canonical_label > current_label) {
                        canonical_partition = get<0>(T.back());
                        canonical_label = current_label;
                    }
                }
                
                std::vector<typename boost::graph_traits<Graph>::vertex_descriptor> canonical_ordering;
                for (typename partition_type<Graph>::type::const_iterator it = canonical_partition.begin(); it != canonical_partition.end(); ++it)
                    canonical_ordering.push_back((*it)[0]);
                return boost::make_tuple(
                      canonical_ordering
                    , canonical_label
                    , orbit
                );
            }
        }

        
        // McKay’s canonical isomorph function Cm(G) is deﬁned to be
        // Cm(G) = max{ Gpi: (pi, nu) is a leaf of T(G) }
        // Input: graph G
        // Output: canonical ordering, canonical label and orbit of G
        template<typename Graph>
        typename canonical_properties_type<Graph>::type
        canonical_properties(Graph const & G) {
            typename partition_type<Graph>::type pi;
            // pi = (V1, V2, ..., Vr), Vi = (n1, n2, ..., nk), ni element of G
            // uncolored graphs: pi is a unit partition (pi has only one part)
            // vertex colored graphs: pi has for each color one part
            detail::initial_partition(G, pi, boost::mpl::bool_<has_property<alps::vertex_type_t, Graph>::vertex_property>());
            typename boost::mpl::if_c<
                  has_property<alps::vertex_type_t, Graph>::vertex_property
                , detail::label_helpers::label_vertex_coloring_helper<Graph>
                , detail::label_helpers::label_no_coloring_helper
            >::type lvch;
            typename boost::mpl::if_c<
                  has_property<alps::edge_type_t, Graph>::edge_property
                , detail::label_helpers::label_edge_coloring_helper<Graph>
                , detail::label_helpers::label_no_coloring_helper
            >::type lech(G);
            // create canonical properties
            return detail::canonical_properties_impl(G, pi, lvch, lech);
        }

        // McKay’s canonical isomorph function Cm(G) is deﬁned to be
        // Cm(G) = max{ Gpi: (pi, nu) is a leaf of T(G) }
        // Input: graph G
        // Output: canonical ordering, canonical label and orbit of G
        template<typename Graph>
        typename canonical_properties_type<Graph>::type
        canonical_properties(Graph const & G, typename color_partition<Graph>::type const& c) {
            typename partition_type<Graph>::type pi;
            // pi = (V1, V2, ..., Vr), Vi = (n1, n2, ..., nk), ni element of G
            // uncolored graphs: pi is a unit partition (pi has only one part)
            // vertex colored graphs: pi has for each color one part
            detail::initial_partition(G, pi, boost::mpl::bool_<has_property<alps::vertex_type_t, Graph>::vertex_property>());

            // Build the label coloring helpers
            typename boost::mpl::if_c<
                  has_property<alps::vertex_type_t, Graph>::vertex_property
                , detail::label_helpers::label_vertex_coloring_helper<Graph>
                , detail::label_helpers::label_no_coloring_helper
            >::type lvch;
            typename boost::mpl::if_c<
                  has_property<alps::edge_type_t, Graph>::edge_property
                , detail::label_helpers::label_edge_coloring_helper<Graph>
                , detail::label_helpers::label_no_coloring_helper
            >::type lech(G,c);
            // create canonical properties
            return detail::canonical_properties_impl(G, pi, lvch, lech);
        }

        // McKay’s canonical isomorph function Cm(G) is deﬁned to be
        // Cm(G) = max{ Gpi: (pi, nu) is a leaf of T(G) }
        // Input: graph G, symmetry breaking vertex v
        // Output: canonical ordering, canonical label and orbit of G
        template<typename Graph>
        typename canonical_properties_type<Graph>::type
        canonical_properties(Graph const & G, typename boost::graph_traits<Graph>::vertex_descriptor v) {
            typename partition_type<Graph>::type pi;
            // pi = (V1, V2, ..., Vr), Vi = (n1, n2, ..., nk), ni element of G
            // uncolored graphs: pi is a unit partition (pi has only one part)
            // vertex colored graphs: pi has for each color one part
            detail::initial_partition(G, pi, v, boost::mpl::bool_<has_property<alps::vertex_type_t, Graph>::vertex_property>());
            typename boost::mpl::if_c<
                  has_property<alps::vertex_type_t, Graph>::vertex_property
                , detail::label_helpers::label_vertex_coloring_helper<Graph>
                , detail::label_helpers::label_no_coloring_helper
            >::type lvch;
            typename boost::mpl::if_c<
                  has_property<alps::edge_type_t, Graph>::edge_property
                , detail::label_helpers::label_edge_coloring_helper<Graph>
                , detail::label_helpers::label_no_coloring_helper
            >::type lech(G);
            // create canonical properties
            return detail::canonical_properties_impl(G, pi, lvch, lech);
        }
    }
}

#endif // ALPS_GRAPH_CANONICAL_PROPERTIES_HPP
