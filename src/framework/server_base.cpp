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

#include "server_base.hpp"

#include <time.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <glog/logging.h>
#include <msgpack.hpp>

#include "mixable.hpp"
#include "mixer/mixer.hpp"
#include "../common/exception.hpp"

namespace jubatus {
namespace framework {

static const std::string VERSION(JUBATUS_VERSION);
static const uint64_t FORMAT_VERSION = 1;

namespace {

std::string build_local_path(
    const server_argv& a,
    const std::string& type,
    const std::string& id) {
  std::ostringstream path;
  path << a.datadir << '/' << a.eth << '_' << a.port << '_' << type << '_' << id
      << ".js";
  return path.str();
}

}  // namespace

server_base::server_base(const server_argv& a)
    : argv_(a),
      update_count_(0) {
}

bool server_base::save(const std::string& id) {
  const std::string path = build_local_path(argv_, "jubatus", id);
  std::ofstream ofs(path.c_str(), std::ios::trunc | std::ios::binary);
  if (!ofs) {
    throw
        JUBATUS_EXCEPTION(
            jubatus::exception::runtime_error("cannot open output file")
            << jubatus::exception::error_file_name(path)
            << jubatus::exception::error_errno(errno));
  }
  try {
    LOG(INFO) << "starting save to " << path;
    msgpack::packer<std::ofstream> packer(&ofs);
    packer << FORMAT_VERSION;
    packer << VERSION;
    system_data_.timestamp_ = time(NULL);
    system_data_.type_ = argv_.type;
    system_data_.is_standalone_ = argv_.is_standalone();
    system_data_.id_ = id;
    // TBD: system_data_.config = config;
    packer << system_data_;

    // TBD: user data containor

    ofs.close();
    LOG(INFO) << "saved to " << path;
    LOG(INFO) << "    timestamp     : " << system_data_.timestamp_;
    LOG(INFO) << "    type          : " << system_data_.type_;
    LOG(INFO) << "    is_standalone : "
        << std::boolalpha << system_data_.is_standalone_;
    LOG(INFO) << "    id            : " << system_data_.id_;
    LOG(INFO) << "    config        : " << system_data_.config_;
  } catch (const std::runtime_error& e) {
    LOG(ERROR) << "failed to save: " << path;
    LOG(ERROR) << e.what();
    throw;
  }
  return true;
}

bool server_base::load(const std::string& id) {
  const std::string path = build_local_path(argv_, "jubatus", id);
  std::ifstream ifs(path.c_str(), std::ios::binary);
  if (!ifs) {
    throw JUBATUS_EXCEPTION(
        jubatus::exception::runtime_error("cannot open input file")
        << jubatus::exception::error_file_name(path)
        << jubatus::exception::error_errno(errno));
  }

  try {
    LOG(INFO) << "starting load from " << path;
    msgpack::unpacker unpacker;
    while (true) {
        unpacker.reserve_buffer(1024);
        ifs.read(unpacker.buffer(), unpacker.buffer_capacity());
        unpacker.buffer_consumed(ifs.gcount());
        if (ifs.fail()) {
            break;
        }
    }
    ifs.close();

    msgpack::unpacked msg;

    // format version
    uint64_t format_version;
    unpacker.next(&msg);
    msg.get().convert(&format_version);
    if (format_version != FORMAT_VERSION) {
      throw JUBATUS_EXCEPTION(
          jubatus::exception::runtime_error(
              "not compatible format: " + format_version)
          << jubatus::exception::error_file_name(path));
    }

    // jubatus version
    std::string version;
    unpacker.next(&msg);
    msg.get().convert(&version);
    if (version != VERSION) {
      throw JUBATUS_EXCEPTION(
          jubatus::exception::runtime_error(
              "not compatible format: " + version)
          << jubatus::exception::error_file_name(path));
    }

    // system data containor
    data_containor data;
    system_data_containor system_data;
    unpacker.next(&msg);
    msg.get().convert(&data);
    if (data.version_ != system_data.version()) {
      throw JUBATUS_EXCEPTION(
          jubatus::exception::runtime_error(
              "not compatible format: " + data.version_)
          << jubatus::exception::error_file_name(path));
    }
    msg.get().convert(&system_data);

    // TBD: user data containor

    LOG(INFO) << "loaded from " << path;
    LOG(INFO) << "    timestamp     : " << system_data.timestamp_;
    LOG(INFO) << "    type          : " << system_data.type_;
    LOG(INFO) << "    is_standalone : "
        << std::boolalpha << system_data.is_standalone_;
    LOG(INFO) << "    id            : " << system_data.id_;
    LOG(INFO) << "    config        : " << system_data.config_;
  } catch (const std::runtime_error& e) {
    ifs.close();
    LOG(ERROR) << "failed to load: " << path;
    LOG(ERROR) << e.what();
    throw;
  }
  return true;
}

void server_base::event_model_updated() {
  ++update_count_;
  if (mixer::mixer* m = get_mixer()) {
    m->updated();
  }
}

}  // namespace framework
}  // namespace jubatus
