# from launch import LaunchDescription
# from launch_ros.actions import Node

# def generate_launch_description():

#     # 唯一正确的节点配置
#     motor_control_node = Node(
#         package="rs_motor_ros2",       # 1. 找哪个包？
#         executable="motor_control_node", # 2. 执行包里的哪个程序？
#         name="motor_control_node",     # 3. 运行起来后，在 ROS 2 里面叫什么名字？
#         output="screen",               # 4. 日志打印到哪里？
#     )

#     return LaunchDescription([
#         motor_control_node
#     ])



# from launch import LaunchDescription
# from launch_ros.actions import Node

# def generate_launch_description():
#     motor_node = Node(
#         package="rs_motor_ros2",
#         executable="motor_control_node",
#         name="motor_control_node",
#         output="screen",

#         # ======================
#         # 👉 在这里改电机ID！！！
#         # ======================
#         parameters=[
#             {"can_interface": "can0"},
#             {"motor_ids": [1, 2]}
#         ]
#     )

#     return LaunchDescription([motor_node])



# from launch import LaunchDescription
# from launch_ros.actions import Node

# def generate_launch_description():
#     return LaunchDescription([

#         # 电机 1 节点
#         Node(
#             package="rs_motor_ros2",
#             executable="motor_control_node",
#             name="motor1_node",
#             parameters=[{"motor_id": 1}]  # 电机1 ID
#         ),

#         # 电机 2 节点
#         Node(
#             package="rs_motor_ros2",
#             executable="motor_control_node",
#             name="motor2_node",
#             parameters=[{"motor_id": 2}]  # 电机2 ID
#         ),
#     ])

from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([

        # 🚀 节点 1：负责控制 ID 为 1 的电机
        Node(
            package="rs_motor_ros2",
            executable="motor_control_node",
            name="motor1_node",          # ROS2 节点名
            output="screen",
            parameters=[{"motor_id": 1}] # 传给 C++ 的单个 ID 参数
        ),

        # 🚀 节点 2：负责控制 ID 为 2 的电机
        Node(
            package="rs_motor_ros2",
            executable="motor_control_node",
            name="motor2_node",          # ROS2 节点名
            output="screen",
            parameters=[{"motor_id": 2}] # 传给 C++ 的单个 ID 参数
        ),
    ])