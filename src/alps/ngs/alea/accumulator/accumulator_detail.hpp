/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                 *
 * ALPS Project: Algorithms and Libraries for Physics Simulations                  *
 *                                                                                 *
 * ALPS Libraries                                                                  *
 *                                                                                 *
 * Copyright (C) 2011 - 2012 by Lukas Gamper <gamperl@gmail.com>                   *
 *                              Mario Koenz <mkoenz@ethz.ch>                       *
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

//The whole file holds the meta-template code to calculate, 
//from what base_types one needs to derive in order to get the 
//requested feautures


#ifndef ALPS_NGS_ALEA_DETAIL_ACCUMULATOR_DETAIL_HEADER
#define ALPS_NGS_ALEA_DETAIL_ACCUMULATOR_DETAIL_HEADER

#include <alps/ngs/alea/features.hpp>

#include <boost/static_assert.hpp>
#include <boost/parameter.hpp>
#include <boost/type_traits.hpp>
#include <boost/utility.hpp>

#include <iostream>

namespace alps
{
    namespace alea
    {
        template<typename T> 
        struct ValueType {};


        namespace detail
        {
        // = = = = = = M E T A   T E M P A L T E   L I S T = = = = = = = = = = =
            template<
                    typename stored_type
                  , typename next_list_item
                  > 
            struct ListItem 
            {
                typedef stored_type type;
                typedef next_list_item next;
            };
            
            struct ListEnd {};  //is used to mark the end of a list

        // = = = = = = = R E M O V E   V O I D   I N   L I S T = = = = = = = = = =
            template<typename list> 
            struct RemoveVoid 
            {
                typedef list type;
            };
            
            template<
                    typename stored_type
                  , typename next_list_item
                  > 
            struct RemoveVoid<
                            ListItem<stored_type, next_list_item> 
                             > 
            {
                typedef ListItem< stored_type
                                , typename RemoveVoid<next_list_item>::type
                                > type;
            };
            
            template<typename next_list_item> 
            struct RemoveVoid<ListItem<void, next_list_item> > 
            {
                typedef typename RemoveVoid<next_list_item>::type type;
            };

        // = = = = = = C O N S T R U C T   L I S T = = = = = = = = = = =
            template<
                  typename _0  = void
                , typename _1  = void
                , typename _2  = void
                , typename _3  = void
                , typename _4  = void
                , typename _5  = void
                , typename _6  = void
                , typename _7  = void
                , typename _8  = void
                , typename _9  = void
            > struct MakeList 
            {
                typedef typename RemoveVoid<
                        ListItem<_0, 
                         ListItem<_1, 
                          ListItem<_2, 
                           ListItem<_3, 
                            ListItem<_4, 
                             ListItem<_5, 
                              ListItem<_6, 
                               ListItem<_7,
                                ListItem<_8, 
                                 ListItem<_9, 
                                  ListEnd
                         > > > > > > > > > >
                        >::type type;
            };

        // = = = = = = = C O N C A T   T W O   L I S T S = = = = = = = = = =
            template <
                    typename list1
                  , typename list2
                  > 
            struct ConcatinateLists 
            {
                typedef ListItem< typename list1::type
                                , typename ConcatinateLists<
                                                            typename list1::next
                                                          , list2
                                                          >::type 
                                > type;
            };
            
            template <typename list2> 
            struct ConcatinateLists<ListEnd, list2> 
            {
                typedef list2 type;
            };

        // = = = = = = = U N I Q U E   L I S T   W A L K E R = = = = = = = = = =
            //walks through the list and eliminates target
            template <
                      typename target
                    , typename list
                    > 
            struct UniqueListWalker 
            {
                typedef ListItem< 
                                typename list::type
                              , typename UniqueListWalker<
                                                        target
                                                        , typename list::next
                                                        >::type 
                                > type;
            };
            
            template <
                      typename target
                    , typename list
                    > 
            struct UniqueListWalker<
                                      target
                                    , ListItem<target, list> 
                                   > 
            {
                typedef typename UniqueListWalker<target, list>::type type;
            };
            
            template <typename target> 
            struct UniqueListWalker<
                                    target
                                  , ListEnd
                                  >
            {
                typedef ListEnd type;
            };
        // = = = = = = U N I Q U E   L I S T = = = = = = = = = = =
        //bc ValueType is at first position one uses typename list::type and after that UniqueList
            template <typename list> struct UniqueList 
            {
                typedef ListItem<
                                typename list::type
                              , typename UniqueList<
                                                    typename UniqueListWalker<
                                                                            typename list::type
                                                                          , typename list::next
                                                                          >::type
                                                    >::type
                > type;
            };
            
            template <> 
            struct UniqueList<ListEnd> 
            {
                typedef ListEnd type;
            };
            
        // = = = = = = = F I N D   V A L U E   T Y P E = = = = = = = = = =
            template<typename list> 
            struct FindValueType 
            {
                typedef typename FindValueType<typename list::next>::type type;
            };
            
            template<
                      typename stored_type
                    , typename next_list_item
                    > 
            struct FindValueType<
                                  ListItem<ValueType<stored_type>
                                         , next_list_item> 
                                > 
            {
                typedef ValueType<stored_type> type;
            };
            
            template<> //no value-type found
            struct FindValueType<ListEnd> {
                BOOST_STATIC_ASSERT_MSG(true, "No ValueType added!");
            };
            
            //takes a list and frontInserts the ValueType 
            template<typename list> 
            struct ValueTypeFirst 
            {
                typedef ListItem<typename FindValueType<list>::type, list> type;
            };


        // = = = = = = = D E P E N D E N C I E S = = = = = = = = = =
            template<typename T> struct Dependencies //trait that is overloaded for each properties
            {
                typedef MakeList<>::type type;
            };

        // = = = = = = = = R E S O L V E   D E P E N D E N C I E S = = = = = = = = =
            template <typename list> 
            struct ResolveDependencies 
            {
                typedef typename ConcatinateLists<
                                                typename ResolveDependencies< //resolve dependencies of the dependencies
                                                                            typename Dependencies<typename list::type>::type
                                                                            >::type
                                              , ListItem<
                                                        typename list::type, 
                                                        typename ResolveDependencies<typename list::next>::type
                                                        >
                                                >::type type;
            };
            
            template <> 
            struct ResolveDependencies<ListEnd> 
            {
                typedef ListEnd type;
            };

        // = = = = = = I M P L E M E N T A T I O N   T Y P E = = = = = = = = = = =
        //every property specializes this template and contains the wanted implementation
            template<typename property, typename base_type>
            struct Implementation {};

        // = = = = = = D E R I V E   F R O M   I M P L E M E N T A T I O N S = = = = = = = = = = =
            template<
                      typename list
                    , typename base_type
                    > 
            struct DeriveProperties 
            {
                typedef typename DeriveProperties<
                                                typename list::next
                                                , Implementation<typename list::type, base_type> //the base_type is expanded here
                                                >::type type;
            };
            
            template<typename base_type> 
            struct DeriveProperties<ListEnd, base_type> 
            {
                typedef base_type type; //here base_type will be Implementation<property1, Implementation<property2, ... Implementation<propertyN, UselessBase> > >
            };

            struct UselessBase {};
        // = = = = = = = A C C U M U L A T O R _ I M P L= = = = = = = = = =
            
            template<
                  typename _0  = void
                , typename _1  = void
                , typename _2  = void
                , typename _3  = void
                , typename _4  = void
                , typename _5  = void
                , typename _6  = void
                , typename _7  = void
                , typename _8  = void
                , typename _9  = void
            > 
            struct accumulator_impl : public DeriveProperties<
                                                            typename UniqueList<
                                                                typename ResolveDependencies<
                                                                        typename ValueTypeFirst<
                                                                            typename MakeList<_0, _1, _2, _3, _4, _5, _6, _7, _8, _9>::type
                                                                                               >::type
                                                                                             >::type
                                                                                >::type
                                                          , UselessBase
                                                            >::type 
                {
                    //typename it for shorter syntax
                    typedef typename DeriveProperties<
                                typename UniqueList<
                                    typename ResolveDependencies<
                                        typename ValueTypeFirst<
                                            typename MakeList<_0, _1, _2, _3, _4, _5, _6, _7, _8, _9>::type
                                            >::type
                                        >::type
                                    >::type,UselessBase
                                >::type base_type;
                    
                    //disable_if is required bc of the named parameter. This template shouldn't act as a copy-ctor
                    template <typename ArgumentPack>
                    accumulator_impl(
                                    ArgumentPack const & args
                                  , typename boost::disable_if<
                                                              boost::is_base_of<accumulator_impl
                                                                            , ArgumentPack>
                                                            , int
                                                            >::type = 0
                                    ): base_type(args) {}
                    
                    //copy-ctor
                    accumulator_impl(accumulator_impl const & arg): base_type(arg) {}
                
            };
            
        // = = = = = = S T R E A M   O P E R A T O R = = = = = = = = = = =
            template<
                  typename _0
                , typename _1
                , typename _2
                , typename _3
                , typename _4
                , typename _5
                , typename _6
                , typename _7
                , typename _8
                , typename _9
            > 
            inline std::ostream & operator <<(std::ostream & os, accumulator_impl<_0, _1, _2, _3, _4, _5, _6, _7, _8, _9> & a)
            {
                a.print(os);
                return os;
            }
        } // end namespace detail
    }//end alea namespace 
}//end alps namespace
#endif // ALPS_NGS_ALEA_DETAIL_ACCUMULATOR_DETAIL_HEADER
