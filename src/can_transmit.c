#include <can2040.h>
#include <hardware/regs/intctrl.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
# include "FreeRTOSConfig.h"


static struct can2040 cbus;
QueueHandle_t msgs;

static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg)
{
    // Send received message to queue
    xQueueSendFromISR(msgs, msg, NULL);
}

static void PIOx_IRQHandler(void)
{
    can2040_pio_irq_handler(&cbus);
}


void canbus_setup(void)
{
    uint32_t pio_num = 0;
    uint32_t sys_clock = 125000000, bitrate = 500000;
    uint32_t gpio_rx = 4, gpio_tx = 5 ;

    // Setup canbus
    can2040_setup(&cbus, pio_num);
    can2040_callback_config(&cbus, can2040_cb);

    // Enable irqs
    irq_set_exclusive_handler(PIO0_IRQ_0, PIOx_IRQHandler);
    irq_set_priority(PIO0_IRQ_0, PICO_DEFAULT_IRQ_PRIORITY - 1);
    irq_set_enabled(PIO0_IRQ_0, 1);

    // Start canbus
    can2040_start(&cbus, sys_clock, bitrate, gpio_rx, gpio_tx);
}

void main_task(__unused void *params)
{
    // Create queue for received messages
    struct can2040_msg data;
    xQueueReceiveFromISR(msgs, &data, NULL);

    // Transmit messages
    for(;;){
        int count = 1;
        struct can2040_msg tmsg;
            // ID is priority encoded 1 = highest priority
            tmsg.id = 1;
            tmsg.dlc = 8;
            tmsg.data32[0] = 0xabcd;
            tmsg.data32[1] = 0x5555;

        // Transmit message
        int sts = can2040_transmit(&cbus, &tmsg);
        sleep_ms(1000);
    }
    
}

int main () {
    // Initialize stdio
    stdio_init_all();
    canbus_setup();

    // Create queue for received messages
    msgs = xQueueCreate(100, sizeof(struct can2040_msg));

    // Create main task
    TaskHandle_t task;
    xTaskCreate(main_task, "MainThread", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &task);
        
    vTaskStartScheduler();
    return 0;
  
}