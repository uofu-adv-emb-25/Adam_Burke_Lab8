#include <can2040.h>
#include <hardware/regs/intctrl.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "FreeRTOSConfig.h"

static struct can2040 cbus;
QueueHandle_t rx_queue;

static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg)
{
    xQueueSendToBackFromISR(rx_queue, msg, NULL);
}

static void PIOx_IRQHandler(void)
{
    can2040_pio_irq_handler(&cbus);
}

void canbus_setup(void)
{
    uint32_t pio_num = 0;
    uint32_t sys_clock = 125000000, bitrate = 500000;
    uint32_t gpio_rx = 4, gpio_tx = 5;

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

void runReceive(__unused void* _) {
    struct can2040_msg rx_msg;
    while (1) {
        xQueueReceive(rx_queue, &rx_msg, portMAX_DELAY);
        printf("Received message: id = %x, data[0] = %x, data[1] = %x\n", rx_msg.id, rx_msg.data32[0], rx_msg.data32[1]);
    }
}

int main () {
    // setup IO
    stdio_init_all();
    canbus_setup();

    // setup parallelism
    TaskHandle_t receive_task;
    rx_queue = xQueueCreate(64, sizeof(struct can2040_msg));
    xTaskCreate(runReceive, "runReceive", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1UL, &receive_task);
    vTaskStartScheduler();

    while (1);
}