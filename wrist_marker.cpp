#include <ros/ros.h>
#include <interactive_markers/interactive_marker_server.h>
#include <tf2/LinearMath/Quaternion.h>
#include <sensor_msgs/JointState.h>

std::shared_ptr<ros::Publisher> joint_pub;
std::shared_ptr<ros::NodeHandle> node_handle;

void processFeedback(const visualization_msgs::InteractiveMarkerFeedbackConstPtr &feedback) {
    Eigen::MatrixXd wrist_joint_transform(3,3).setZero();
    tf2::Quaternion orientation;
    tf2::fromMsg(feedback.pose.orientation, orientation);
    auto orientation_matrix = tf2::Matrix3x3(orientation);
    tf2::tf2Scalar roll, pitch, yaw;
    Eigen::Vector3d = orientation_rpy(roll, pitch, yaw);
    Eigen::VectorXd joint_values = wrist_joint_transform * orientation_rpy;
    sensor_msgs::JointState joint_state;
    joint_state.name = ["a","b","c"];
    joint_state.position = joint_values;
    joint_state.stamp = ros::Time::now();
    static int count = 0;
    joint_state.seq = count++;
    joint_pub->publish(joint_state); 
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "wrist_marker");
    interactive_markers::InteractiveMarkerServer server("wrist_marker_server");

    ros::NodeHandle n;
    node_handle = &n;
    std::string s;
    n.getParam("wrist_frame", s)
    visualization_msgs::InteractiveMarker int_marker;
    int_marker.header.frame_id = s;
    int_marker.header.stamp = ros::Time::now();
    int_marker.name = "wrist_marker";
    int_marker.description = "Orientation control";
    
    visualization_msgs::InteractiveMarkerControl rotate_control;
    rotate_control.name = "orientation";
    rotate_control.interaction_mode = visualization_msgs::InteractiveMarkerControl::ROTATE_3D;

    int_marker.controls.push_back(rotate_control);

    server.insert(int_marker, &processFeedback);
    server.applyChanges();

    joint_pub = &n.advertise<joint_messages>("joint_state");

    ros::spin();
}