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



from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    motor_node = Node(
        package="rs_motor_ros2",
        executable="motor_control_node",
        name="motor_control_node",
        output="screen",

        # ======================
        # 👉 在这里改电机ID！！！
        # ======================
        parameters=[
            {"can_interface": "can0"},
            {"motor_ids": [1, 2]}
        ]
    )

    return LaunchDescription([motor_node])