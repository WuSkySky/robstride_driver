
// #include "motor_ros2/motor_cfg.h"
// #include "stdint.h"
// #include <atomic>
// #include <iostream>
// #include <memory>
// #include <rclcpp/node.hpp>
// #include <rclcpp/rclcpp.hpp>
// #include <std_msgs/msg/string.hpp>
// #include <thread>
// #include <unistd.h>
// #include <vector>
// #include "std_msgs/msg/float64.hpp"
// #include "std_msgs/msg/float64_multi_array.hpp"
// #include "interface/msg/motor_state.hpp"
// #include "interface/msg/motor_state_array.hpp"

// using MotorState = interface::msg::MotorState;
// using MotorStateArray = interface::msg::MotorStateArray;

// class MotorControlSample : public rclcpp::Node {
// public:
//   MotorControlSample() : rclcpp::Node("motor_control_set_node") {

//     // 从 launch 读取参数
//     this->declare_parameter("can_interface", "can0");
//     this->declare_parameter("motor_ids", std::vector<int64_t>{1, 2});

//     std::string can_interface = this->get_parameter("can_interface").as_string();
//     std::vector<int64_t> motor_ids = this->get_parameter("motor_ids").as_integer_array();

//     RCLCPP_INFO(this->get_logger(), "CAN接口: %s", can_interface.c_str());
//     RCLCPP_INFO(this->get_logger(), "电机数量: %d", (int)motor_ids.size());

//     // 根据ID创建电机
//     for (int id : motor_ids) {
//       RCLCPP_INFO(this->get_logger(), "初始化电机 ID: %d", id);
//       auto motor = std::make_shared<RobStrideMotor>(can_interface.c_str(), 0xFF, id, 2);
//       motor->enable_motor();
//       motors_.push_back(motor);
//     }

//     // 发布状态
//     pub_motor_states = this->create_publisher<MotorStateArray>("motor_state", 10);

//     // 订阅指令
//     sub_position_command1 = this->create_subscription<std_msgs::msg::Float64>(
//       "motor1_cmd_position", 10,
//       std::bind(&MotorControlSample::sub_position_command1_callback, this, std::placeholders::_1));

//     sub_position_command2 = this->create_subscription<std_msgs::msg::Float64>(
//       "motor2_cmd_position", 10,
//       std::bind(&MotorControlSample::sub_position_command2_callback, this, std::placeholders::_1));

//     // 启动线程
//     worker_thread_ = std::thread(&MotorControlSample::excute_loop, this);
//   }

//   ~MotorControlSample() {
//     for (auto& m : motors_) m->Disenable_Motor(0);
//     running_ = false;
//     if (worker_thread_.joinable()) worker_thread_.join();
//   }

//   void excute_loop() {
//     float vel1 = 0.0f, vel2 = 0.0f;

//     while (running_ && rclcpp::ok()) {
//       if (motors_.size() < 2) {
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//         continue;
//       }

//       float t1 = target_position1;
//       float t2 = target_position2;

//       auto [pos_s, v_s, tor_s, tmp_s] = motors_[0]->send_motion_command(0, t1, vel1, 1.1, 0.1);
//       auto [pos_b, v_b, tor_b, tmp_b] = motors_[1]->send_motion_command(0, t2, vel2, 1.1, 0.1);

//       MotorStateArray msg;
//       MotorState s1, s2;
//       s1.position = pos_s; s1.velocity = v_s; s1.torque = tor_s; s1.temperature = tmp_s;
//       s2.position = pos_b; s2.velocity = v_b; s2.torque = tor_b; s2.temperature = tmp_b;
//       msg.states.push_back(s1);
//       msg.states.push_back(s2);

//       pub_motor_states->publish(msg);
//       std::this_thread::sleep_for(std::chrono::milliseconds(1));
//     }
//   }

// private:
//   std::vector<std::shared_ptr<RobStrideMotor>> motors_;
//   rclcpp::Publisher<MotorStateArray>::SharedPtr pub_motor_states;
//   rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr sub_position_command1;
//   rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr sub_position_command2;
//   std::atomic<float> target_position1{0}, target_position2{0};
//   std::thread worker_thread_;
//   std::atomic<bool> running_{true};

//   void sub_position_command1_callback(const std_msgs::msg::Float64::SharedPtr msg) { target_position1 = msg->data; }
//   void sub_position_command2_callback(const std_msgs::msg::Float64::SharedPtr msg) { target_position2 = msg->data; }
// };

// int main(int argc, char** argv) {
//   rclcpp::init(argc, argv);
//   auto node = std::make_shared<MotorControlSample>();
//   rclcpp::executors::MultiThreadedExecutor executor;
//   executor.add_node(node);
//   executor.spin();
//   rclcpp::shutdown();
//   return 0;
// }








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
#include "std_msgs/msg/float64.hpp"
#include "interface/msg/motor_state.hpp"

using MotorState = interface::msg::MotorState;

class SingleMotorControl : public rclcpp::Node {
public:
  SingleMotorControl() : rclcpp::Node("single_motor_node") {

    // 1. 声明和获取参数：这里只接收一个单独的整型 ID
    this->declare_parameter("can_interface", "can0");
    this->declare_parameter("motor_id", 1); // 默认ID为1

    std::string can_interface = this->get_parameter("can_interface").as_string();
    int motor_id = this->get_parameter("motor_id").as_int();

    RCLCPP_INFO(this->get_logger(), "节点 [%s] 启动！CAN: %s, 监控电机 ID: %d", 
                this->get_name(), can_interface.c_str(), motor_id);

    // 2. 实例化并使能单台电机
    motor_ = std::make_shared<RobStrideMotor>(can_interface.c_str(), 0xFF, motor_id, 2);
    motor_->enable_motor();

    // 3. 发布者：发布自己的状态（使用单电机消息类型 MotorState）
    // 动态映射话题名，比如叫：/motor1/state
    pub_motor_state = this->create_publisher<MotorState>("~/state", 10);

    // 4. 订阅者：接收自己的控制指令，话题名映射为：~/cmd_position
    sub_position_command = this->create_subscription<std_msgs::msg::Float64>(
      "~/cmd_position", 10,
      std::bind(&SingleMotorControl::sub_position_callback, this, std::placeholders::_1));

    worker_thread_ = std::thread(&SingleMotorControl::excute_loop, this);
  }

  ~SingleMotorControl() {
    if (motor_) motor_->Disenable_Motor(0);
    running_ = false;
    if (worker_thread_.joinable()) worker_thread_.join();
  }

  void excute_loop() {
    float vel = 0.0f;
    while (running_ && rclcpp::ok()) {
      float t = target_position.load(std::memory_order_relaxed);

      // 发送单台电机指令
      auto [pos, v, tor, tmp] = motor_->send_motion_command(0, t, vel, 1.1, 0.1);

      // 发布单台电机状态
      MotorState msg;
      msg.position = pos;
      msg.velocity = v;
      msg.torque = tor;
      msg.temperature = tmp;
      pub_motor_state->publish(msg);

      std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 1ms 闭环
    }
  }

private:
  std::shared_ptr<RobStrideMotor> motor_;
  rclcpp::Publisher<MotorState>::SharedPtr pub_motor_state;
  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr sub_position_command;
  std::atomic<float> target_position{0.0f};
  std::thread worker_thread_;
  std::atomic<bool> running_{true};

  void sub_position_callback(const std_msgs::msg::Float64::SharedPtr msg) { 
    target_position.store(msg->data, std::memory_order_relaxed); 
  }
};

int main(int argc, char** argv) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<SingleMotorControl>();
  rclcpp::spin(node); // 单节点单线程，直接用标准单线程 spin 即可，资源开销极小
  rclcpp::shutdown();
  return 0;
}






