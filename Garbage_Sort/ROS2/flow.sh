#!/bin/bash

exit 0


##############################################################################
# Terminal 1

source /opt/ros/jazzy/setup.bash
export RCUTILS_COLORIZED_OUTPUT=1

colcon build --symlink-install

source install/setup.sh

ros2 launch joy_teleop teleop.launch.py

ros2 launch routine routine.launch.py
ros2 run routine routine_node

ros2 node list
ros2 node info /routine_node
ros2 topic echo /joy

ros2 run stepper stepper_node