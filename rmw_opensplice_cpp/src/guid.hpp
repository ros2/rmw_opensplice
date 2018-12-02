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

#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "dds_dcps.h"

// TODO(ross-desmond): This should all be in opensplice
typedef char octet;

// http://www.eprosima.com/docs/fast-rtps/1.6.0/html/_guid_8h_source.html
struct GuidPrefix_t
{
  static const unsigned int size = 12;
  octet value[size];

  GuidPrefix_t()
  {
    memset(value, 0, size);
  }

  GuidPrefix_t(octet guid[size])
  {
    memcpy(value, guid, size);
  }

  GuidPrefix_t(const GuidPrefix_t & g)
  {
    memcpy(value, g.value, size);
  }

  GuidPrefix_t(GuidPrefix_t && g)
  {
    memmove(value, g.value, size);
  }

  GuidPrefix_t & operator=(const GuidPrefix_t & guidpre)
  {
    memcpy(value, guidpre.value, size);
    return *this;
  }

  GuidPrefix_t & operator=(GuidPrefix_t && guidpre)
  {
    memmove(value, guidpre.value, size);
    return *this;
  }

  static GuidPrefix_t unknown()
  {
    return GuidPrefix_t();
  }

#ifndef DOXYGEN_SHOULD_SKIP_THIS_PUBLIC

  bool operator==(const GuidPrefix_t & prefix) const
  {
    return memcmp(value, prefix.value, size) == 0;
  }

  bool operator!=(const GuidPrefix_t & prefix) const
  {
    return memcmp(value, prefix.value, size) != 0;
  }

#endif
};

inline bool operator<(const GuidPrefix_t & g1, const GuidPrefix_t & g2)
{
  for (uint8_t i = 0; i < 12; ++i) {
    if (g1.value[i] < g2.value[i]) {
      return true;
    } else if (g1.value[i] > g2.value[i]) {
      return false;
    }
  }
  return false;
}

inline std::ostream & operator<<(std::ostream & output, const GuidPrefix_t & guiP)
{
  output << std::hex;
  for (uint8_t i = 0; i < 11; ++i) {
    output << (int) guiP.value[i] << ".";
  }
  output << (int) guiP.value[11];
  return output << std::dec;
}

// TODO(ross-desmond): big/little endian
inline void DDS_InstanceHandle_to_GUID(
  struct GuidPrefix_t * guid,
  DDS::InstanceHandle_t instanceHandle)
{
  memcpy(guid->value, (char *) &instanceHandle, guid->size);
}

inline void DDS_BuiltinTopicKey_to_GUID(
  struct GuidPrefix_t * guid,
  DDS::BuiltinTopicKey_t buitinTopicKey)
{
#if BIG_ENDIAN
  /* Big Endian */
  memcpy(guid->value, (octet *) buitinTopicKey, guid->size);
#else
  /* Little Endian */
  int i;
  octet * guidElement;
  octet * topicKeyBuffer = (octet *)buitinTopicKey;
  octet * keyBufferElement;
  for (i = 0; i < 3; ++i) {
    octet * guidElement = &guid->value[i * 3];
    octet * keyBufferElement = (DDS_Octet *)(&buitinTopicKey[i * 3]);
    guidElement[0] = keyBufferElement[2];
    guidElement[1] = keyBufferElement[1];
    guidElement[2] = keyBufferElement[0];
  }

#endif
}

#endif  // GUID_HPP_
