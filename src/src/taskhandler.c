
#include <FreeRTOS.h>
#include <stdio.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>
#include <task.h>

#define MAIN_LED_DELAY 800

void led_task(void *pvParameters);

void start_tasks();

char ssid[] = "3807444";
char pass[] = "berlin2022";


int main() {
    stdio_init_all();  // Initialize

    printf("Main Program Executation start!\n");

    start_tasks();
    

    return 0;
}

void start_tasks() {
    // Create Your Task
    xTaskCreate(
        led_task,    // Task to be run
        "LED_TASK",  // Name of the Task for debugging and managing its Task Handle
        1024,        // Stack depth to be allocated for use with task's stack (see docs)
        NULL,        // Arguments needed by the Task (NULL because we don't have any)
        1,           // Task Priority - Higher the number the more priority [max is (configMAX_PRIORITIES - 1) provided in FreeRTOSConfig.h]
        NULL         // Task Handle if available for managing the task
    );

    // Should start you scheduled Tasks (such as the LED_Task above)
    vTaskStartScheduler();

    while (true) {
        // Your program should never get here
    };
}

void led_task(void *pvParameters) {

    // Connect to wifi part
    bool is_connected = true;
    if (cyw43_arch_init_with_country(CYW43_COUNTRY_SWEDEN)) {
        printf("WiFi init failed\n");
        is_connected = false;
        return 1;
    }
    printf("initialised\n");

    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(ssid, pass, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("failed to connect\n");
        return 1;
    }
    printf("connected\n");

    

    // Blink led while connected to wifi
    while (is_connected) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        printf("LED tuned ON!\n");
        vTaskDelay(MAIN_LED_DELAY);  // Delay by TICKS defined by FreeRTOS priorities
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        printf("LED turned OFF\n");
        vTaskDelay(MAIN_LED_DELAY);
    }
}
