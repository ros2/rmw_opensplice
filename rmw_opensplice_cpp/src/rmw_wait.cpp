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

#ifdef __clang__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wmismatched-tags"
#endif
#if defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable: 4099)
#endif
#include <ccpp_dds_dcps.h>
#if defined(_MSC_VER)
# pragma warning(pop)
#endif
#ifdef __clang__
# pragma GCC diagnostic pop
#endif
#include <dds_dcps.h>

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw/types.h"

#include "identifier.hpp"
#include "types.hpp"

// The extern "C" here enforces that overloading is not used.
extern "C"
{
rmw_ret_t check_attach_condition_error(DDS::ReturnCode_t retcode)
{
  if (retcode == DDS::RETCODE_OK) {
    return RMW_RET_OK;
  }
  if (retcode == DDS::RETCODE_OUT_OF_RESOURCES) {
    RMW_SET_ERROR_MSG("failed to attach condition to wait set: out of resources");
  } else if (retcode == DDS::RETCODE_BAD_PARAMETER) {
    RMW_SET_ERROR_MSG("failed to attach condition to wait set: condition pointer was invalid");
  } else {
    RMW_SET_ERROR_MSG("failed to attach condition to wait set");
  }
  return RMW_RET_ERROR;
}

rmw_ret_t
rmw_wait(
  rmw_subscriptions_t * subscriptions,
  rmw_guard_conditions_t * guard_conditions,
  rmw_services_t * services,
  rmw_clients_t * clients,
  rmw_wait_set_t * wait_set,
  const rmw_time_t * wait_timeout)
{
  // To ensure that we properly clean up the wait set, we declare an
  // object whose destructor will detach what we attached (this was previously
  // being done inside the destructor of the wait set.
  struct atexit_t
  {
    ~atexit_t()
    {
      // Manually detach conditions and clear sequences, to ensure a clean wait set for next time.
      if (!wait_set) {
        RMW_SET_ERROR_MSG("wait set handle is null");
        return;
      }
      RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
        wait set handle,
        wait_set->implementation_identifier, opensplice_cpp_identifier,
        return )
      OpenSpliceWaitSetInfo * wait_set_info = static_cast<OpenSpliceWaitSetInfo *>(wait_set->data);
      if (!wait_set_info) {
        RMW_SET_ERROR_MSG("WaitSet implementation struct is null");
        return;
      }

      DDS::WaitSet * dds_wait_set = static_cast<DDS::WaitSet *>(wait_set_info->wait_set);
      if (!dds_wait_set) {
        RMW_SET_ERROR_MSG("DDS wait set handle is null");
        return;
      }

      DDS::ConditionSeq * attached_conditions =
        static_cast<DDS::ConditionSeq *>(wait_set_info->attached_conditions);
      if (!attached_conditions) {
        RMW_SET_ERROR_MSG("DDS condition sequence handle is null");
        return;
      }

      DDS::ReturnCode_t retcode;
      retcode = dds_wait_set->get_conditions(*attached_conditions);
      if (retcode != DDS::RETCODE_OK) {
        RMW_SET_ERROR_MSG("Failed to get attached conditions for wait set");
        return;
      }

      for (uint32_t i = 0; i < attached_conditions->length(); ++i) {
        retcode = dds_wait_set->detach_condition((*attached_conditions)[i]);
        if (retcode != DDS::RETCODE_OK) {
          RMW_SET_ERROR_MSG("Failed to get detach condition from wait set");
        }
      }

      DDS::ConditionSeq * active_conditions =
        static_cast<DDS::ConditionSeq *>(wait_set_info->active_conditions);
      if (!active_conditions) {
        RMW_SET_ERROR_MSG("DDS condition sequence handle is null");
        return;
      }

      // Disassociate conditions left in active_conditions so that when the
      // wait set (and therefore the active_conditions sequence) are destroyed
      // they do not try to free the entities to which they point.
      // These entities are already being deleted (are owned) by other entities
      // like rmw_guard_conditions_t and rmw_node_t.
      // Without this step, sporadic bad memory accesses could occur when the
      // items added to the wait set were destroyed before the wait set.
      // The items in active_conditions are not being leaked though because
      // other entities own them and are responsible for removing them.
      for (uint32_t i = 0; i < active_conditions->length(); ++i) {
        active_conditions->get_buffer()[i] = nullptr;
      }
    }
    rmw_wait_set_t * wait_set = NULL;
  } atexit;

  atexit.wait_set = wait_set;

  if (!wait_set) {
    RMW_SET_ERROR_MSG("wait set handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    wait set,
    wait_set->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR);

  OpenSpliceWaitSetInfo * wait_set_info = static_cast<OpenSpliceWaitSetInfo *>(wait_set->data);
  if (!wait_set_info) {
    RMW_SET_ERROR_MSG("WaitSet implementation struct is null");
    return RMW_RET_ERROR;
  }

  DDS::WaitSet * dds_wait_set = static_cast<DDS::WaitSet *>(wait_set_info->wait_set);
  if (!dds_wait_set) {
    RMW_SET_ERROR_MSG("DDS wait set handle is null");
    return RMW_RET_ERROR;
  }

  DDS::ConditionSeq * active_conditions =
    static_cast<DDS::ConditionSeq *>(wait_set_info->active_conditions);
  if (!active_conditions) {
    RMW_SET_ERROR_MSG("DDS condition sequence handle is null");
    return RMW_RET_ERROR;
  }

  DDS::ConditionSeq * attached_conditions =
    static_cast<DDS::ConditionSeq *>(wait_set_info->attached_conditions);
  if (!attached_conditions) {
    RMW_SET_ERROR_MSG("DDS condition sequence handle is null");
    return RMW_RET_ERROR;
  }

  // add a condition for each subscriber
  if (subscriptions) {
    for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
      OpenSpliceStaticSubscriberInfo * subscriber_info =
        static_cast<OpenSpliceStaticSubscriberInfo *>(subscriptions->subscribers[i]);
      if (!subscriber_info) {
        RMW_SET_ERROR_MSG("subscriber info handle is null");
        return RMW_RET_ERROR;
      }
      DDS::ReadCondition * read_condition = subscriber_info->read_condition;
      if (!read_condition) {
        RMW_SET_ERROR_MSG("read condition handle is null");
        return RMW_RET_ERROR;
      }
      rmw_ret_t status = check_attach_condition_error(
        dds_wait_set->attach_condition(read_condition));
      if (status != RMW_RET_OK) {
        return status;
      }
    }
  }

  // add a condition for each guard condition
  if (guard_conditions) {
    for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
      DDS::GuardCondition * guard_condition =
        static_cast<DDS::GuardCondition *>(guard_conditions->guard_conditions[i]);
      if (!guard_condition) {
        RMW_SET_ERROR_MSG("guard condition handle is null");
        return RMW_RET_ERROR;
      }
      rmw_ret_t status = check_attach_condition_error(
        dds_wait_set->attach_condition(guard_condition));
      if (status != RMW_RET_OK) {
        return status;
      }
    }
  }

  // add a condition for each service
  if (services) {
    for (size_t i = 0; i < services->service_count; ++i) {
      OpenSpliceStaticServiceInfo * service_info =
        static_cast<OpenSpliceStaticServiceInfo *>(services->services[i]);
      if (!service_info) {
        RMW_SET_ERROR_MSG("service info handle is null");
        return RMW_RET_ERROR;
      }
      DDS::ReadCondition * read_condition = service_info->read_condition_;
      if (!read_condition) {
        RMW_SET_ERROR_MSG("read condition handle is null");
        return RMW_RET_ERROR;
      }
      rmw_ret_t status = check_attach_condition_error(
        dds_wait_set->attach_condition(read_condition));
      if (status != RMW_RET_OK) {
        return status;
      }
    }
  }

  // add a condition for each client
  if (clients) {
    for (size_t i = 0; i < clients->client_count; ++i) {
      OpenSpliceStaticClientInfo * client_info =
        static_cast<OpenSpliceStaticClientInfo *>(clients->clients[i]);
      if (!client_info) {
        RMW_SET_ERROR_MSG("client info handle is null");
        return RMW_RET_ERROR;
      }
      DDS::ReadCondition * read_condition = client_info->read_condition_;
      if (!read_condition) {
        RMW_SET_ERROR_MSG("read condition handle is null");
        return RMW_RET_ERROR;
      }
      rmw_ret_t status = check_attach_condition_error(
        dds_wait_set->attach_condition(read_condition));
      if (status != RMW_RET_OK) {
        return status;
      }
    }
  }

  // invoke wait until one of the conditions triggers

  DDS::Duration_t timeout;
  if (!wait_timeout) {
    timeout = DDS::DURATION_INFINITE;
  } else {
    timeout.sec = static_cast<DDS::Long>(wait_timeout->sec);
    timeout.nanosec = static_cast<DDS::Long>(wait_timeout->nsec);
  }
  DDS::ReturnCode_t status = dds_wait_set->wait(*active_conditions, timeout);

  if (status != DDS::RETCODE_OK && status != DDS::RETCODE_TIMEOUT) {
    RMW_SET_ERROR_MSG("failed to wait on wait set");
    return RMW_RET_ERROR;
  }

  // set subscriber handles to zero for all not triggered status conditions
  if (subscriptions) {
    for (size_t i = 0; i < subscriptions->subscriber_count; ++i) {
      OpenSpliceStaticSubscriberInfo * subscriber_info =
        static_cast<OpenSpliceStaticSubscriberInfo *>(subscriptions->subscribers[i]);
      if (!subscriber_info) {
        RMW_SET_ERROR_MSG("subscriber info handle is null");
        return RMW_RET_ERROR;
      }
      DDS::ReadCondition * read_condition = subscriber_info->read_condition;
      if (!read_condition) {
        RMW_SET_ERROR_MSG("read condition handle is null");
        return RMW_RET_ERROR;
      }
      if (!read_condition->get_trigger_value()) {
        // if the status condition was not triggered
        // reset the subscriber handle
        subscriptions->subscribers[i] = 0;
      }
      DDS::ReturnCode_t detach_status = dds_wait_set->detach_condition(read_condition);
      if (detach_status != DDS::RETCODE_OK && detach_status != DDS::RETCODE_PRECONDITION_NOT_MET) {
        RMW_SET_ERROR_MSG("failed to detach guard condition");
        return RMW_RET_ERROR;
      }
    }
  }

  if (guard_conditions) {
    for (size_t i = 0; i < guard_conditions->guard_condition_count; ++i) {
      DDS::GuardCondition * guard_condition =
        static_cast<DDS::GuardCondition *>(guard_conditions->guard_conditions[i]);
      if (!guard_condition) {
        RMW_SET_ERROR_MSG("guard condition handle is null");
        return RMW_RET_ERROR;
      }

      if (!guard_condition->get_trigger_value()) {
        guard_conditions->guard_conditions[i] = 0;
      } else {
        // reset the trigger value for triggered guard conditions
        if (guard_condition->set_trigger_value(false) != DDS::RETCODE_OK) {
          RMW_SET_ERROR_MSG("failed to set trigger value to false");
          return RMW_RET_ERROR;
        }
      }
      if (dds_wait_set->detach_condition(guard_condition) != DDS::RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to detach guard condition");
        return RMW_RET_ERROR;
      }
    }
  }

  // set service handles to zero for all not triggered conditions
  if (services) {
    for (size_t i = 0; i < services->service_count; ++i) {
      OpenSpliceStaticServiceInfo * service_info =
        static_cast<OpenSpliceStaticServiceInfo *>(services->services[i]);
      if (!service_info) {
        RMW_SET_ERROR_MSG("service info handle is null");
        return RMW_RET_ERROR;
      }
      DDS::ReadCondition * read_condition = service_info->read_condition_;
      if (!read_condition) {
        RMW_SET_ERROR_MSG("read condition handle is null");
        return RMW_RET_ERROR;
      }

      // search for service condition in active set
      uint32_t j = 0;
      for (; j < active_conditions->length(); ++j) {
        if ((*active_conditions)[j] == read_condition) {
          break;
        }
      }
      // if service condition is not found in the active set
      // reset the service handle
      if (!(j < active_conditions->length())) {
        services->services[i] = 0;
      }
      if (dds_wait_set->detach_condition(read_condition) != DDS::RETCODE_OK) {
        RMW_SET_ERROR_MSG("failed to detach guard condition");
        return RMW_RET_ERROR;
      }
    }
  }

  // set client handles to zero for all not triggered conditions
  if (clients) {
    for (size_t i = 0; i < clients->client_count; ++i) {
      OpenSpliceStaticClientInfo * client_info =
        static_cast<OpenSpliceStaticClientInfo *>(clients->clients[i]);
      if (!client_info) {
        RMW_SET_ERROR_MSG("client info handle is null");
        return RMW_RET_ERROR;
      }
      DDS::ReadCondition * read_condition = client_info->read_condition_;
      if (!read_condition) {
        RMW_SET_ERROR_MSG("read condition handle is null");
        return RMW_RET_ERROR;
      }

      // search for service condition in active set
      uint32_t j = 0;
      for (; j < active_conditions->length(); ++j) {
        if ((*active_conditions)[j] == read_condition) {
          break;
        }
      }
      // if client condition is not found in the active set
      // reset the client handle
      if (!(j < active_conditions->length())) {
        clients->clients[i] = 0;
      }
      DDS::ReturnCode_t detach_status = dds_wait_set->detach_condition(read_condition);
      if (detach_status != DDS::RETCODE_OK && detach_status != DDS::RETCODE_PRECONDITION_NOT_MET) {
        RMW_SET_ERROR_MSG("failed to detach guard condition");
        return RMW_RET_ERROR;
      }
    }
  }

  if (status == DDS::RETCODE_TIMEOUT) {
    return RMW_RET_TIMEOUT;
  }
  return RMW_RET_OK;
}
}  // extern "C"
