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

#include <ccpp_dds_dcps.h>
#include <dds_dcps.h>

#include "rmw/allocators.h"
#include "rmw/error_handling.h"
#include "rmw/impl/cpp/macros.hpp"
#include "rmw/rmw.h"
#include "rmw/types.h"

#include "identifier.hpp"
#include "types.hpp"

#include "introspection_msgs/msg/ros_meta.h"
#include "rosidl_generator_c/string_functions.h"
#include "rosidl_generator_c/primitives_array_functions.h"

#include <sys/time.h>

// The extern "C" here enforces that overloading is not used.
extern "C"
{

rmw_ret_t
create_publisher_ros_meta(rmw_node_t * node)
{
  //publish node name  
  rmw_qos_profile_t qos_profile;
  qos_profile.depth = 10;
  qos_profile.reliability = RMW_QOS_POLICY_RELIABLE;
  qos_profile.history = RMW_QOS_POLICY_KEEP_LAST_HISTORY;
  qos_profile.durability = RMW_QOS_POLICY_TRANSIENT_LOCAL_DURABILITY;

  const rosidl_message_type_support_t * ts_rosmeta = ROSIDL_GET_MSG_TYPE_SUPPORT(introspection_msgs, ROSMeta);
  rmw_publisher_t * pub_rosmeta = rmw_create_publisher(node,
                                               ts_rosmeta,
                                               "ros_meta",
                                               &qos_profile);
  if(!pub_rosmeta)
  {
    RMW_SET_ERROR_MSG("create_publisher_ros_meta() could not create publisher");
    goto fail_introspection;
  }
  
  // reserve memory
  introspection_msgs__msg__ROSMeta rosmeta_msg;
  if(!introspection_msgs__msg__ROSMeta__init(&rosmeta_msg)){
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail_introspection;
  }  
  if(!rosidl_generator_c__String__assign(&rosmeta_msg.node_name, node->name)){
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail_introspection;
  }
  if(!rosidl_generator_c__uint8__Array__init(&rosmeta_msg.id, RMW_GID_STORAGE_SIZE)){
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail_introspection;
  }

  rmw_gid_t * gid ;
  {gid = (rmw_gid_t*)rmw_allocate(sizeof(rmw_gid_t));}
  if (!gid) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail_introspection;
  }
  // we don't sent the implementation identifier 
  // char* data_aux2 = (char*) rmw_allocate(sizeof(char)*strlen("OpenSplice")+1);
  // memcpy(data_aux2, "OpenSplice\0",strlen("OpenSplice")+1);
  // gid->implementation_identifier = data_aux2;
  if(rmw_get_gid_for_publisher(pub_rosmeta, gid) != RMW_RET_OK){
    RMW_SET_ERROR_MSG("unable to to get gid");
    goto fail_introspection;
  }
  
  for(unsigned i = 0; i < rosmeta_msg.id.size; i++){
    rosmeta_msg.id.data[i] = gid->data[i];
  }
  if (rmw_publish(pub_rosmeta, &rosmeta_msg) != RMW_RET_OK) {
    RMW_SET_ERROR_MSG("Error publishing message");
    goto fail_introspection;
  }

  //free memory
  rmw_free((rmw_gid_t*)gid);
  rosidl_generator_c__String__fini(&rosmeta_msg.node_name);
  rosidl_generator_c__uint8__Array__fini(&rosmeta_msg.id);
  introspection_msgs__msg__ROSMeta__fini(&rosmeta_msg);

  return RMW_RET_OK;
fail_introspection:
  if(pub_rosmeta){
    if (rmw_destroy_publisher(node, pub_rosmeta)){
      RMW_SET_ERROR_MSG("failed to delete subscriber");  
    }
  }
  if(gid){
    rmw_free((rmw_gid_t*)gid);
  }
  rosidl_generator_c__String__fini(&rosmeta_msg.node_name);
  rosidl_generator_c__uint8__Array__fini(&rosmeta_msg.id);
  introspection_msgs__msg__ROSMeta__fini(&rosmeta_msg);
  return RMW_RET_ERROR;
}

rmw_node_t *
rmw_create_node(const char * name, size_t domain_id)
{
  if (!name) {
    RMW_SET_ERROR_MSG("name is null");
    return nullptr;
  }
  DDS::DomainParticipantFactory_var dp_factory = DDS::DomainParticipantFactory::get_instance();
  if (!dp_factory) {
    RMW_SET_ERROR_MSG("failed to get domain participant factory");
    return nullptr;
  }

  DDS::DomainId_t domain = static_cast<DDS::DomainId_t>(domain_id);
  DDS::DomainParticipant * participant = nullptr;

  // Make sure that the OSPL_URI is set, otherwise node creation will fail.
  char * ospl_uri = nullptr;
  const char * ospl_uri_env = "OSPL_URI";
#ifndef _WIN32
  ospl_uri = getenv(ospl_uri_env);
#else
  size_t ospl_uri_size;
  _dupenv_s(&ospl_uri, &ospl_uri_size, ospl_uri_env);
#endif
  if (!ospl_uri) {
    RMW_SET_ERROR_MSG("OSPL_URI not set");
    return nullptr;
  } else {
#ifdef _WIN32
    free(ospl_uri);
#endif
  }

  // Ensure the ROS_DOMAIN_ID env variable is set, otherwise parsing of the config may fail.
  // Also make sure the it is set to the domain_id passed in, otherwise it will fail.
  // But first backup the current ROS_DOMAIN_ID.
  char * ros_domain_id = nullptr;
  const char * env_var = "ROS_DOMAIN_ID";
#ifndef _WIN32
  ros_domain_id = getenv(env_var);
#else
  size_t ros_domain_id_size;
  _dupenv_s(&ros_domain_id, &ros_domain_id_size, env_var);
#endif

  // On Windows, setting the ROS_DOMAIN_ID does not fix the problem, so error early.
#ifdef _WIN32
  if (!ros_domain_id) {
    RMW_SET_ERROR_MSG("environment variable ROS_DOMAIN_ID is not set");
    fprintf(stderr, "[rmw_opensplice_cpp]: error: %s\n", rmw_get_error_string_safe());
    return nullptr;
  }
#endif

  // Set the ROS_DOMAIN_ID explicitly (if not Windows).
#ifndef _WIN32
  auto domain_id_as_string = std::to_string(domain_id);
  int ret = 0;
  ret = setenv(env_var, domain_id_as_string.c_str(), 1);
  if (0 != ret) {
    RMW_SET_ERROR_MSG("failed to set the ROS_DOMAIN_ID");
    return nullptr;
  }
#endif

  participant = dp_factory->create_participant(
    domain, PARTICIPANT_QOS_DEFAULT, NULL, DDS::STATUS_MASK_NONE);
  if (!participant) {
    RMW_SET_ERROR_MSG("failed to create domain participant");
    return NULL;
  }

  // Restore the ROS_DOMAIN_ID if necessary (and not Windows).
#ifndef _WIN32
  if (ros_domain_id) {
    ret = setenv(env_var, ros_domain_id, 1);
    if (0 != ret) {
      RMW_SET_ERROR_MSG("failed to reset the ROS_DOMAIN_ID");
      return nullptr;
    }
  } else {
    // Otherwise unset it again.
    ret = unsetenv(env_var);
    if (0 != ret) {
      RMW_SET_ERROR_MSG("failed to unset the ROS_DOMAIN_ID");
      return nullptr;
    }
  }
#endif

  rmw_node_t * node = nullptr;
  OpenSpliceStaticNodeInfo * node_info = nullptr;
  CustomPublisherListener * publisher_listener = nullptr;
  CustomSubscriberListener * subscriber_listener = nullptr;
  void * buf = nullptr;

  DDS::DataReader * data_reader = nullptr;
  DDS::PublicationBuiltinTopicDataDataReader * builtin_publication_datareader = nullptr;
  DDS::SubscriptionBuiltinTopicDataDataReader * builtin_subscription_datareader = nullptr;
  DDS::Subscriber * builtin_subscriber = participant->get_builtin_subscriber();
  if (!builtin_subscriber) {
    RMW_SET_ERROR_MSG("builtin subscriber handle is null");
    goto fail;
  }

  // setup publisher listener
  data_reader = builtin_subscriber->lookup_datareader("DCPSPublication");
  builtin_publication_datareader =
    DDS::PublicationBuiltinTopicDataDataReader::_narrow(data_reader);
  if (!builtin_publication_datareader) {
    RMW_SET_ERROR_MSG("builtin publication datareader handle is null");
    goto fail;
  }

  buf = rmw_allocate(sizeof(CustomPublisherListener));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(publisher_listener, buf, goto fail, CustomPublisherListener)
  buf = nullptr;
  builtin_publication_datareader->set_listener(publisher_listener, DDS::DATA_AVAILABLE_STATUS);

  data_reader = builtin_subscriber->lookup_datareader("DCPSSubscription");
  builtin_subscription_datareader =
    DDS::SubscriptionBuiltinTopicDataDataReader::_narrow(data_reader);
  if (!builtin_subscription_datareader) {
    RMW_SET_ERROR_MSG("builtin subscription datareader handle is null");
    goto fail;
  }

  // setup subscriber listener
  buf = rmw_allocate(sizeof(CustomSubscriberListener));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(subscriber_listener, buf, goto fail, CustomSubscriberListener)
  buf = nullptr;
  builtin_subscription_datareader->set_listener(subscriber_listener, DDS::DATA_AVAILABLE_STATUS);

  node = rmw_node_allocate();
  if (!node) {
    RMW_SET_ERROR_MSG("failed to allocate rmw_node_t");
    goto fail;
  }

  node->name = reinterpret_cast<const char *>(rmw_allocate(sizeof(char) * strlen(name) + 1));
  if (!node->name) {
    RMW_SET_ERROR_MSG("failed to allocate memory for node name");
    goto fail;
  }
  memcpy(const_cast<char *>(node->name), name, strlen(name) + 1);

  buf = rmw_allocate(sizeof(OpenSpliceStaticNodeInfo));
  if (!buf) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    goto fail;
  }
  RMW_TRY_PLACEMENT_NEW(node_info, buf, goto fail, OpenSpliceStaticNodeInfo)
  buf = nullptr;
  node_info->participant = participant;
  node_info->publisher_listener = publisher_listener;
  node_info->subscriber_listener = subscriber_listener;

  node->implementation_identifier = opensplice_cpp_identifier;
  node->data = node_info;

  if(create_publisher_ros_meta(node) != RMW_RET_OK){
    RMW_SET_ERROR_MSG("Error creating publisher");
    goto fail;
  }
  
  return node;
fail:
  if (participant) {
    if (dp_factory->delete_participant(participant) != DDS::RETCODE_OK) {
      std::stringstream ss;
      ss << "leaking domain participant while handling failure at: " <<
        __FILE__ << ":" << __LINE__ << '\n';
      (std::cerr << ss.str()).flush();
    }
  }
  if (publisher_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      publisher_listener->~CustomPublisherListener(), CustomPublisherListener)
    rmw_free(publisher_listener);
  }
  if (subscriber_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      subscriber_listener->~CustomSubscriberListener(), CustomSubscriberListener)
    rmw_free(subscriber_listener);
  }
  if (node_info) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      node_info->~OpenSpliceStaticNodeInfo(), OpenSpliceStaticNodeInfo)
    rmw_free(node_info);
  }
  if (buf) {
    rmw_free(buf);
  }
  if (node) {
    if (node->name) {
      rmw_free(const_cast<char *>(node->name));
    }
    rmw_node_free(node);
  }
  return nullptr;
}

rmw_ret_t
rmw_destroy_node(rmw_node_t * node)
{
  if (!node) {
    RMW_SET_ERROR_MSG("received null pointer");
    return RMW_RET_ERROR;
  }
  RMW_CHECK_TYPE_IDENTIFIERS_MATCH(
    node handle,
    node->implementation_identifier, opensplice_cpp_identifier,
    return RMW_RET_ERROR)

  DDS::DomainParticipantFactory_var dp_factory = DDS::DomainParticipantFactory::get_instance();
  if (!dp_factory) {
    RMW_SET_ERROR_MSG("failed to get domain participant factory");
    return RMW_RET_ERROR;
  }
  auto node_info = static_cast<OpenSpliceStaticNodeInfo *>(node->data);
  if (!node_info) {
    RMW_SET_ERROR_MSG("node info handle is null");
    return RMW_RET_ERROR;
  }
  auto participant = static_cast<DDS::DomainParticipant *>(node_info->participant);
  if (!participant) {
    RMW_SET_ERROR_MSG("participant handle is null");
    return RMW_RET_ERROR;
  }

  auto result = RMW_RET_OK;
  // This unregisters types and destroys topics which were shared between
  // publishers and subscribers and could not be cleaned up in the delete functions.
  if (participant->delete_contained_entities() != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to delete contained entities of participant");
    result = RMW_RET_ERROR;
  }

  if (dp_factory->delete_participant(participant) != DDS::RETCODE_OK) {
    RMW_SET_ERROR_MSG("failed to delete participant");
    result = RMW_RET_ERROR;
  }

  if (node_info->publisher_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      node_info->publisher_listener->~CustomPublisherListener(), CustomPublisherListener)
    rmw_free(node_info->publisher_listener);
    node_info->publisher_listener = nullptr;
  }
  if (node_info->subscriber_listener) {
    RMW_TRY_DESTRUCTOR_FROM_WITHIN_FAILURE(
      node_info->subscriber_listener->~CustomSubscriberListener(), CustomSubscriberListener)
    rmw_free(node_info->subscriber_listener);
    node_info->subscriber_listener = nullptr;
  }

  rmw_free(node_info);
  node->data = nullptr;
  rmw_free(const_cast<char *>(node->name));
  node->name = nullptr;
  rmw_node_free(node);
  return result;
}

rmw_ros_meta_t * 
rmw_get_node_names(void)
{  
  // Use the current ROS_DOMAIN_ID.
  char * ros_domain_id = nullptr;
  const char * env_var = "ROS_DOMAIN_ID";
#ifndef _WIN32
  ros_domain_id = getenv(env_var);
#else
  size_t ros_domain_id_size;
  _dupenv_s(&ros_domain_id, &ros_domain_id_size, env_var);
#endif  
  size_t domain_id = std::stoi(ros_domain_id);

  // On Windows, setting the ROS_DOMAIN_ID does not fix the problem, so error early.
#ifdef _WIN32
  if (!ros_domain_id) {
    RMW_SET_ERROR_MSG("environment variable ROS_DOMAIN_ID is not set");
    fprintf(stderr, "[rmw_opensplice_cpp]: error: %s\n", rmw_get_error_string_safe());
    return nullptr;
  }
#endif

  // Create a ROS middleware node that will be used to 
  // fetch other participant's data from the "ros_meta" topic
  rmw_node_t * rmw_node;
  rmw_node = rmw_create_node("get_node_names", domain_id);

  // Do not use builtin topics but instead, make use of the "ros_meta"
  //  topic create for this purpose.
  rmw_qos_profile_t qos_profile;
  // TODO comment each one of these qos_profile options
  qos_profile.depth = 10;
  qos_profile.reliability = RMW_QOS_POLICY_RELIABLE;
  qos_profile.history = RMW_QOS_POLICY_KEEP_LAST_HISTORY;
  qos_profile.durability = RMW_QOS_POLICY_TRANSIENT_LOCAL_DURABILITY;
  const rosidl_message_type_support_t * ts_rosmeta = ROSIDL_GET_MSG_TYPE_SUPPORT(introspection_msgs, ROSMeta);
  rmw_subscription_t * subs = rmw_create_subscription(rmw_node,
                                                      ts_rosmeta,
                                                      "ros_meta", 
                                                      &qos_profile,
                                                      false);

  rmw_ros_meta_t* ros_meta_data = (rmw_ros_meta_t*)rmw_allocate(sizeof(rmw_ros_meta_t));
  ros_meta_data->count = 0;
  ros_meta_data->node_names = NULL;
  ros_meta_data->ids = NULL;
  if (!ros_meta_data) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    return NULL;
  }

  // FIXME change this message for one that contains also the gid
  introspection_msgs__msg__ROSMeta ros_message;
  introspection_msgs__msg__ROSMeta__init(&ros_message);
  bool taken = true;
  int count = 0;

  //timer variables
  struct timeval a, b;
  long totalb, totala;
  long diff;

  //get initial time
  gettimeofday(&a, NULL);
  totala = a.tv_sec + a.tv_usec/1000000;
  bool timeout = false;

  while(!timeout || taken){
    rmw_ret_t ret = rmw_take(subs, &ros_message, &taken); 
    if (ret != RMW_RET_OK) {
      RMW_SET_ERROR_MSG("failed to take message");    
    }else{
      if(ret == RMW_RET_OK && taken){
        count++;

        // Based on count, allocate new structures for ros_meta_data, adding the new
        //  one and freeing previous allocated memory
        rmw_string_t* node_names_aux = (rmw_string_t*) rmw_allocate(sizeof(rmw_string_t)*count);
        rmw_gid_t* ids_aux = (rmw_gid_t*) rmw_allocate(sizeof(rmw_gid_t)*count);
        for (int i=0;i < count-1; i++){

          char* data_aux = (char*) rmw_allocate(sizeof(char)*strlen(ros_meta_data->node_names[i].data)+1);
          memcpy(data_aux, ros_meta_data->node_names[i].data, strlen(ros_meta_data->node_names[i].data)+1);
          node_names_aux[i].data = data_aux;
          rmw_free((char*)ros_meta_data->node_names[i].data);

          char* implementation_identifier_aux = (char*) rmw_allocate(sizeof(char)*strlen(
              ros_meta_data->ids[i].implementation_identifier)+1);
          strcpy(implementation_identifier_aux, ros_meta_data->ids[i].implementation_identifier);
          ids_aux[i].implementation_identifier = implementation_identifier_aux;
          rmw_free((char*)ros_meta_data->ids[i].implementation_identifier);         
          
          memcpy(ids_aux[i].data, ros_meta_data->ids[i].data, sizeof(uint8_t) * RMW_GID_STORAGE_SIZE);
        }
        rmw_free((rmw_string_t *)ros_meta_data->node_names);
        rmw_free((rmw_gid_t *)ros_meta_data->ids);

        ros_meta_data->node_names = node_names_aux;
        ros_meta_data->ids = ids_aux;        

        // Now copy the new data
        char* data_aux = (char*) rmw_allocate(sizeof(char)*strlen(ros_message.node_name.data)+1);
        memcpy(data_aux, ros_message.node_name.data, ros_message.node_name.size+1);
        ros_meta_data->node_names[count-1].data = data_aux;

        char* data_aux2 = (char*) rmw_allocate(sizeof(char)*strlen(opensplice_cpp_identifier)+1);
        memcpy(data_aux2, opensplice_cpp_identifier,strlen(opensplice_cpp_identifier)+1);
        ros_meta_data->ids[count-1].implementation_identifier = data_aux2;

        memcpy(ros_meta_data->ids[count-1].data, ros_message.id.data, ros_message.id.size);

        // Assign the final count
        ros_meta_data->count = count;
      }      
    }
    gettimeofday(&b, NULL);
    totalb = b.tv_sec + b.tv_usec/1000000;
    diff = (totalb - totala);
    // wait for 1 millisecond
    if(diff > 0.001){
      timeout = true;
    }
  }

  introspection_msgs__msg__ROSMeta__fini(&ros_message);

  if (rmw_destroy_subscription(rmw_node, subs)){
    RMW_SET_ERROR_MSG("failed to delete subscriber");  
  }

  if (rmw_destroy_node(rmw_node) != RMW_RET_OK) {
    RMW_SET_ERROR_MSG("failed to delete participant");
  }
  return ros_meta_data;
}

rmw_ret_t 
rmw_destroy_ros_meta(rmw_ros_meta_t * rosmeta)
{
  if (!rosmeta) {
    RMW_SET_ERROR_MSG("received null pointer");
    return RMW_RET_ERROR;
  }

  for (int i=0;i < rosmeta->count; i++){
    if (!rosmeta->node_names[i].data) {
      RMW_SET_ERROR_MSG("received null pointer");
      return RMW_RET_ERROR;
    }
    rmw_free((char*)rosmeta->node_names[i].data);
    
    if (!rosmeta->ids[i].implementation_identifier) {
      RMW_SET_ERROR_MSG("received null pointer");
      return RMW_RET_ERROR;
    }
    rmw_free((char*)rosmeta->ids[i].implementation_identifier);
  }

  if (!rosmeta->node_names) {
    RMW_SET_ERROR_MSG("received null pointer");
    return RMW_RET_ERROR;
  }
  rmw_free((rmw_string_t *)rosmeta->node_names);

  if (!rosmeta->ids) {
    RMW_SET_ERROR_MSG("received null pointer");
    return RMW_RET_ERROR;
  }
  rmw_free((rmw_gid_t *)rosmeta->ids);

  rmw_free((rmw_ros_meta_t *)rosmeta);

  return RMW_RET_OK;
}
}  // extern "C"
