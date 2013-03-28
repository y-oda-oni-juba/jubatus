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

#ifndef JUBATUS_FRAMEWORK_EXTERNALIZABLE_HPP_
#define JUBATUS_FRAMEWORK_EXTERNALIZABLE_HPP_

#include <string>

#include <msgpack.hpp>

namespace jubatus {
namespace framework {

class data_containor {
 public:
  data_containor()
      : version_(1) {
  }

  virtual ~data_containor() {
  }

  uint64_t version_;
  virtual uint64_t version() { return -1; }

  MSGPACK_DEFINE(version_);
};

class system_data_containor : public data_containor {
 public:
  time_t timestamp_;
  std::string type_;    // task(engine) type name
  bool is_standalone_;   // running mode
  std::string id_;      // unique name
  std::string config_;  // config values

  uint64_t version() {
      return 1;
  }

  MSGPACK_DEFINE(version_, timestamp_, type_, is_standalone_, id_, config_);
};


}  // namespace framework
}  // namespace jubatus

#endif  // JUBATUS_FRAMEWORK_EXTERNALIZABLE_HPP_
