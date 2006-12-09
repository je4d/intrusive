/////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Olaf Krzikalla 2004-2006.
// (C) Copyright Ion Gazta�aga  2006.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/intrusive for documentation.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTRUSIVE_ISLIST_HOOK_HPP
#define BOOST_INTRUSIVE_ISLIST_HOOK_HPP

#include <boost/intrusive/detail/config_begin.hpp>
#include <boost/intrusive/detail/utilities.hpp>
#include <boost/intrusive/detail/pointer_type.hpp>
#include <boost/intrusive/detail/pointer_to_other.hpp>
#include <boost/intrusive/detail/slist_node.hpp>
#include <boost/intrusive/slist_algorithms.hpp>
#include <boost/intrusive/value_traits_type.hpp>
#include <boost/get_pointer.hpp>
#include <stdexcept>

namespace boost {
namespace intrusive {

//! Derive a class from islist_base_hook in order to store objects in 
//! in an islist. islist_base_hook holds the data necessary to maintain the 
//! list and provides an appropriate value_traits class for islist.
//! 
//! The first integer template argument defines a tag to identify the node. 
//! The same tag value can be used in different classes, but if a class is 
//! derived from more then one islist_base_hook, then each islist_base_hook needs its 
//! unique tag.
//!
//! The second boolean template argument SafeMode controls initializes
//! the node to a safe state in the constructor and asserts if the node is destroyed
//! or it's assigned but it's still inserted in a islist.
//!
//! The third argument is the pointer type that will be used internally in the hook
//! and the islist configured from this hook.
template<int Tag, bool SafeMode = true, class VoidPointer = void*>
class islist_base_hook
   :  private detail::slist_node_traits<VoidPointer>::node
{
   public:
   typedef detail::slist_node_traits<VoidPointer>     node_traits;
   enum { type = SafeMode? safe_mode_value_traits : normal_value_traits};

   private:
   typedef slist_algorithms<node_traits> sequence_algorithms;
   typedef typename node_traits::node                 node;
   typedef typename detail::pointer_to_other
      <VoidPointer, node>::type                       node_ptr;
   typedef typename detail::pointer_to_other
      <VoidPointer, const node>::type                 const_node_ptr;
   typedef islist_base_hook
      <Tag, SafeMode, VoidPointer>                    this_type;
   typedef typename detail::pointer_to_other
      <VoidPointer, this_type>::type                  this_type_ptr;
   typedef typename detail::pointer_to_other
      <VoidPointer, const this_type>::type            const_this_type_ptr;

   node_ptr this_as_node()
   {  return node_ptr(static_cast<node *const>(this)); }

   const_node_ptr this_as_node() const
   {  return const_node_ptr(static_cast<const node *const>(this)); }

   public:

   //! <b>Effects</b>: If SafeMode is true the node, such that it is not 
   //!   in a sequence.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Rationale</b>: Providing a default-constructor
   //!   makes classes using islist_base_hook STL-compliant without forcing the 
   //!   user to do some additional work.
   islist_base_hook()
      : node_traits::node()
   {
      if(SafeMode){
         sequence_algorithms::init(this_as_node());
      }
   }

   //! <b>Effects</b>: Initializes the node, such that it is not 
   //!   in a sequence. The argument is ignored.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Rationale</b>: Providing a copy-constructor
   //!   makes classes using islist_base_hook STL-compliant without forcing the 
   //!   user to do some additional work. "swap" can be used to emulate
   //!   move-semantics.
   islist_base_hook(const islist_base_hook& )
      :  node_traits::node()
   {
      if(SafeMode){
         sequence_algorithms::init(this_as_node());
      }
   }

   //! <b>Effects</b>: If safe-mode is true
   //!   Initializes the node, such that it is not 
   //!   in a sequence. The argument is ignored.
   //! 
   //! <b>Throws</b>: Nothing.
   //! 
   //! <b>Rationale</b>: Providing an assignment operator 
   //!   makes classes using islist_base_hook STL-compliant without forcing the 
   //!   user to do some additional work. "swap" can be used to emulate
   //!   move-semantics.
   islist_base_hook& operator=(const islist_base_hook& )
   { 
      if(SafeMode){
         BOOST_ASSERT(!this->linked());
      }
      return *this; 
   }

   //! <b>Effects</b>: If SafeMode is set to false, the destructor does
   //!   nothing (ie. no code is generated). If SafeMode is true and the
   //!   object is stored in an islist an assertion is raised.
   //! 
   //! <b>Throws</b>: Nothing.
   ~islist_base_hook() 
   { 
      if(SafeMode){
         BOOST_ASSERT(!this->linked());
      }
   }

   //! <b>Effects</b>: Swapping two nodes swaps the position of the elements 
   //!   related to that nodes in one or two sequences. That is, if the node 
   //!   this is part of the element e1, the node x is part of the element e2 
   //!   and both elements are included in the sequences s1 and s2, then after 
   //!   the swap-operation e1 is in s2 at the position of e2 and e2 is in s1 
   //!   at the position of e1. If one element is not in a sequence, then 
   //!   after the swap-operation the other element is not in a sequence. 
   //!   Iterators to e1 and e2 related to that nodes are invalidated. 
   //!
   //! <b>Complexity</b>: Linear
   //!
   //! <b>Throws</b>: Nothing. 
   void swap_nodes(islist_base_hook& other) 
   { sequence_algorithms::swap_nodes(this_as_node(), other.this_as_node()); }

   //! <b>Precondition</b>: The hook must be in safe-mode.
   //!
   //! <b>Returns</b>: true, if the node belongs to a sequence, false
   //!   otherwise. This function can be used to test whether islist::current 
   //!   will return a valid iterator. 
   //!
   //! <b>Complexity</b>: Constant
   bool linked() const 
   {
      //linked() can be only used in safe-mode
      BOOST_ASSERT(SafeMode);
      return !sequence_algorithms::unique(this_as_node());
   }

   //! The value_traits class is used as the first template argument for islist. 
   //! The template argument T defines the class type stored in islist. Objects 
   //! of type T and of types derived from T can be stored. T doesn't need to be 
   //! copy-constructible or assignable.
   template<class T>
   struct value_traits
      : detail::derivation_value_traits<T, this_type, Tag>
   {};

   //! <b>Effects</b>: Converts a pointer to a node stored in a sequence into
   //!   a pointer to the hook that holds that node.
   //! 
   //! <b>Throws</b>: Nothing.
   static this_type_ptr to_hook(node_ptr p)
   {
      using boost::get_pointer;
      return this_type_ptr(static_cast<islist_base_hook*> (get_pointer(p))); 
   }

   //! <b>Effects</b>: Converts a const pointer to a node stored in a sequence into
   //!   a const pointer to the hook that holds that node.
   //! 
   //! <b>Throws</b>: Nothing.
   static const_this_type_ptr to_hook(const_node_ptr p)
   {
      using boost::get_pointer;
      return this_type_ptr(static_cast<const islist_base_hook*> (get_pointer(p))); 
   }

   //! <b>Effects</b>: Returns a pointer to the node that this hook holds.
   //! 
   //! <b>Throws</b>: Nothing.
   node_ptr to_node_ptr()
   { return this_as_node(); }

   //! <b>Effects</b>: Returns a const pointer to the node that this hook holds.
   //! 
   //! <b>Throws</b>: Nothing.
   const_node_ptr to_node_ptr() const
   { return this_as_node(); }
};


//! Derive a class from islist_auto_base_hook in order to store objects in 
//! in an islist. islist_auto_base_hook holds the data necessary to maintain the 
//! list and provides an appropriate value_traits class for islist.
//!
//! The difference between islist_auto_base_hook and islist_base_hook is that
//! islist_auto_base_hook removes itself automatically from the container
//! in the assignment operator and the destructor. It also provides a new
//! "unlink_self" method so that the user can unlink its class without using
//! the container.
//!
//! islist_auto_base_hook can only be used with non constant-time islists because
//! only those lists can support the auto-unlink feature.
//! 
//! The first integer template argument defines a tag to identify the node. 
//! The same tag value can be used in different classes, but if a class is 
//! derived from more then one islist_auto_base_hook, then each islist_auto_base_hook needs its 
//! unique tag.
//!
//! The second argument is the pointer type that will be used internally in the hook
//! and the islist configured from this hook.
template<int Tag, class VoidPointer = void*>
class islist_auto_base_hook
   :  private detail::slist_node_traits<VoidPointer>::node
{
   public:
   typedef detail::slist_node_traits<VoidPointer>     node_traits;
   enum { type = auto_unlink_value_traits  };

   private:
   typedef slist_algorithms<node_traits> sequence_algorithms;
   typedef typename node_traits::node                 node;
   typedef typename detail::pointer_to_other
      <VoidPointer, node>::type                       node_ptr;
   typedef typename detail::pointer_to_other
      <VoidPointer, const node>::type                 const_node_ptr;
   typedef islist_auto_base_hook
      <Tag, VoidPointer>                              this_type;
   typedef typename detail::pointer_to_other
      <VoidPointer, this_type>::type                  islist_node_d_ptr;
   typedef typename detail::pointer_to_other
      <VoidPointer, const this_type>::type            const_this_type_ptr;

   node_ptr this_as_node()
   {  return node_ptr(static_cast<node *const>(this)); }

   const_node_ptr this_as_node() const
   {  return const_node_ptr(static_cast<const node *const>(this)); }

   public:

   //! <b>Effects</b>: Initializes the node, such that it is not 
   //!   in a sequence.
   //! 
   //! <b>Throws</b>: Nothing. 
   //! 
   //! <b>Rationale</b>: Providing a default-constructor
   //!   makes classes using islist_auto_base_hook STL-compliant without forcing the 
   //!   user to do some additional work.
   islist_auto_base_hook()
      : node_traits::node()
   {  sequence_algorithms::init(this_as_node());   }

   //! <b>Effects</b>: Initializes the node, such that it is not 
   //!   in a sequence. The argument is ignored.
   //! 
   //! <b>Throws</b>: Nothing. 
   //! 
   //! <b>Rationale</b>: Providing a copy-constructor
   //!   makes classes using islist_auto_base_hook STL-compliant without forcing the 
   //!   user to do some additional work.
   islist_auto_base_hook(const islist_auto_base_hook& )
      :  node_traits::node()
   {  sequence_algorithms::init(this_as_node());   }

   //! <b>Effects</b>: Removes the node if it's inserted in a sequence.
   //!   The argument is ignored.
   //! 
   //! <b>Throws</b>: Nothing. 
   //! 
   //! <b>Rationale</b>: Providing an assignment operator 
   //!   makes classes using islist_auto_base_hook STL-compliant without forcing the 
   //!   user to do some additional work.
   islist_auto_base_hook& operator=(const islist_auto_base_hook& )
   {  this->unlink_self();  }

   //! <b>Effects</b>: Removes the node if it's inserted in a sequence.
   //! 
   //! <b>Throws</b>: Nothing. 
   ~islist_auto_base_hook() 
   {  this->unlink_self();  }

   //! <b>Effects</b>: Swapping two nodes swaps the position of the elements 
   //!   related to that nodes in one or two sequences. That is, if the node 
   //!   this is part of the element e1, the node x is part of the element e2 
   //!   and both elements are included in the sequences s1 and s2, then after 
   //!   the swap-operation e1 is in s2 at the position of e2 and e2 is in s1 
   //!   at the position of e1. If one element is not in a sequence, then 
   //!   after the swap-operation the other element is not in a sequence. 
   //!   Iterators to e1 and e2 related to that nodes are invalidated. 
   //!
   //! <b>Complexity</b>: Linear 
   //!
   //! <b>Throws</b>: Nothing.
   void swap_nodes(islist_auto_base_hook& other) 
   { sequence_algorithms::swap_nodes(this_as_node(), other.this_as_node()); }

   //! <b>Returns</b>: true, if the node belongs to a sequence, false
   //!   otherwise. This function can be used to test whether islist::current 
   //!   will return a valid iterator. 
   //!
   //! <b>Complexity</b>: Constant 
   bool linked() const 
   {  return !sequence_algorithms::unique(this_as_node());  }

   //! <b>Effects</b>: Removes the node if it's inserted in a sequence.
   //! 
   //! <b>Throws</b>: Nothing. 
   void unlink_self()
   {
      sequence_algorithms::unlink(this_as_node());
      sequence_algorithms::init(this_as_node());
   }

   //! The value_traits class is used as the first template argument for islist. 
   //! The template argument T defines the class type stored in islist. Objects 
   //! of type T and of types derived from T can be stored. T doesn't need to be 
   //! copy-constructible or assignable.
   template<class T>
   struct value_traits
      : detail::derivation_value_traits<T, this_type, Tag>
   {};

   //! <b>Effects</b>: Converts a pointer to a node stored in a sequence into
   //!   a pointer to the hook that holds that node.
   //! 
   //! <b>Throws</b>: Nothing. 
   static islist_node_d_ptr to_hook(node_ptr p)
   {
      using boost::get_pointer;
      return islist_node_d_ptr(static_cast<islist_auto_base_hook*> (get_pointer(p))); 
   }

   //! <b>Effects</b>: Converts a const pointer to a node stored in a sequence into
   //!   a const pointer to the hook that holds that node.
   //! 
   //! <b>Throws</b>: Nothing. 
   static const_this_type_ptr to_hook(const_node_ptr p)
   {
      using boost::get_pointer;
      return islist_node_d_ptr(static_cast<const islist_auto_base_hook*> (get_pointer(p))); 
   }

   //! <b>Effects</b>: Returns a pointer to the node that this hook holds.
   //! 
   //! <b>Throws</b>: Nothing.
   node_ptr to_node_ptr()
   { return this_as_node(); }

   //! <b>Effects</b>: Returns a const pointer to the node that this hook holds.
   //! 
   //! <b>Throws</b>: Nothing.
   const_node_ptr to_node_ptr() const
   { return this_as_node(); }
};


//! Put a public data member islist_member_hook in order to store objects of this class in
//! an islist. islist_member_hook holds the data necessary for maintaining the list and 
//! provides an appropriate value_traits class for islist.
//! 
//! The template argument T defines the class type stored in islist. Objects of type 
//! T and of types derived from T can be stored. T doesn't need to be 
//! copy-constructible or assignable.
//! 
//! The second boolean template argument SafeMode controls initializes
//! the node to a safe state in the constructor and asserts if the node is destroyed
//! or it's assigned but it's still inserted in a islist.
//!
//! The third argument is the pointer type that will be used internally in the hook
//! and the islist configured from this hook.
template<class T, bool SafeMode = true, class VoidPointer = void*>
class islist_member_hook 
   :  private detail::slist_node_traits<VoidPointer>::node
{
   public:
   typedef detail::slist_node_traits<VoidPointer>     node_traits;
   enum { type = SafeMode? safe_mode_value_traits : normal_value_traits};

   private:
   typedef typename detail::pointer_to_other
      <VoidPointer, T>::type                          pointer;
   typedef slist_algorithms<node_traits>      sequence_algorithms;
   typedef typename node_traits::node                 node;
   typedef typename detail::pointer_to_other
      <VoidPointer, node>::type                       node_ptr;
   typedef typename detail::pointer_to_other
      <VoidPointer, const node>::type                 const_node_ptr;
   typedef islist_member_hook<T, SafeMode, VoidPointer> this_type;
   typedef typename detail::pointer_to_other
      <VoidPointer, this_type >::type                 this_type_ptr;
   typedef typename detail::pointer_to_other
      <VoidPointer, const this_type >::type           const_this_type_ptr;

   node_ptr this_as_node()
   {  return node_ptr(static_cast<node *const>(this)); }

   const_node_ptr this_as_node() const
   {  return const_node_ptr(static_cast<const node *const>(this)); }

   public:
   //! <b>Effects</b>: If SafeMode is true the node, such that it is not 
   //!   in a sequence.
   //! 
   //! <b>Throws</b>: Nothing. 
   //! 
   //! <b>Rationale</b>: Providing a default-constructor
   //!   makes classes using islist_member_hook STL-compliant without forcing the 
   //!   user to do some additional work.
   islist_member_hook()
      :  node_traits::node()
   {
      if(SafeMode){
         sequence_algorithms::init(this_as_node());
      }
   }

   //! <b>Effects</b>: Initializes the node, such that it is not 
   //!   in a sequence. The argument is ignored.
   //! 
   //! <b>Throws</b>: Nothing. 
   //! 
   //! <b>Rationale</b>: Providing a copy-constructor
   //!   makes classes using islist_member_hook STL-compliant without forcing the 
   //!   user to do some additional work. "swap" can be used to emulate
   //!   move-semantics.
   islist_member_hook(const islist_member_hook& )
      :  node_traits::node()
   {
      if(SafeMode){
         sequence_algorithms::init(this_as_node());
      }
   }

   //! <b>Effects</b>: If safe-mode is true
   //!   Initializes the node, such that it is not 
   //!   in a sequence. The argument is ignored.
   //! 
   //! <b>Throws</b>: Nothing. 
   //! 
   //! <b>Rationale</b>: Providing an assignment operator 
   //!   makes classes using islist_member_hook STL-compliant without forcing the 
   //!   user to do some additional work. "swap" can be used to emulate
   //!   move-semantics.
   islist_member_hook& operator=(const islist_member_hook& ) 
   { 
      if(SafeMode){
         BOOST_ASSERT(!this->linked());
      }
      return *this; 
   }

   //! <b>Effects</b>: If SafeMode is set to false, the destructor does
   //!   nothing (ie. no code is generated). If SafeMode is true and the
   //!   object is stored in an islist an assertion is raised.
   //! 
   //! <b>Throws</b>: Nothing. 
   ~islist_member_hook() 
   { 
      if(SafeMode){
         BOOST_ASSERT(!this->linked());
      }
   }

   //! <b>Effects</b>: Swapping two nodes swaps the position of the elements 
   //!   related to that nodes in one or two sequences. That is, if the node 
   //!   this is part of the element e1, the node x is part of the element e2 
   //!   and both elements are included in the sequences s1 and s2, then after 
   //!   the swap-operation e1 is in s2 at the position of e2 and e2 is in s1 
   //!   at the position of e1. If one element is not in a sequence, then 
   //!   after the swap-operation the other element is not in a sequence. 
   //!   Iterators to e1 and e2 related to that nodes are invalidated. 
   //!
   //! <b>Complexity</b>: Linear 
   //!
   //! <b>Throws</b>: Nothing.
   void swap_nodes(islist_member_hook& other) 
   { sequence_algorithms::swap_nodes(this_as_node(), other.this_as_node()); }

   //! <b>Precondition</b>: The hook must be in safe-mode.
   //!
   //! <b>Returns</b>: true, if the node belongs to a sequence, false
   //!   otherwise. This function can be used to test whether islist::current 
   //!   will return a valid iterator. 
   //!
   //! <b>Complexity</b>: Constant
   bool linked() const 
   {
      //We must be in safe-mode to know if we are really linked
      //Otherwise, this would lead to an unknown state
      BOOST_ASSERT(SafeMode);
      return !sequence_algorithms::unique(this_as_node()); 
   }

   //! The value_traits class is used as the first template argument for islist. 
   //! The template argument is a pointer to member pointing to the node in 
   //! the class. Objects of type T and of types derived from T can be stored. 
   //! T doesn't need to be copy-constructible or assignable.
   template<this_type T::* M>
   struct value_traits
      : detail::member_value_traits<T, this_type, M>
   {};

   //! <b>Effects</b>: Converts a pointer to a node stored in a sequence into
   //!   a pointer to the hook that holds that node.
   //! 
   //! <b>Throws</b>: Nothing. 
   static this_type_ptr to_hook(node_ptr p)
   {
      using boost::get_pointer;
      return this_type_ptr(static_cast<this_type*> (get_pointer(p))); 
   }

   //! <b>Effects</b>: Converts a const pointer to a node stored in a sequence into
   //!   a const pointer to the hook that holds that node.
   //! 
   //! <b>Throws</b>: Nothing. 
   static const_this_type_ptr to_hook(const_node_ptr p)
   {
      using boost::get_pointer;
      return this_type_ptr(static_cast<this_type*> (get_pointer(p))); 
   }

   //! <b>Effects</b>: Returns a pointer to the node that this hook holds.
   //! 
   //! <b>Throws</b>: Nothing. 
   node_ptr to_node_ptr()
   { return this_as_node(); }

   //! <b>Effects</b>: Returns a const pointer to the node that this hook holds.
   //! 
   //! <b>Throws</b>: Nothing. 
   const_node_ptr to_node_ptr() const
   { return this_as_node(); }
};


//! Put a public data member islist_auto_member_hook in order to store objects of this class in
//! an islist. islist_auto_member_hook holds the data necessary for maintaining the list and 
//! provides an appropriate value_traits class for islist.
//!
//! The difference between islist_auto_member_hook and islist_auto_member_hook is that
//! islist_auto_member_hook removes itself automatically from the container
//! in the assignment operator and the destructor. It also provides a new
//! "unlink_self" method so that the user can unlink its class without using
//! the container.
//!
//! islist_auto_member_hook can only be used with non constant-time islists because
//! only those lists can support the auto-unlink feature.
//! 
//! The first template argument T defines the class type stored in islist. Objects of
//! type T and of types derived from T can be stored. T doesn't need to be 
//! copy-constructible or assignable.
//!
//! The second argument is the pointer type that will be used internally in the hook
//! and the islist configured from this hook.
template<class T, class VoidPointer = void*>
class islist_auto_member_hook 
   :  private detail::slist_node_traits<VoidPointer>::node
{
   public:
   typedef detail::slist_node_traits<VoidPointer>     node_traits;
   enum { type = auto_unlink_value_traits  };

   private:
   typedef typename detail::pointer_to_other
      <VoidPointer, T>::type                          pointer;
   typedef slist_algorithms<node_traits>      sequence_algorithms;
   typedef typename node_traits::node                 node;
   typedef typename detail::pointer_to_other
      <VoidPointer, node>::type                       node_ptr;
   typedef typename detail::pointer_to_other
      <VoidPointer, const node>::type                 const_node_ptr;
   typedef islist_auto_member_hook<T, VoidPointer>    this_type;
   typedef typename detail::pointer_to_other
      <VoidPointer, this_type >::type                 this_type_ptr;
   typedef typename detail::pointer_to_other
      <VoidPointer, const this_type >::type           const_this_type_ptr;

   node_ptr this_as_node()
   {  return node_ptr(static_cast<node *const>(this)); }

   const_node_ptr this_as_node() const
   {  return const_node_ptr(static_cast<const node *const>(this)); }

   public:
   //! <b>Effects</b>: Initializes the node, such that it is not 
   //!   in a sequence.
   //! 
   //! <b>Throws</b>: Nothing. 
   //! 
   //! <b>Rationale</b>: Providing a default-constructor
   //!   makes classes using islist_auto_base_hook STL-compliant without forcing the 
   //!   user to do some additional work.
   islist_auto_member_hook()
      :  node_traits::node()
   {  sequence_algorithms::init(this_as_node());   }

   //! <b>Effects</b>: Initializes the node, such that it is not 
   //!   in a sequence. The argument is ignored.
   //! 
   //! <b>Throws</b>: Nothing. 
   //! 
   //! <b>Rationale</b>: Providing a copy-constructor
   //!   makes classes using islist_auto_base_hook STL-compliant without forcing the 
   //!   user to do some additional work.
   islist_auto_member_hook(const islist_auto_member_hook& )
      :  node_traits::node()
   {  sequence_algorithms::init(this_as_node());   }

   //! <b>Effects</b>: Removes the node if it's inserted in a sequence.
   //!   The argument is ignored.
   //! 
   //! <b>Throws</b>: Nothing. 
   //! 
   //! <b>Rationale</b>: Providing an assignment operator 
   //!   makes classes using islist_auto_base_hook STL-compliant without forcing the 
   //!   user to do some additional work.
   islist_auto_member_hook& operator=(const islist_auto_member_hook& ) 
   {  this->unlink_self();  }

   //! <b>Effects</b>: If SafeMode is set to false, the destructor does
   //!   nothing (ie. no code is generated). If SafeMode is true and the
   //!   object is stored in an islist an assertion is raised.
   //! 
   //! <b>Throws</b>: Nothing. 
   ~islist_auto_member_hook() 
   {  this->unlink_self();  }

   //! <b>Effects</b>: Swapping two nodes swaps the position of the elements 
   //!   related to that nodes in one or two sequences. That is, if the node 
   //!   this is part of the element e1, the node x is part of the element e2 
   //!   and both elements are included in the sequences s1 and s2, then after 
   //!   the swap-operation e1 is in s2 at the position of e2 and e2 is in s1 
   //!   at the position of e1. If one element is not in a sequence, then 
   //!   after the swap-operation the other element is not in a sequence. 
   //!   Iterators to e1 and e2 related to that nodes are invalidated. 
   //!
   //! <b>Complexity</b>: Constant 
   //!
   //! <b>Throws</b>: Nothing.
   void swap_nodes(islist_auto_member_hook& other) 
   { sequence_algorithms::swap_nodes(this_as_node(), other.this_as_node()); }

   //! <b>Returns</b>: true, if the node belongs to a sequence, false
   //!   otherwise. This function can be used to test whether islist::current 
   //!   will return a valid iterator. 
   //!
   //! <b>Complexity</b>: Constant 
   bool linked() const 
   {  return !sequence_algorithms::unique(this_as_node());  }

   //! The value_traits class is used as the first template argument for islist. 
   //! The template argument is a pointer to member pointing to the node in 
   //! the class. Objects of type T and of types derived from T can be stored. 
   //! T doesn't need to be copy-constructible or assignable.
   template<this_type T::* M>
   struct value_traits
      : detail::member_value_traits<T, this_type, M>
   {};

   //! <b>Effects</b>: Removes the node if it's inserted in a sequence.
   //! 
   //! <b>Throws</b>: Nothing. 
   void unlink_self()
   {
      sequence_algorithms::unlink(this_as_node());
      sequence_algorithms::init(this_as_node());
   }

   //! <b>Effects</b>: Converts a pointer to a node stored in a sequence into
   //!   a pointer to the hook that holds that node.
   //! 
   //! <b>Throws</b>: Nothing. 
   static this_type_ptr to_hook(node_ptr p)
   {
      using boost::get_pointer;
      return this_type_ptr(static_cast<this_type*> (get_pointer(p))); 
   }

   //! <b>Effects</b>: Converts a const pointer to a node stored in a sequence into
   //!   a const pointer to the hook that holds that node.
   //! 
   //! <b>Throws</b>: Nothing. 
   static const_this_type_ptr to_hook(const_node_ptr p)
   {
      using boost::get_pointer;
      return const_this_type_ptr(static_cast<const this_type*> (get_pointer(p))); 
   }

   //! <b>Effects</b>: Returns a pointer to the node that this hook holds.
   //! 
   //! <b>Throws</b>: Nothing. 
   node_ptr to_node_ptr()
   { return this_as_node(); }

   //! <b>Effects</b>: Returns a const pointer to the node that this hook holds.
   //! 
   //! <b>Throws</b>: Nothing. 
   const_node_ptr to_node_ptr() const
   { return this_as_node(); }
};

/*
//! This class template is used as an value_traits template parameters for islist. 
//! The first template argument is the type T, which defines the class type
//! stored in islist. Objects of type T and of types derived from T can be stored.
//! T doesn't need to be copy-constructible or assignable.
//!
//! <b>Effects</b>: Prev and Next are used to identify the pointers pointing to 
//!   the previous and next element.
//!
//! <b>Requires</b>: T must have two data members that are pointers pointing to 
//!   T (and they must be accessible for member). In addition, T must be default 
//!   constructible, since internally islist uses a T as header of the list.
//!
//! <b>Notes</b>: Never directly touch the two pointers used by islist. If you need 
//!   to initialize them, use init_node. For further information see the description
//!   of init_node and remove_node.

template<class T
        ,class PointerToT
        ,PointerToT T::* Next
        ,bool SafeMode = true>
struct islist_ptr_to_member
{
   typedef T            value_type;
   typedef PointerToT   pointer;
   enum { SafeMode = SafeMode };

   private:
   typedef PointerToT                        node_ptr;
   typedef typename detail::pointer_to_other
      <PointerToT, const T>::type            const_node_ptr;
   typedef const_node_ptr                    const_pointer;

   public:
   struct node_traits
   {
      typedef T                                 node;
      typedef typename detail::pointer_to_other
         <PointerToT, void>::type               void_pointer;

      private:
      typedef PointerToT                        node_ptr;
      typedef typename detail::pointer_to_other
         <PointerToT, const T>::type            const_node_ptr;

      public:
      static node_ptr get_next(const_node_ptr n)
      {
         //Can we overload "operator ->*" for smart pointers?
         //return n->*Next;
         using boost::get_pointer;
         return get_pointer(n)->*Next;
      }

      static void set_next(node_ptr n, node_ptr next)
      {
         //Can we overload "operator ->*" for smart pointers?
         //return n->*Next;
         using boost::get_pointer;
         get_pointer(n)->*Next = next;
      }
   };
  
   static node_ptr to_node_ptr(value_type& v) 
   {  return &v;  }

   static const_node_ptr to_node_ptr(const value_type& v) 
   {  return &v;  }
  
   static pointer to_value_ptr(node_ptr n)
   {  return n;  }

   static const_pointer to_value_ptr(const_node_ptr n)
   {  return n;  }

   static void unlink_node(typename node_traits::node& data)
   {  slist_algorithms<node_traits>::unlink(&data); }

   static void init_node(typename node_traits::node& data)
   {  slist_algorithms<node_traits>::init(&data);  }  

   static bool linked(typename node_traits::node& data)
   {  slist_algorithms<node_traits>::unique(&data);  }  

   static void swap_nodes(T& elem1, T& elem2) 
   {  slist_algorithms<node_traits>::swap_nodes(to_node_ptr(elem1), to_node_ptr(elem2)); }
};
*/
} //namespace intrusive 
} //namespace boost 

#include<boost/intrusive/detail/config_end.hpp>

#endif //BOOST_INTRUSIVE_ISLIST_HOOK_HPP
