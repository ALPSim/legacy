/***************************************************************************
* ALPS++/osiris library
*
* alps/osiris/dump.h      dumps for object serialization
*
* $Id$
*
* Copyright (C) 1994-2003 by Matthias Troyer <troyer@itp.phys.ethz.ch>,
*                            Synge Todo <wistaria@comp-phys.org>,
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
**************************************************************************/

#ifndef ALPS_OSIRIS_DUMP_H
#define ALPS_OSIRIS_DUMP_H

#include <alps/config.h>
#include <boost/smart_ptr.hpp>
#include <complex>
#include <iostream>
#include <map>
#include <rpc/types.h>
#include <typeinfo>
#include <vector>

namespace alps {

class ODump
{
public:
  ODump(uint32_t v = 0);
  virtual ~ODump() {}

  uint32_t version() const { return version_; } 
  
# define ALPS_DUMP_DO_TYPE(T) \
  virtual void write_simple(T x);
  ALPS_DUMP_DO_TYPE(bool)
  ALPS_DUMP_DO_TYPE(int8_t)
  ALPS_DUMP_DO_TYPE(uint8_t)
  ALPS_DUMP_DO_TYPE(int16_t)
  ALPS_DUMP_DO_TYPE(uint16_t)
  virtual void write_simple(int32_t x) = 0;
  ALPS_DUMP_DO_TYPE(uint32_t)
# ifndef BOOST_NO_INT64_T
  ALPS_DUMP_DO_TYPE(int64_t)
  ALPS_DUMP_DO_TYPE(uint64_t)
# endif
  ALPS_DUMP_DO_TYPE(float)
  virtual void write_simple(double x) = 0;
  ALPS_DUMP_DO_TYPE(long double)
# undef ALPS_DUMP_DO_TYPE

  template <class T>
  void write_complex(const std::complex<T>& x)
  {
    write_simple(std::real(x));
    write_simple(std::imag(x));
  }

# define ALPS_DUMP_DO_TYPE(T) \
  virtual void write_array(std::size_t n, const T * p);
  ALPS_DUMP_DO_TYPE(bool)
  ALPS_DUMP_DO_TYPE(int8_t)
  ALPS_DUMP_DO_TYPE(uint8_t)
  ALPS_DUMP_DO_TYPE(int16_t)
  ALPS_DUMP_DO_TYPE(uint16_t)
  ALPS_DUMP_DO_TYPE(int32_t)
  ALPS_DUMP_DO_TYPE(uint32_t)
# ifndef BOOST_NO_INT64_T
  ALPS_DUMP_DO_TYPE(int64_t)
  ALPS_DUMP_DO_TYPE(uint64_t)
# endif
  ALPS_DUMP_DO_TYPE(float)
  ALPS_DUMP_DO_TYPE(double)
  ALPS_DUMP_DO_TYPE(long double)
# undef ALPS_DUMP_DO_TYPE

  template <class T>
  void write_array(std::size_t n, const std::complex<T>* p)
  { write_array(2 * n, reinterpret_cast<const T*>(p)); }

  virtual void write_string(std::size_t n, const char* s);

  /** register an object to prepare serializing a pointer to it.
      after writing an object it has to be registered with the dump to
      allow writing a pointer to the object. */

  void registerObjectAddress(void* p);

  /** serialize a pointer.
      after registering an object's address a pointer to it can be serialized.
      This is done by writing an integer number associated with the object
      when its address is registered. */
      
  void writePointer(void* p);
  
private: 
  uint32_t version_;
  uint32_t highestNumber_;
  std::map<void *, uint32_t> numberOfPointer_;
};


class IDump
{
public:
  IDump(uint32_t v=0);
  virtual ~IDump() {}

  uint32_t version() const { return version_;} 
  
# define ALPS_DUMP_DO_TYPE(T) \
  virtual void read_simple(T& x);
  ALPS_DUMP_DO_TYPE(bool)
  ALPS_DUMP_DO_TYPE(int8_t)
  ALPS_DUMP_DO_TYPE(uint8_t)
  ALPS_DUMP_DO_TYPE(int16_t)
  ALPS_DUMP_DO_TYPE(uint16_t)
  virtual void read_simple(int32_t& x) = 0;
  ALPS_DUMP_DO_TYPE(uint32_t)
# ifndef BOOST_NO_INT64_T
  ALPS_DUMP_DO_TYPE(int64_t)
  ALPS_DUMP_DO_TYPE(uint64_t)
# endif
  ALPS_DUMP_DO_TYPE(float)
  virtual void read_simple(double& x) = 0;
  ALPS_DUMP_DO_TYPE(long double)
# undef ALPS_DUMP_DO_TYPE

  template <class T>
  void read_complex(std::complex<T>& x)
  {
    x = std::complex<T>(get<T>(), get<T>());
  }

# define ALPS_DUMP_DO_TYPE(T) \
  virtual void read_array(std::size_t n, T * p);
  ALPS_DUMP_DO_TYPE(bool)
  ALPS_DUMP_DO_TYPE(int8_t)
  ALPS_DUMP_DO_TYPE(uint8_t)
  ALPS_DUMP_DO_TYPE(int16_t)
  ALPS_DUMP_DO_TYPE(uint16_t)
  ALPS_DUMP_DO_TYPE(int32_t)
  ALPS_DUMP_DO_TYPE(uint32_t)
# ifndef BOOST_NO_INT64_T
  ALPS_DUMP_DO_TYPE(int64_t)
  ALPS_DUMP_DO_TYPE(uint64_t)
# endif
  ALPS_DUMP_DO_TYPE(float)
  ALPS_DUMP_DO_TYPE(double)
  ALPS_DUMP_DO_TYPE(long double)
# undef ALPS_DUMP_DO_TYPE

  template <class T>
  void read_array(std::size_t n, std::complex<T>* p)
  { read_array(2 * n, reinterpret_cast<float*>(p)); }

  virtual void read_string(std::size_t n, char* s);

# define ALPS_DUMP_DO_TYPE(T) \
  operator T () { return get<T>(); }
  ALPS_DUMP_DO_TYPE(bool)
  ALPS_DUMP_DO_TYPE(int8_t)
  ALPS_DUMP_DO_TYPE(uint8_t)
  ALPS_DUMP_DO_TYPE(int16_t)
  ALPS_DUMP_DO_TYPE(uint16_t)
  ALPS_DUMP_DO_TYPE(int32_t)
  ALPS_DUMP_DO_TYPE(uint32_t)
# ifndef BOOST_NO_INT64_T
  ALPS_DUMP_DO_TYPE(int64_t)
  ALPS_DUMP_DO_TYPE(uint64_t)
# endif
  ALPS_DUMP_DO_TYPE(float)
  ALPS_DUMP_DO_TYPE(double)
  ALPS_DUMP_DO_TYPE(long double)
# undef ALPS_DUMP_DO_TYPE

  template <class T>
  operator std::complex<T> ()
  {
    std::complex<T> x;
    read_simple(x);
    return x;
  }

  template <class T>
  inline T get()
  {
    T x; read_simple(x);
    return x;
  }

  // read the next boolean value from the dump and return its value.
  bool test() { return get<bool>(); }

  /** register an object to prepare deserializing a pointer to it.
      after reading an object it has to be registered with the dump to
      allow reading a pointer to the object. */
  void registerObjectAddress(void* p);

  /** deserialize a pointer.
      After registering an object's address a pointer to it can be deserialized.
      This is done by reading an integer number associated with the object
      when its address is registered. */
  void* readPointer();
  
  /** deserialize a pointer to T.
      After registering an object's address a pointer to it can be deserialized.
      This is done by reading an integer number associated with the object
      when its address is registered. This function just calls readPointer()
      and type casts the pointer. It is a more convenient way of reading pointer.*/
  template <class T>
  T* readAPointer() {return static_cast<T*>(readPointer());}
 
private:
  uint32_t version_;
  std::vector<void*> pointerVector_;
};

} // end namespace

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
namespace alps {
#endif

#define DUMP_DO_TYPE(T) \
inline alps::ODump& operator<<(alps::ODump& dump, T x) \
{ dump.write_simple(x); return dump; } \
inline alps::IDump& operator>>(alps::IDump& dump, T& x) \
{ dump.read_simple(x); return dump; }
DUMP_DO_TYPE(bool)
DUMP_DO_TYPE(int8_t)
DUMP_DO_TYPE(uint8_t)
DUMP_DO_TYPE(int16_t)
DUMP_DO_TYPE(uint16_t)
DUMP_DO_TYPE(int32_t)
DUMP_DO_TYPE(uint32_t)
#ifndef BOOST_NO_INT64_T
DUMP_DO_TYPE(int64_t)
DUMP_DO_TYPE(uint64_t)
#endif
DUMP_DO_TYPE(float)
DUMP_DO_TYPE(double)
DUMP_DO_TYPE(long double)
#undef DUMP_DO_TYPE

template<class T>
alps::ODump& operator<<(alps::ODump& dump, const std::complex<T>& x)
{ dump.write_complex(x); return dump; }

template<class T>
alps::IDump& operator>>(alps::IDump& dump, std::complex<T>& x)
{
  dump.read_simple(x);
  return dump;
}

// conversion function
// template <class T>
// inline T read(alps::IDump& dump) { return dump.template get<T>(); }

#ifndef BOOST_NO_OPERATORS_IN_NAMESPACE
} // namespace alps
#endif

#endif // OSIRIS_DUMP_H
