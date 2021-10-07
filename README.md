# NOTE: this is no longer supported in ROS 2, so this repository has been archived.

# A ROS 2 RMW implementation for Vortex OpenSplice

With the code in this repository, it is possible to use [*ROS 2*](https://index.ros.org/doc/ros2)
with [*Vortex OpenSplice*](https://github.com/ADLINK-IST/opensplice) as the underlying DDS
implementation.

## Known limitations

MANUAL_BY_TOPIC Liveliness QoS is not supported by OpenSplice when used with DDSi network service.
