#include "motor_ros2/motor_cfg.h"
#include "stdint.h"
#include <atomic>
#include <iostream>
#include <memory>
#include <rclcpp/node.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>
#include <thread>
#include <unistd.h>
#include <vector>
#include "std_msgs/msg/float64.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"




class MotorControlSample : public rclcpp::Node {
public:
  MotorControlSample()
      : rclcpp::Node("motor_control_set_node"),
        motor(RobStrideMotor("can0", 0xFF, 0x01, 2)) {
    
    // pub
    pub_motor_state_array = this->create_publisher<std_msgs::msg::Float64MultiArray>("motor_state", 10);

    // sub
    sub_position_command = this->create_subscription<std_msgs::msg::Float64>(
        "motor_cmd_position", 10,
        std::bind(&MotorControlSample::sub_position_command_callback,
                this,
                std::placeholders::_1));

    motor.Get_RobStrite_Motor_parameter(0x7005);
    usleep(1000);
    motor.enable_motor();
    usleep(1000);
    worker_thread_ = std::thread(&MotorControlSample::excute_loop, this);
  }

  ~MotorControlSample() {
    motor.Disenable_Motor(0);
    running_ = false; // 停止线程
    if (worker_thread_.joinable())
      worker_thread_.join(); // 等待线程结束
  }

  void excute_loop() {
    float position = 1.57f;
    float velocity = 0.0f;
    while (true) {
      // 自定义循环逻辑
      // 依次为速度，运控，位置模式, 电流，CSP位置
      position = target_position.load(std::memory_order_relaxed);

      auto [position_feedback, velocity_feedback, torque, temperature] =
           motor.send_motion_command(0.0, position, velocity, 1.1f, 0.1f);
          // motor.RobStrite_Motor_PosPP_control(velocity, 0.5f, position);
          //   motor.RobStrite_Motor_Current_control(-0.1);
          // motor.send_velocity_mode_command(5.0f);
          // motor.RobStrite_Motor_PosCSP_control(velocity, position);

      // 打包成 MultiArray
        std_msgs::msg::Float64MultiArray msg;
        msg.data = {position_feedback, velocity_feedback, torque, temperature};   // 注意顺序：位置,速度,扭矩,温度
        pub_motor_state_array->publish(msg);

      std::this_thread::sleep_for(std::chrono::milliseconds(1)); // loop rate
    }
  }

private:

  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr pub_motor_state_array;

  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr sub_position_command;

  std::atomic<float> target_position{0.0f};

  void sub_position_command_callback(const std_msgs::msg::Float64::SharedPtr msg)
  {
    target_position.store(msg->data,std::memory_order_relaxed);
  }

  std::thread worker_thread_;
  std::atomic<bool> running_ = true;

  RobStrideMotor motor;
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);

  auto controller = std::make_shared<MotorControlSample>();

  rclcpp::executors::MultiThreadedExecutor executor;

  executor.add_node(controller);

  executor.spin();

  rclcpp::shutdown();

  return 0;
}