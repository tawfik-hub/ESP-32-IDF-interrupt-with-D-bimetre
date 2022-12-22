#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_rom_gpio.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "esp_timer.h"
#define INPUT_PIN 15
#define LED_PIN 13

int state = 0;
int count = 0;
volatile int  Pulse_Count;
unsigned int  Liter_per_hour;
unsigned long Current_Time, Loop_Time; 
QueueHandle_t interputQueue;

static void IRAM_ATTR gpio_interrupt_handler(void *args)
{
    int pinNumber = (int)args;
    xQueueSendFromISR(interputQueue, &pinNumber, NULL);
}



void LED_Control_Task(void *params)
{
    int pinNumber; // count = 0;
    while (true)
    {
        if (xQueueReceive(interputQueue, &pinNumber, portMAX_DELAY))
        {
            //printf("GPIO %d was pressed %d times. The state is %d\n", pinNumber, count++, gpio_get_level(INPUT_PIN));
            count++;
            gpio_set_level(LED_PIN, gpio_get_level(INPUT_PIN));
        }
    }
}
esp_err_t esp_timer_early_init(void);
void app_main()
{
    esp_rom_gpio_pad_select_gpio(LED_PIN);
    //gpio_pad_select_gpio(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(INPUT_PIN);
    gpio_set_direction(INPUT_PIN, GPIO_MODE_INPUT);
    gpio_pulldown_en(INPUT_PIN);
    gpio_pullup_dis(INPUT_PIN);
    gpio_set_intr_type(INPUT_PIN, GPIO_INTR_POSEDGE);

    interputQueue = xQueueCreate(10, sizeof(int));
    xTaskCreate(LED_Control_Task, "LED_Control_Task", 2048, NULL, 1, NULL);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(INPUT_PIN, gpio_interrupt_handler, (void *)INPUT_PIN);
    while(true)
    {
        Liter_per_hour = (count * 60 / 7);
        count = 0;
        printf("%ud Liter/hour \n", Liter_per_hour);  
        vTaskDelay(100);
    }
}