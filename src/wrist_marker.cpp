#include <ros/ros.h>
#include <interactive_markers/interactive_marker_server.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/convert.h>
#include <tf2/LinearMath/Matrix3x3.h>
#include <sensor_msgs/JointState.h>
#include <Eigen/Dense>
#include <memory>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

std::shared_ptr<ros::Publisher> joint_pub;
std::shared_ptr<ros::NodeHandle> node_handle;

void processFeedback(const visualization_msgs::InteractiveMarkerFeedbackConstPtr &feedback) {
    Eigen::MatrixXd wrist_joint_transform(3,3);
    wrist_joint_transform.setIdentity();
    tf2::Quaternion orientation;
    geometry_msgs::Quaternion quat_msg = feedback->pose.orientation;
    tf2::convert(quat_msg, orientation);

    auto orientation_matrix = tf2::Matrix3x3(orientation);
    double roll, pitch, yaw; 
    orientation_matrix.getRPY(roll, pitch, yaw);
    Eigen::Vector3d orientation_rpy(roll,pitch,yaw);
    Eigen::VectorXd joint_values = wrist_joint_transform * orientation_rpy;
    sensor_msgs::JointState joint_state;
    joint_state.name = {"a","b","c"};
    std::vector<double> data(joint_values.data(), joint_values.data() + joint_values.size());
    joint_state.position = data;
    joint_state.header.stamp = ros::Time::now();
    static int count = 0;
    joint_state.header.seq = count++;
    joint_pub->publish(joint_state); 
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "wrist_marker");
    interactive_markers::InteractiveMarkerServer server("wrist_marker_server");

    node_handle.reset(new ros::NodeHandle);
    ros::NodeHandle &n = *node_handle;
    //node_handle = make_shared<ros::NodeHandle>{);
    std::string s;
    n.getParam("wrist_frame", s);
    s = "wrist_marker";
    visualization_msgs::InteractiveMarker int_marker;
    int_marker.header.frame_id = s;
    int_marker.header.stamp = ros::Time::now();
    int_marker.name = "wrist_marker";
    int_marker.description = "Orientation control";
    
    visualization_msgs::InteractiveMarkerControl rotate_control;
    rotate_control.orientation.w = 1;
    rotate_control.orientation.x = 1;
    rotate_control.orientation.y = 0;
    rotate_control.orientation.z = 0;
    rotate_control.name = "rotate_x";
    rotate_control.interaction_mode = visualization_msgs::InteractiveMarkerControl::ROTATE_AXIS;
    int_marker.controls.push_back(rotate_control);

    rotate_control.orientation.w = 1;
    rotate_control.orientation.x = 0;
    rotate_control.orientation.y = 1;
    rotate_control.orientation.z = 0;
    rotate_control.name = "rotate_y";
    rotate_control.interaction_mode = visualization_msgs::InteractiveMarkerControl::ROTATE_AXIS;
    int_marker.controls.push_back(rotate_control);

    rotate_control.orientation.w = 1;
    rotate_control.orientation.x = 0;
    rotate_control.orientation.y = 0;
    rotate_control.orientation.z = 1;
    rotate_control.name = "rotate_z";
    rotate_control.interaction_mode = visualization_msgs::InteractiveMarkerControl::ROTATE_AXIS;
    int_marker.controls.push_back(rotate_control);

    server.insert(int_marker, &processFeedback);
    server.applyChanges();
    
    auto pub = n.advertise<sensor_msgs::JointState>("joint_states", 1);
    joint_pub.reset(&pub);

    ros::spin();
}