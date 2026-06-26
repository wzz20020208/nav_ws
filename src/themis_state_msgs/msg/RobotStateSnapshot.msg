std_msgs/Header header

geometry_msgs/Point position
geometry_msgs/Quaternion orientation

geometry_msgs/Vector3 velocity

geometry_msgs/Vector3 angular_rate

geometry_msgs/Vector3 accel_bias
geometry_msgs/Vector3 gyro_bias


#[position(3), velocity(3), rotation_error(3), accel_bias(3), gyro_bias(3)]
float64[225] covariance
