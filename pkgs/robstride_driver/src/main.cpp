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
#include "interface/msg/motor_state.hpp"
#include "interface/msg/motor_state_array.hpp"


//这个是C++ 的类型别名（type alias）
//作用:将 interface::msg::MotorState 这个长名字简写为 MotorState
using MotorState = interface::msg::MotorState; 
//多电机dsiplay状态
using MotorStateArray = interface::msg::MotorStateArray;



class MotorControlSample : public rclcpp::Node {
public:
  MotorControlSample()
      : rclcpp::Node("motor_control_set_node"),
        motor_small(RobStrideMotor("can0", 0xFF, 0x01, 2)),
        motor_big(RobStrideMotor("can0", 0xFF, 0x02, 2)){
    
    // pub
    pub_motor_states = this->create_publisher<MotorStateArray>("motor_state", 10);

    // sub
    sub_position_command1 = this->create_subscription<std_msgs::msg::Float64>(
        "motor1_cmd_position", 10,
        std::bind(&MotorControlSample::sub_position_command1_callback,
                this,
                std::placeholders::_1));
    sub_position_command2 = this->create_subscription<std_msgs::msg::Float64>(
        "motor2_cmd_position", 10,
        std::bind(&MotorControlSample::sub_position_command2_callback,
                this,
                std::placeholders::_1));
    
    

    motor_small.Get_RobStrite_Motor_parameter(0x7005);
    motor_big.Get_RobStrite_Motor_parameter(0x7005);
    usleep(1000);
    //motor.enable_motor();
    motor_small.enable_motor();
    motor_big.enable_motor();

    usleep(1000);
    worker_thread_ = std::thread(&MotorControlSample::excute_loop, this);
  }

  ~MotorControlSample() {
    motor_small.Disenable_Motor(0);
    motor_big.Disenable_Motor(0);
    running_ = false;//停止线程
    if (worker_thread_.joinable())
        worker_thread_.join();//等待线程结束
  }

  void excute_loop() {
    //float position = 1.57f;
    //float velocity = 0.0f;
    float position_small = 1.57f, position_big = 1.57f;
    float velocity_small = 0.0f, velocity_big = 0.0f;
    //改成running，不是死循环了
    while (running_) {
      // 自定义循环逻辑
      // 依次为速度，运控，位置模式, 电流，CSP位置
      //position = target_position.load(std::memory_order_relaxed);
      // 如果两个电机需要独立的目标位置，可以用两个原子变量分别控制
      // 简单起见，这里使用同一个 target_position 给两个电机
      float target1 = target_position1.load(std::memory_order_relaxed);
      float target2 = target_position2.load(std::memory_order_relaxed);

      //auto [position_feedback, velocity_feedback, torque, temperature] =
        //   motor.send_motion_command(0.0, position, velocity, 1.1f, 0.1f);
          // motor.RobStrite_Motor_PosPP_control(velocity, 0.5f, position);
          //   motor.RobStrite_Motor_Current_control(-0.1);
          // motor.send_velocity_mode_command(5.0f);
          // motor.RobStrite_Motor_PosCSP_control(velocity, position);
      
        
       // 获取小电机状态
        auto [pos_s, vel_s, tor_s, temp_s] = motor_small.send_motion_command(0.0, target1, velocity_small, 1.1f, 0.1f);
        // 获取大电机状态
        auto [pos_b, vel_b, tor_b, temp_b] = motor_big.send_motion_command(0.0, target2, velocity_big, 1.1f, 0.1f);

      // 发布消息
      MotorStateArray msg; 
      //msg.position = position_feedback;
      //msg.velocity = velocity_feedback;
      //msg.torque = torque;
      //msg.temperature = temperature;
      //pub_motor_state->publish(msg);
      MotorState state_small;
        state_small.position = pos_s;
        state_small.velocity = vel_s;
        state_small.torque = tor_s;
        state_small.temperature = temp_s;
        msg.states.push_back(state_small);
        
        MotorState state_big;
        state_big.position = pos_b;
        state_big.velocity = vel_b;
        state_big.torque = tor_b;
        state_big.temperature = temp_b;
        msg.states.push_back(state_big);
        
        pub_motor_states->publish(msg);


      std::this_thread::sleep_for(std::chrono::milliseconds(1)); // loop rate
    }
  }

private:

  //rclcpp::Publisher<MotorState>::SharedPtr pub_motor_state;
  RobStrideMotor motor_small;   // ID=1 的小电机
  RobStrideMotor motor_big;     // ID=2 的大电机
  rclcpp::Publisher<MotorStateArray>::SharedPtr pub_motor_states;

  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr sub_position_command1;
  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr sub_position_command2;


  std::atomic<float> target_position1{0.0f};
  std::atomic<float> target_position2{0.0f};

  void sub_position_command1_callback(const std_msgs::msg::Float64::SharedPtr msg)
  {
    target_position1.store(msg->data,std::memory_order_relaxed);
  }

  void sub_position_command2_callback(const std_msgs::msg::Float64::SharedPtr msg)
  {
    target_position2.store(msg->data,std::memory_order_relaxed);
  }

  std::thread worker_thread_;
  std::atomic<bool> running_ = true;

  //RobStrideMotor motor;
  //RobStrideMotor motor_small;   
  //RobStrideMotor motor_big; 
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