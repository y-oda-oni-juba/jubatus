// Jubatus: Online machine learning framework for distributed environment
// Copyright (C) 2011,2012 Preferred Infrastructure and Nippon Telegraph and Telephone Corporation.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License version 2.1 as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef JUBATUS_FRAMEWORK_DATA_CONTAINOR_HPP_
#define JUBATUS_FRAMEWORK_DATA_CONTAINOR_HPP_

#include <string>
#include <cstring>

#include <msgpack.hpp>
#include <pficommon/data/string/utility.h>

namespace jubatus {
namespace framework {

class data_header {
 public:
  data_header() {};
  explicit data_header(std::string version);

  ~data_header() {
  }

  enum { current_version = 1 };

  std::string magic_;            // 'jubatus'
  uint64_t format_version_;      // format version
  std::string jubatus_version_;  // jubatus version

  bool verify_format_version() const {
    return format_version_ == current_version;
  }

  bool verify_jubatus_version() const {
      return pfi::data::string::starts_with(
              jubatus_version_, std::string("0.4."));  // for example
  }

  bool is_valid() const {
    return magic_ == "jubatus";
  }

};

class data_containor {
 public:
  data_containor()
      : version_(0) {
  }

  virtual ~data_containor() {
  }

  uint64_t version_;

  bool verify_version(uint64_t version) const {
      return version_ == version;
  }

  MSGPACK_DEFINE(version_);
};

class system_data_containor : public data_containor {
 public:
  time_t timestamp_;
  std::string type_;    // task(engine) type name
  bool is_standalone_;  // running mode
  std::string id_;      // unique name
  std::string config_;  // config values

  enum { current_version = 1 };

  MSGPACK_DEFINE(version_, timestamp_, type_, is_standalone_, id_, config_);
};

class user_data_containor : public data_containor {
 public:
  virtual ~user_data_containor() {
  }

  enum { current_version = 1 };

  MSGPACK_DEFINE(version_);
};


}  // namespace framework
}  // namespace jubatus

#endif  // JUBATUS_FRAMEWORK_DATA_CONTAINOR_HPP_
