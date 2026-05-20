from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    ld = LaunchDescription()
    node1 = Node(
        package='rs_motor_ros2',
        executable='motor_control_node',
        name='motor_control_node1',
        # remappings:作用是将节点中的话题名称进行重命名，以便与其他节点进行通信。
        # 在这个例子中，'/motor_state'和'/motor_cmd_position'这两个话题被重命名为相同的名称，这样其他节点就可以通过这些话题来订阅或发布消息。
        remappings=[
        ('/motor_state', '/motor_state_1'),
        ('/motor_cmd_position', '/motor_cmd_position_1')
        ],
        output="screen",
         # 为第一个节点设置 motor_id 参数
        parameters=[
            {"motor_id": 1} 
        ]
)
    ld.add_action(node1)
    
    node2=Node(
        package='rs_motor_ros2',
        executable='motor_control_node',
        name='motor_control_node2',
        remappings=[
        ('/motor_state', '/motor_state_2'),
        ('/motor_cmd_position', '/motor_cmd_position_2')
        ],
        output="screen",
        parameters=[
            {"motor_id": 2} 
        ]
    )
    ld.add_action(node2)

    return ld
