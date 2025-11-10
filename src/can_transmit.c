#include <can2040.h>
#include <hardware/regs/intctrl.h>
#include <stdio.h>
#include <pico/stdlib.h>


static struct can2040 cbus;

static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg)
{
    printf("Transmit complete for message ID %d\n", msg->id);
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

int main () {
    
    stdio_init_all();
    canbus_setup();
    

    for(;;){
        int count = 1;
        struct can2040_msg tmsg;
            tmsg.id = 1;
            tmsg.dlc = 8;
            tmsg.data32[0] = 0xabcd;
            tmsg.data32[1] = 0x5555;
        int sts = can2040_transmit(&cbus, &tmsg);
        //printf("message sent times with status %d\n", sts);
        sleep_ms(1000);

    }
     
}