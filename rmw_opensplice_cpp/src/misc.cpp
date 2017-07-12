#include <ccpp_dds_dcps.h>
#include <dds_dcps.h>

#include "rcutils/format_string.h"
#include "rcutils/types.h"
#include "rcutils/split.h"

#include "rmw/error_handling.h"
#include "rmw/rmw.h"

#include "misc.hpp"

static const char * const ros_topic_prefix = "rt";

bool
process_topic_name(
  const char * topic_name,
  bool avoid_ros_namespace_conventions,
  char ** topic_str,
  char ** partition_str)
{
  bool success = true;
  rcutils_string_array_t name_tokens = rcutils_get_zero_initialized_string_array();
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (rcutils_split_last(topic_name, '/', allocator, &name_tokens) != RCUTILS_RET_OK) {
    RMW_SET_ERROR_MSG(rcutils_get_error_string_safe())
    success = false;
    goto end;
  }
  if (name_tokens.size == 1) {
    if (!avoid_ros_namespace_conventions) {
      *partition_str = DDS::string_dup(ros_topic_prefix);
    }
    *topic_str = DDS::string_dup(name_tokens.data[0]);
  } else if (name_tokens.size == 2) {
    if (avoid_ros_namespace_conventions) {
      // no ros_topic_prefix, so store the user's namespace directly
      *partition_str = DDS::string_dup(name_tokens.data[0]);
    } else {
      // concat the ros_topic_prefix with the user's namespace
      char * concat_str =
        rcutils_format_string(allocator, "%s/%s", ros_topic_prefix, name_tokens.data[0]);
      if (!concat_str) {
        RMW_SET_ERROR_MSG("could not allocate memory for partition string")
        success = false;
        goto end;
      }
      *partition_str = DDS::string_dup(concat_str);
      allocator.deallocate(concat_str, allocator.state);
    }
    *topic_str = DDS::string_dup(name_tokens.data[1]);
  } else {
    RMW_SET_ERROR_MSG("incorrectly formatted topic name")
    success = false;
  }

end:
  // all necessary strings are copied into opensplice
  // free that memory
  if (rcutils_string_array_fini(&name_tokens) != RCUTILS_RET_OK) {
    fprintf(stderr, "Failed to destroy the token string array\n");
  }
  return success;
}
