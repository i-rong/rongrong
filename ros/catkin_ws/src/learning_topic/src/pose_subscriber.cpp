#include <ros/ros.h>
#include "turtlesim/Pose.h"

// 接收到订阅的消息后，会进入消息回调函数
void poseCallback(const turtlesim::Pose::ConstPtr& msg) {
    ROS_INFO("turtle pose: x%0.6f, y:%0.6f", msg->x, msg->y);
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "pose_subscriber");
    ros::NodeHandle n;
    ros::Subscriber pose_sub = n.subscribe("/turtle1/pose", 10, poseCallback);
    ros::spin(); // 循环等待回调函数
    return 0;
}
