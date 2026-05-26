# 🚀 Line Follower Robot — BHU Winner

An autonomous line follower robot designed for competitive robotics events. This project combines embedded systems, control theory, sensor processing, and mechanical optimization to achieve highly accurate and stable line tracking alongwith maze solving algorithms for maze travesal.

## 🏆 Achievements 
  🥇 Winner at BHU Line Follower Competition
  ⚡ Optimized for high-speed track navigation
  🎯 Tuned PID control for stable movement
  🤖 Real-time sensor-based autonomous navigation

## ⚙️ Features
- High-speed PID-based line tracking
- Real-time sensor array processing
- Sharp turn recovery algorithm
- Differential drive motor control
- Speed ramping and acceleration control
- Lightweight chassis optimization
- Noise-filtered sensor readings
- Competition-oriented firmware optimization
  
## 🧠 Core Concepts Used
- Embedded Systems
- Arduino / ESP32 programming
- Timer-based control loops
- PWM motor control
- Sensor interfacing
- Control Systems
- PID control
- Error correction
- Dynamic speed adjustment
- Robotics
- Sensor fusion
- Autonomous navigation
  
## 🛠️ Hardware Used
| Component | Description |
|---|---|
| Microcontroller | ESP32 |
| Sensors | Analog QTR-8RC Sensor |
| Motor Driver | L298N |
| Motors | N20 12V DC Geared Motors |
| Battery | Li-ion |
| Chassis | Custom Lightweight Chassis |

## 🔌 Working Principle
  The sensor array continuously reads the line position and calculates an error value relative to the center of the robot.
  The PID controller processes this error:
  - P → reacts to present error
  - I → compensates accumulated error
  - D → predicts future error
  The controller dynamically adjusts motor speeds to keep the robot aligned with the track even at high speeds.

## 📈 PID Control
  The control equation:

```math
u(t)=K_p e(t)+K_i \int e(t)\,dt + K_d \frac{de(t)}{dt}
```

Where:
- `e(t)` = line position error
- `Kp` = proportional gain
- `Ki` = integral gain
- `Kd` = derivative gain


## 🎯 Optimization Techniques Used
**Mechanical**
  - Weight reduction
  - Lower center of gravity
  - Improved wheel traction
**Software**
  - Faster control loop timing
  - Reduced sensor noise
  - Adaptive motor speed tuning
  - Corner recovery handling
**Electrical**
  - Stable power delivery
  - Efficient motor driving
  - Noise filtering

## 🧪 Future Improvements
- Faster motors and high speed sensors
- Encoder-based closed-loop speed control
- Adaptive PID tuning
- Custom PCB revision
- Machine-learning-assisted path prediction
- ROS integration
- Edge AI enhancements
