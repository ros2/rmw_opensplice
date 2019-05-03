// Copyright 2014-2015 Open Source Robotics Foundation, Inc.
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

#include "qos.hpp"

#include <limits>

#include "rmw/error_handling.h"
#include "rosidl_typesupport_opensplice_cpp/impl/error_checking.hpp"

using rosidl_typesupport_opensplice_cpp::impl::check_get_default_datareader_qos;
using rosidl_typesupport_opensplice_cpp::impl::check_get_default_datawriter_qos;

template<typename DDSEntityQos>
bool set_entity_qos_from_profile_generic(
  const rmw_qos_profile_t & qos_profile,
  DDSEntityQos & entity_qos)
{
  switch (qos_profile.history) {
    case RMW_QOS_POLICY_HISTORY_KEEP_LAST:
      entity_qos.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
      break;
    case RMW_QOS_POLICY_HISTORY_KEEP_ALL:
      entity_qos.history.kind = DDS::KEEP_ALL_HISTORY_QOS;
      break;
    case RMW_QOS_POLICY_HISTORY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS history policy");
      return false;
  }

  switch (qos_profile.reliability) {
    case RMW_QOS_POLICY_RELIABILITY_BEST_EFFORT:
      entity_qos.reliability.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
      break;
    case RMW_QOS_POLICY_RELIABILITY_RELIABLE:
      entity_qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
      break;
    case RMW_QOS_POLICY_RELIABILITY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS reliability policy");
      return false;
  }

  switch (qos_profile.durability) {
    case RMW_QOS_POLICY_DURABILITY_TRANSIENT_LOCAL:
      entity_qos.durability.kind = DDS::TRANSIENT_LOCAL_DURABILITY_QOS;
      break;
    case RMW_QOS_POLICY_DURABILITY_VOLATILE:
      entity_qos.durability.kind = DDS::VOLATILE_DURABILITY_QOS;
      break;
    case RMW_QOS_POLICY_DURABILITY_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS durability policy");
      return false;
  }

  if (qos_profile.depth != RMW_QOS_POLICY_DEPTH_SYSTEM_DEFAULT) {
    entity_qos.history.depth =
      static_cast<DDS::Long>(qos_profile.depth);
  }

  // DDS_DeadlineQosPolicy has default value of DDS_DURATION_INFINITE, don't touch it for 0
  if (qos_profile.deadline.sec != 0 || qos_profile.deadline.nsec != 0) {
    entity_qos.deadline.period.sec = qos_profile.deadline.sec;
    entity_qos.deadline.period.nanosec = qos_profile.deadline.nsec;
  }

  switch (qos_profile.liveliness) {
    case RMW_QOS_POLICY_LIVELINESS_AUTOMATIC:
      entity_qos.liveliness.kind = DDS::AUTOMATIC_LIVELINESS_QOS;
      break;
    case RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_NODE:
      entity_qos.liveliness.kind = DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
      break;
    case RMW_QOS_POLICY_LIVELINESS_MANUAL_BY_TOPIC:
      entity_qos.liveliness.kind = DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS;
      break;
    case RMW_QOS_POLICY_LIVELINESS_SYSTEM_DEFAULT:
      break;
    default:
      RMW_SET_ERROR_MSG("Unknown QoS liveliness policy");
      return false;
  }
  if (qos_profile.liveliness_lease_duration.sec != 0 ||
    qos_profile.liveliness_lease_duration.nsec != 0)
  {
    entity_qos.liveliness.lease_duration.sec = qos_profile.liveliness_lease_duration.sec;
    entity_qos.liveliness.lease_duration.nanosec = qos_profile.liveliness_lease_duration.nsec;
  }

  // ensure the history depth is at least the requested queue size
  assert(entity_qos.history.depth >= 0);
  if (
    entity_qos.history.kind == DDS::KEEP_LAST_HISTORY_QOS &&
    static_cast<size_t>(entity_qos.history.depth) < qos_profile.depth)
  {
    if (qos_profile.depth > (std::numeric_limits<DDS::Long>::max)()) {
      RMW_SET_ERROR_MSG(
        "failed to set history depth since the requested queue size exceeds the DDS type");
      return false;
    }
    entity_qos.history.depth = static_cast<DDS::Long>(qos_profile.depth);
  }

  return true;
}

bool
set_entity_qos_from_profile(
  const rmw_qos_profile_t & qos_profile,
  DDS::DataReaderQos & entity_qos)
{
  // Set any QoS settings that are specific to DataReader, then call the shared version
  return set_entity_qos_from_profile_generic(qos_profile, entity_qos);
}

bool
set_entity_qos_from_profile(
  const rmw_qos_profile_t & qos_profile,
  DDS::DataWriterQos & entity_qos)
{
  // Set any QoS settings that are specific to DataWriter, then call the shared version
  if (qos_profile.lifespan.sec != 0 || qos_profile.lifespan.nsec != 0) {
    entity_qos.lifespan.duration.sec = qos_profile.lifespan.sec;
    entity_qos.lifespan.duration.nanosec = qos_profile.lifespan.nsec;
  }
  return set_entity_qos_from_profile_generic(qos_profile, entity_qos);
}

bool get_datareader_qos(
  DDS::Subscriber * subscriber,
  const rmw_qos_profile_t & qos_profile,
  DDS::DataReaderQos & datareader_qos)
{
  if (subscriber == nullptr) {
    datareader_qos = DATAREADER_QOS_DEFAULT;
  } else {
    DDS::ReturnCode_t status;
    status = subscriber->get_default_datareader_qos(datareader_qos);
    if (nullptr != (check_get_default_datareader_qos(status))) {
      RMW_SET_ERROR_MSG(check_get_default_datareader_qos(status));
      return false;
    }
  }
  return set_entity_qos_from_profile(qos_profile, datareader_qos);
}

bool get_datawriter_qos(
  DDS::Publisher * publisher,
  const rmw_qos_profile_t & qos_profile,
  DDS::DataWriterQos & datawriter_qos)
{
  if (publisher == nullptr) {
    datawriter_qos = DATAWRITER_QOS_DEFAULT;
  } else {
    DDS::ReturnCode_t status;
    status = publisher->get_default_datawriter_qos(datawriter_qos);
    if (nullptr != check_get_default_datawriter_qos(status)) {
      RMW_SET_ERROR_MSG(check_get_default_datawriter_qos(status));
      return false;
    }
  }

  return set_entity_qos_from_profile(qos_profile, datawriter_qos);
}
