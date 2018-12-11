// Copyright 2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef GUID_HPP_
#define GUID_HPP_

#include <dds_dcps.h>

#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>


inline void DDS_BuiltinTopicKey_to_GUID(
  DDS::InstanceHandle_t * guid,
  DDS::BuiltinTopicKey_t buitinTopicKey)
{
  v_builtinTopicKey gid;

  // copyInTopicKey(builtinTopicKey, &gid);
  gid.systemId = buitinTopicKey[0];
  gid.localId = buitinTopicKey[1];
  gid.serial = buitinTopicKey[2];

  *guid = u_instanceHandleFromGID(gid);
}

#endif  // GUID_HPP_
