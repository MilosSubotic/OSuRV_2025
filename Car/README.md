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
-



  
Start up procedure:
-Turn on PI (Internal switch)
-SSH to PI
-go to ROS2/ackibot_ws_scripts
-./mars_joys.sh
-Turn on the joypad (RB+HOME)
-./ackibot_run_sbc.sh
-Turn on Motors (External switch)
-Enjoy
