// Jubatus: Online machine learning framework for distributed environment
// Copyright (C) 2011 Preferred Infrastructure and Nippon Telegraph and Telephone Corporation.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#include <gtest/gtest.h>
#include <exception>
#include <pficommon/lang/scoped_ptr.h>

#include "storage_factory.hpp"
#include "local_storage.hpp"
#include "local_storage_mixture.hpp"

using namespace pfi::lang;

namespace jubatus {
namespace storage {

TEST(storage_factory, trivial) {
  {
    scoped_ptr<storage_base> s(storage_factory::create_storage("local"));
    EXPECT_EQ(typeid(local_storage), typeid(*s));
  }
  {
    scoped_ptr<storage_base> s(storage_factory::create_storage("local_mixture"));
    EXPECT_EQ(typeid(local_storage_mixture), typeid(*s));
  }
  {
    EXPECT_THROW(storage_factory::create_storage("unknown"),
                 std::exception);
  }

}

}
}
