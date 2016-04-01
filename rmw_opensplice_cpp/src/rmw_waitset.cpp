// Copyright 2015 Open Source Robotics Foundation, Inc.
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

#include <ccpp_dds_dcps.h>
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
rmw_waitset_t *
rmw_create_waitset(size_t max_conditions)
{
  rmw_waitset_t * waitset = rmw_waitset_allocate();
  OpenSpliceWaitSetInfo * waitset_info = nullptr;

  // From here onward, error results in unrolling in the goto fail block.
  if (!waitset) {
    RMW_SET_ERROR_MSG("failed to allocate waitset");
    goto fail;
  }
  waitset->implementation_identifier = opensplice_cpp_identifier;
  waitset->data = rmw_allocate(sizeof(OpenSpliceWaitSetInfo));

  waitset_info = static_cast<OpenSpliceWaitSetInfo *>(waitset->data);

  if (!waitset_info) {
    RMW_SET_ERROR_MSG("failed to allocate waitset implementation struct");
    goto fail;
  }

  waitset_info->waitset = static_cast<DDS::WaitSet *>(rmw_allocate(sizeof(DDS::WaitSet)));
  if (!waitset_info->waitset) {
    RMW_SET_ERROR_MSG("failed to allocate waitset");
    goto fail;
  }

  RMW_TRY_PLACEMENT_NEW(
    waitset_info->waitset, waitset_info->waitset, goto fail, DDS::WaitSet)

  // Now allocate storage for the ConditionSeq objects
  waitset_info->active_conditions =
    static_cast<DDS::ConditionSeq *>(rmw_allocate(sizeof(DDS::ConditionSeq)));
  if (!waitset_info->active_conditions) {
    RMW_SET_ERROR_MSG("failed to allocate active_conditions sequence");
    goto fail;
  }

  waitset_info->attached_conditions =
    static_cast<DDS::ConditionSeq *>(rmw_allocate(sizeof(DDS::ConditionSeq)));
  if (!waitset_info->attached_conditions) {
    RMW_SET_ERROR_MSG("failed to allocate attached_conditions sequence");
    goto fail;
  }

  // If max_conditions is greater than zero, re-allocate both ConditionSeqs to max_conditions
  if (max_conditions > 0) {
    RMW_TRY_PLACEMENT_NEW(
      waitset_info->active_conditions, waitset_info->active_conditions, goto fail,
      DDS::ConditionSeq, static_cast<DDS::ULong>(max_conditions))

    RMW_TRY_PLACEMENT_NEW(
      waitset_info->attached_conditions, waitset_info->attached_conditions, goto fail,
      DDS::ConditionSeq,
      static_cast<DDS::ULong>(max_conditions))
  } else {
    // Else, don't preallocate: the vectors will size dynamically when rmw_wait is called.
    // Default-construct the ConditionSeqs.
    RMW_TRY_PLACEMENT_NEW(
      waitset_info->active_conditions, waitset_info->active_conditions, goto fail,
      DDS::ConditionSeq)

    RMW_TRY_PLACEMENT_NEW(
      waitset_info->attached_conditions, waitset_info->attached_conditions, goto fail,
      DDS::ConditionSeq)
  }

  return waitset;

fail:
  if (waitset_info) {
    if (waitset_info->active_conditions) {
      // How to know which constructor threw?
      RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
        waitset_info->active_conditions->~DDS_DCPSUObjSeq(), ConditionSeq)
      rmw_free(waitset_info->active_conditions);
    }
    if (waitset_info->attached_conditions) {
      RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
        waitset_info->attached_conditions->~DDS_DCPSUObjSeq(), ConditionSeq)
      rmw_free(waitset_info->attached_conditions);
    }
    if (waitset_info->waitset) {
      RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(waitset_info->waitset->~WaitSet(), WaitSet)
      rmw_free(waitset_info->waitset);
    }
    waitset_info = nullptr;
  }
  if (waitset) {
    if (waitset->data) {
      rmw_free(waitset->data);
    }
    rmw_waitset_free(waitset);
  }
  return nullptr;
}

rmw_ret_t
rmw_destroy_waitset(rmw_waitset_t * waitset)
{
  if (!waitset) {
    RMW_SET_ERROR_MSG("waitset handle is null");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    waitset handle,
    waitset->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  auto result = RMW_RET_OK;
  OpenSpliceWaitSetInfo * waitset_info = static_cast<OpenSpliceWaitSetInfo *>(waitset->data);

  // Explicitly call destructor since the "placement new" was used

  if (waitset_info->active_conditions) {
    RMW_TRY_DESTRUCTOR(
      waitset_info->active_conditions->~DDS_DCPSUObjSeq(), ConditionSeq, result = RMW_RET_ERROR)
    rmw_free(waitset_info->active_conditions);
  }
  if (waitset_info->attached_conditions) {
    RMW_TRY_DESTRUCTOR(
      waitset_info->attached_conditions->~DDS_DCPSUObjSeq(), ConditionSeq, result = RMW_RET_ERROR)
    rmw_free(waitset_info->attached_conditions);
  }
  if (waitset_info->waitset) {
    RMW_TRY_DESTRUCTOR(waitset_info->waitset->~WaitSet(), WaitSet, result = RMW_RET_ERROR)
    rmw_free(waitset_info->waitset);
    waitset_info = nullptr;
  }
  if (waitset->data) {
    rmw_free(waitset->data);
  }
  if (waitset) {
    rmw_waitset_free(waitset);
  }
  return result;
}
}  // extern "C"
