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

    // data_header
    data_header header(JUBATUS_VERSION);
    ofs << header.magic_ << std::endl;
    ofs << header.format_version_ << std::endl;
    ofs << header.jubatus_version_ << std::endl;

    // system_data
    system_data_containor system_data;
    system_data.version_ = system_data.current_version;
    system_data.timestamp_ = time(NULL);
    system_data.type_ = argv_.type;
    system_data.is_standalone_ = argv_.is_standalone();
    system_data.id_ = id;
    system_data.config_ = config_;

    msgpack::packer<std::ofstream> packer(&ofs);
    packer << system_data;

    // user_data
    for (size_t i = 0; i < user_data_list_.size(); ++i) {
      packer << *user_data_list_[i];
    }

    ofs.close();

    LOG(INFO) << "saved to " << path;
    print_data(header, system_data);
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

    // data header
    data_header header;

    getline(ifs, header.magic_);
    if (!header.is_valid()) {
      throw JUBATUS_EXCEPTION(
          jubatus::exception::runtime_error(
              "invalid file format")
          << jubatus::exception::error_file_name(path));
    }

    std::string buf;
    getline(ifs, buf);
    header.format_version_ = atoi(buf.c_str());
    if (!header.verify_format_version()) {
      throw JUBATUS_EXCEPTION(
          jubatus::exception::runtime_error(
              "not compatible file format version")
          << jubatus::exception::error_file_name(path));
    }

    getline(ifs, header.jubatus_version_);
    if (!header.verify_jubatus_version()) {
      throw JUBATUS_EXCEPTION(
          jubatus::exception::runtime_error(
              "not compatible jubatus version")
          << jubatus::exception::error_file_name(path));
    }

    msgpack::unpacker unpacker;
    msgpack::unpacked msg;
    while (true) {
        unpacker.reserve_buffer(1024);
        ifs.read(unpacker.buffer(), unpacker.buffer_capacity());
        unpacker.buffer_consumed(ifs.gcount());
        if (ifs.fail()) {
            break;
        }
    }

    // system data
    data_containor data;
    system_data_containor system_data;
    unpacker.next(&msg);
    msg.get().convert(&data);
    if (!data.verify_version(system_data.current_version)) {
      throw JUBATUS_EXCEPTION(
          jubatus::exception::runtime_error(
              "not compatible file format")
          << jubatus::exception::error_file_name(path));
    }

    msg.get().convert(&system_data);
    if (system_data.type_ != argv_.type) {
      throw JUBATUS_EXCEPTION(
          jubatus::exception::runtime_error(
              "invalid engine type")
          << jubatus::exception::error_file_name(path));
    }

    if (system_data.config_ != config_) {
      LOG(WARNING) << "loaded config is different from stored";
    }

    // user_data
    for (size_t i = 0; i < user_data_list_.size(); ++i) {
        unpacker.next(&msg);
        msg.get().convert(&data);
        if (!data.verify_version(user_data_list_[i]->current_version)) {
          throw JUBATUS_EXCEPTION(
              jubatus::exception::runtime_error(
                  "not compatible file format")
              << jubatus::exception::error_file_name(path));
        }
        msg.get().convert(user_data_list_[i]);
    }

    LOG(INFO) << "loaded from " << path;
    print_data(header, system_data);

    ifs.close();
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

void server_base::register_user_data(user_data_containor* d) {
  user_data_list_.push_back(d);
}

void server_base::print_data(const data_header &h, const system_data_containor &s) const {
    LOG(INFO) << "    magic number    : " << h.magic_;
    LOG(INFO) << "    format version  : " << h.format_version_;
    LOG(INFO) << "    jubatus version : " << h.jubatus_version_;
    LOG(INFO) << "    system_data";
    LOG(INFO) << "      version       : " << s.version_;
    LOG(INFO) << "      timestamp     : " << s.timestamp_;
    LOG(INFO) << "      type          : " << s.type_;
    LOG(INFO) << "      is_standalone : "
        << std::boolalpha << s.is_standalone_;
    LOG(INFO) << "      id            : " << s.id_;
    LOG(INFO) << "      config        : " << s.config_;
}

data_header::data_header(const std::string version) {
  magic_ = "jubatus";
  format_version_ = current_version;
  jubatus_version_ = version;
}

}  // namespace framework
}  // namespace jubatus
