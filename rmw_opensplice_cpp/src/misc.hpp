#ifndef MISC_HPP
#define MISC_HPP_

RMW_LOCAL
bool process_topic_name(
  const char * topic_name,
  bool avoid_ros_namespace_conventions,
  char ** topic_str,
  char ** partition_str);

#endif  // MISC_HPP_
