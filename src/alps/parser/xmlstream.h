/***************************************************************************
* ALPS/parser library
*
* alps/parser/xmlstream.h   XML stream class
*
* $Id$
*
* Copyright (C) 2001-2003 by Synge Todo <wistaria@comp-phys.org>,
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

#ifndef ALPS_PARSER_XMLSTREAM_H
#define ALPS_PARSER_XMLSTREAM_H

#include <alps/config.h>
#include <alps/parser/attributes.h>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <stack>
#include <string>

namespace alps {

namespace detail {

struct header_t
{
header_t(const std::string& enc = "")
  : version("1.0"), encoding(enc) {}
std::string version;
std::string encoding;
};

struct start_tag_t
{
  start_tag_t(const std::string& n) : name(n) {}
  std::string name;
};

struct end_tag_t
{
  end_tag_t(const std::string& n = "") : name(n) {}
  std::string name;
};

struct attribute_t
{
template<class T>
attribute_t(const std::string& n, const T& v) : attr(n, v) {}
XMLAttribute attr;
};

struct pi_t : public start_tag_t
{
pi_t(const std::string& n) : start_tag_t(n) {}
};

} // namespace detail

class oxstream
{
public:
  oxstream(std::ostream& os = std::cout, uint32_t incl = 2);
  oxstream(const boost::filesystem::path& file, uint32_t incl = 2);
  ~oxstream();

  oxstream& operator<<(const detail::header_t& c);
  oxstream& operator<<(const detail::start_tag_t& c);
  oxstream& operator<<(const detail::end_tag_t& c);
  oxstream& operator<<(const XMLAttribute& c);
  oxstream& operator<<(const XMLAttributes& c);
  oxstream& operator<<(const detail::attribute_t& c);
  oxstream& operator<<(const detail::pi_t& c);
  oxstream& start_comment();
  oxstream& end_comment();
  oxstream& start_cdata();
  oxstream& end_cdata();
  oxstream& no_linebreak();

  oxstream& operator<<(const std::string& t) {
    return text_str(t);
  }
  oxstream& operator<<(const char t) {
    return text_str(std::string(1,t));
  }
  oxstream& operator<<(const char * t) {
    return text_str(t);
  }

  // operator<< for intrinsic types

# define ALPS_XMLSTREAM_DO_TYPE(T) \
  oxstream& operator<<(const T t) \
  { return text_str(boost::lexical_cast<std::string, T>(t)); }
  ALPS_XMLSTREAM_DO_TYPE(bool)
  ALPS_XMLSTREAM_DO_TYPE(signed char)
  ALPS_XMLSTREAM_DO_TYPE(unsigned char)
  ALPS_XMLSTREAM_DO_TYPE(short)
  ALPS_XMLSTREAM_DO_TYPE(unsigned short)
  ALPS_XMLSTREAM_DO_TYPE(int)
  ALPS_XMLSTREAM_DO_TYPE(unsigned int)
  ALPS_XMLSTREAM_DO_TYPE(long)
  ALPS_XMLSTREAM_DO_TYPE(unsigned long)
# ifdef BOOST_HAS_LONG_LONG
  ALPS_XMLSTREAM_DO_TYPE(long long)
  ALPS_XMLSTREAM_DO_TYPE(unsigned long long)
# endif
  ALPS_XMLSTREAM_DO_TYPE(float)
  ALPS_XMLSTREAM_DO_TYPE(double)
  ALPS_XMLSTREAM_DO_TYPE(long double)

# undef ALPS_XMLSTREAM_DO_TYPE

  // for manipulators
  template<class T>
  oxstream& operator<<(T (*fn)(const std::string&)) {
    return (*this << fn(std::string()));
  }
  oxstream& operator<<(oxstream& (*fn)(oxstream& oxs)) { return fn(*this); }

  std::ostream& stream() { return os_; }

protected:
  oxstream& text_str(const std::string& text);

  void output(bool close = false);
  void output_offset();

private:
  enum Context { NotSpecified, StartTag, PI, Text, Comment, Cdata };

  boost::filesystem::ofstream of_;
  std::ostream& os_;
  std::stack<std::pair<std::string, bool> > stack_;
  XMLAttributes attr_;
  Context context_;
  bool linebreak_;
  uint32_t offset_;
  uint32_t offset_incl_;
};

// manipulators

inline detail::header_t header(const std::string& enc) {
  return detail::header_t(enc);
}

inline detail::start_tag_t start_tag(const std::string& name) {
  return detail::start_tag_t(name);
}

inline detail::end_tag_t end_tag(const std::string& name = "") {
  return detail::end_tag_t(name);
}

template<class T>
inline detail::attribute_t attribute(const std::string& name, const T& value) {
  return detail::attribute_t(name, value);
}

inline detail::pi_t processing_instruction(const std::string& name) {
  return detail::pi_t(name);
}

inline detail::attribute_t xml_namespace(const std::string& name,
					 const std::string& url) {
  return detail::attribute_t("xmlns:" + name, url);
}

inline oxstream& start_comment(oxstream& oxs) { return oxs.start_comment(); }

inline oxstream& end_comment(oxstream& oxs) { return oxs.end_comment(); }

inline oxstream& start_cdata(oxstream& oxs) { return oxs.start_comment(); }

inline oxstream& end_cdata(oxstream& oxs) { return oxs.end_comment(); }

inline oxstream& no_linebreak(oxstream& oxs) { return oxs.no_linebreak(); }

// replace "<", "&", etc to entities
std::string convert(const std::string& str);

} // namespace alps

#endif // ALPS_PARSER_XMLSTREAM_H
