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

#include "types.hpp"

#include <algorithm>
#include <map>
#include <set>
#include <string>

#include "rmw/error_handling.h"

std::string
create_type_name(
  const message_type_support_callbacks_t * callbacks,
  const std::string & sep)
{
  return std::string(callbacks->package_name) +
         "::" + sep + "::dds_::" + callbacks->message_name + "_";
}

CustomDataReaderListener::CustomDataReaderListener()
: print_discovery_logging_(false)
{
  // TODO(wjwwood): replace this with actual console logging when we have that
  char * discovery_logging_value = nullptr;
  const char * discovery_logging_env = "RMW_PRINT_DISCOVERY_LOGGING";
  size_t discovery_logging_size;
#ifndef _WIN32
  discovery_logging_value = getenv(discovery_logging_env);
  if (discovery_logging_value) {
    discovery_logging_size = strlen(discovery_logging_value);
  }
#else
  _dupenv_s(&discovery_logging_value, &discovery_logging_size, discovery_logging_env);
#endif
  if (discovery_logging_value) {
    std::string str(discovery_logging_value, discovery_logging_size);
    std::string str_lower(str);
    std::transform(str_lower.begin(), str_lower.end(), str_lower.begin(), std::tolower);
    if (str != "0" && str_lower != "false" && str_lower != "off") {
      print_discovery_logging_ = true;
    }
#ifdef _WIN32
    free(ospl_uri);
#endif
  }
}

size_t
CustomDataReaderListener::count_topic(const char * topic_name)
{
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = topic_names_and_types_.find(topic_name);
  if (it == topic_names_and_types_.end()) {
    return 0;
  } else {
    return it->second.size();
  }
}

void
CustomDataReaderListener::fill_topic_names_and_types(
  std::map<std::string, std::set<std::string>> & tnat)
{
  std::lock_guard<std::mutex> lock(mutex_);
  for (auto it : topic_names_and_types_) {
    for (auto & jt : it.second) {
      tnat[it.first].insert(jt);
    }
  }
}

void
print_discovery_logging(
  const std::string & prefix,
  const std::string & name,
  const std::string & type,
  CustomDataReaderListener::EndPointType end_point_type)
{
  // filter builtin OpenSplice topics
  if (
    name.compare(0, 4, "DCPS") != 0 &&
    name.compare(0, 2, "d_") != 0 &&
    name.compare(0, 2, "q_") != 0 &&
    name.compare("CMParticipant") != 0)
  {
    std::string msg = prefix +
      ((end_point_type == CustomDataReaderListener::PublisherEP) ? "P" : "S") +
      ": " + name + " <" + type + ">";
    printf("%s\n", msg.c_str());
  }
}

void
CustomDataReaderListener::add_information(
  const DDS::SampleInfo & sample_info,
  const std::string & topic_name,
  const std::string & type_name,
  EndPointType end_point_type)
{
  // store topic name and type name
  auto & topic_types = topic_names_and_types_[topic_name];
  topic_types.insert(type_name);
  // store mapping to instance handle
  TopicDescriptor topic_descriptor;
  topic_descriptor.instance_handle = sample_info.instance_handle;
  topic_descriptor.name = topic_name;
  topic_descriptor.type = type_name;
  topic_descriptors_.push_back(topic_descriptor);
  if (print_discovery_logging_) {
    print_discovery_logging("+", topic_name, type_name, end_point_type);
  }
}

void
CustomDataReaderListener::remove_information(
  const DDS::SampleInfo & sample_info,
  EndPointType end_point_type)
{
  // find entry by instance handle
  for (auto it = topic_descriptors_.begin(); it != topic_descriptors_.end(); ++it) {
    if (it->instance_handle == sample_info.instance_handle) {
      if (print_discovery_logging_) {
        print_discovery_logging("-", it->name, it->type, end_point_type);
      }
      // remove entries
      auto & topic_types = topic_names_and_types_[it->name];
      topic_types.erase(topic_types.find(it->type));
      if (topic_types.empty()) {
        topic_names_and_types_.erase(it->name);
      }
      topic_descriptors_.erase(it);
      break;
    }
  }
}

CustomPublisherListener::CustomPublisherListener(rmw_guard_condition_t * graph_guard_condition)
: graph_guard_condition_(graph_guard_condition)
{
}

void
CustomPublisherListener::on_data_available(DDS::DataReader * reader)
{
  std::lock_guard<std::mutex> lock(mutex_);
  DDS::PublicationBuiltinTopicDataDataReader * builtin_reader =
    DDS::PublicationBuiltinTopicDataDataReader::_narrow(reader);

  DDS::PublicationBuiltinTopicDataSeq data_seq;
  DDS::SampleInfoSeq info_seq;
  DDS::ReturnCode_t retcode = builtin_reader->take(
    data_seq, info_seq, DDS::LENGTH_UNLIMITED,
    DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);

  if (retcode == DDS::RETCODE_NO_DATA) {
    return;
  }
  if (retcode != DDS::RETCODE_OK) {
    fprintf(stderr, "failed to access data from the built-in reader\n");
    return;
  }

  for (DDS::ULong i = 0; i < data_seq.length(); ++i) {
    if (info_seq[i].valid_data) {
      if (info_seq[i].instance_state == DDS::ALIVE_INSTANCE_STATE) {
        add_information(
          info_seq[i], data_seq[i].topic_name.in(), data_seq[i].type_name.in(), PublisherEP);
      } else {
        remove_information(info_seq[i], PublisherEP);
      }
    } else {
      remove_information(info_seq[i], PublisherEP);
    }
  }

  if (data_seq.length() > 0) {
    rmw_ret_t ret = rmw_trigger_guard_condition(graph_guard_condition_);
    if (ret != RMW_RET_OK) {
      fprintf(stderr, "failed to trigger graph guard condition: %s\n", rmw_get_error_string_safe());
    }
  }

  builtin_reader->return_loan(data_seq, info_seq);
}

CustomSubscriberListener::CustomSubscriberListener(rmw_guard_condition_t * graph_guard_condition)
: graph_guard_condition_(graph_guard_condition)
{
}

void
CustomSubscriberListener::on_data_available(DDS::DataReader * reader)
{
  std::lock_guard<std::mutex> lock(mutex_);
  DDS::SubscriptionBuiltinTopicDataDataReader * builtin_reader =
    DDS::SubscriptionBuiltinTopicDataDataReader::_narrow(reader);

  DDS::SubscriptionBuiltinTopicDataSeq data_seq;
  DDS::SampleInfoSeq info_seq;
  DDS::ReturnCode_t retcode = builtin_reader->take(
    data_seq, info_seq, DDS::LENGTH_UNLIMITED,
    DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);

  if (retcode == DDS::RETCODE_NO_DATA) {
    return;
  }
  if (retcode != DDS::RETCODE_OK) {
    fprintf(stderr, "failed to access data from the built-in reader\n");
    return;
  }

  for (DDS::ULong i = 0; i < data_seq.length(); ++i) {
    if (info_seq[i].valid_data) {
      if (info_seq[i].instance_state == DDS::ALIVE_INSTANCE_STATE) {
        add_information(
          info_seq[i], data_seq[i].topic_name.in(), data_seq[i].type_name.in(), SubscriberEP);
      } else {
        remove_information(info_seq[i], SubscriberEP);
      }
    } else {
      remove_information(info_seq[i], SubscriberEP);
    }
  }

  if (data_seq.length() > 0) {
    rmw_ret_t ret = rmw_trigger_guard_condition(graph_guard_condition_);
    if (ret != RMW_RET_OK) {
      fprintf(stderr, "failed to trigger graph guard condition: %s\n", rmw_get_error_string_safe());
    }
  }

  builtin_reader->return_loan(data_seq, info_seq);
}
