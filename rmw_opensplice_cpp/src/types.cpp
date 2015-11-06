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

#include <string>

std::string
create_type_name(
  const message_type_support_callbacks_t * callbacks,
  const std::string & sep)
{
  return std::string(callbacks->package_name) +
         "::" + sep + "::dds_::" + callbacks->message_name + "_";
}

void
CustomDataReaderListener::add_information(
  const DDS::SampleInfo & sample_info,
  const std::string & topic_name,
  const std::string & type_name)
{
  // store topic name and type name
  auto & topic_types = topic_names_and_types[topic_name];
  topic_types.insert(type_name);
  // store mapping to instance handle
  TopicDescriptor topic_descriptor;
  topic_descriptor.instance_handle = sample_info.instance_handle;
  topic_descriptor.name = topic_name;
  topic_descriptor.type = type_name;
  topic_descriptors.push_back(topic_descriptor);
}

void
CustomDataReaderListener::remove_information(const DDS::SampleInfo & sample_info)
{
  // find entry by instance handle
  for (auto it = topic_descriptors.begin(); it != topic_descriptors.end(); ++it) {
    if (it->instance_handle == sample_info.instance_handle) {
      // remove entries
      auto & topic_types = topic_names_and_types[it->name];
      topic_types.erase(topic_types.find(it->type));
      if (topic_types.empty()) {
        topic_names_and_types.erase(it->name);
      }
      topic_descriptors.erase(it);
      break;
    }
  }
}

void
CustomPublisherListener::on_data_available(DDS::DataReader * reader)
{
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
      add_information(info_seq[i], data_seq[i].topic_name.in(), data_seq[i].type_name.in());
    } else {
      remove_information(info_seq[i]);
    }
  }

  builtin_reader->return_loan(data_seq, info_seq);
}

void
CustomSubscriberListener::on_data_available(DDS::DataReader * reader)
{
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
      add_information(info_seq[i], data_seq[i].topic_name.in(), data_seq[i].type_name.in());
    } else {
      remove_information(info_seq[i]);
    }
  }

  builtin_reader->return_loan(data_seq, info_seq);
}
