#include <Arduino.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <rmw_microros/rmw_microros.h>
#include <std_msgs/msg/empty.h>

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_subscription_t subscriber;

std_msgs__msg__Empty msg;

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Aborting.\n",__LINE__,(int)temp_rc);}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){printf("Failed status on line %d: %d. Continuing.\n",__LINE__,(int)temp_rc);}}

void subscription_callback(const void * msg_in) {
    digitalWrite(9, HIGH);
    delay(10);
    digitalWrite(9, LOW);
}

void setup() {
    pinMode(9, OUTPUT);
    digitalWrite(9, LOW);

    IPAddress ip(192, 168, 3, 202);
    uint16_t port = 8888;
    set_microros_wifi_transports("WiFi_Mulinex_3", "1234567890", ip, port);

    while (rmw_uros_ping_agent(100, 3) != RMW_RET_OK) {
        delay(100);
    }

    allocator = rcl_get_default_allocator();
    RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));
    RCCHECK(rclc_node_init_default(&node, "arm_laser", "", &support));
    RCCHECK(rclc_executor_init(&executor, &support.context, 1, &allocator));

    RCCHECK(rclc_subscription_init_default(
        &subscriber, &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Empty),
        "shoot"
    ));

    RCCHECK(rclc_executor_add_subscription(
        &executor, &subscriber, &msg,
        &subscription_callback, ON_NEW_DATA
    ));

    digitalWrite(9, LOW);
}

void loop() {
    RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(5)));
}