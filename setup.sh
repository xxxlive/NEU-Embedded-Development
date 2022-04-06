#!/bin/bash
# Please run this shell script as root

# here's the part of initialize face & gesture recognization
conda activate yolo
modprobe v4l2loopback devices=2
nohup ffmpeg -re -i /dev/video0 -f v4l2 /dev/video1 &
nohup ffmpeg -re -i /dev/video1 -f v4l2 /dev/video2 &
cd ~/Documents/temporal-shift-module/online_demo  # PATH
nohup python3 main.py &
cd ~/Documents/Ultra-Light-Fast-Generic-Face-Detector-1MB-master  # PATH
nohup python3 run_video_face_detect_onnx.py &

# booting up servers...
cd ~/Documents/NEU-Embedded-Development  # PATH
#python3 boot_server.py &

# monitor resource usage
jtop
