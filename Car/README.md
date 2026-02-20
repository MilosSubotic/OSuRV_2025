Build instructions:
  https://github.com/cxxx1828/ROS2-Robot-Car

Infrastructure:
-PI5 acts as CPU
-Arduino NANO controls Motors
-PI tells NANO how to control BLDC and Servo
-NANO tells PI information about BLDC and Servo AND measurments from Sensors
-fw_pkgs.hpp is the packet structure (there is one copy in FW and one in ROS2/ackibot_node CHANGE BOTH!)
  -M2S is PI to NANO
  -S2M is NANO to PI
-FW/Arduino_Motor_Controller/Arduino_Motor_Controller.ino is the file you upload to Arduino
-ROS2/ackibot_ws/src/ackibot_node/src/fw_node.cpp is the file where you change PI instructions

Possible problems with PI Wi-Fi:
 Easiest is to connect PI to a Monitor and Wi-Fi and get IP from console. If it doesnt work, PING your PC from PI and it should now.

Start up procedure:
-Turn on PI (Internal switch)
-SSH to PI
-go to ROS2/ackibot_ws_scripts
-./mars_joys.sh
-Turn on the joypad (RB+HOME)
-./ackibot_run_sbc.sh
-Turn on Motors (External switch)
-Enjoy
