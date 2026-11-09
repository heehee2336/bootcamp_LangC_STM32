#include <stdint.h>
#include <stdbool.h>

#define RCC_BASE        0x40023800UL
#define GPIOA_BASE      0x40020000UL
#define GPIOB_BASE      0x40020400UL
#define GPIOC_BASE      0x40020800UL
#define GPIOD_BASE      0x40020C00UL

#define REG(addr)       (*(volatile uint32_t *)(addr))

#define RCC_AHB1ENR     REG(RCC_BASE + 0x30)

#define GPIOA_MODER     REG(GPIOA_BASE + 0x00)
#define GPIOA_OTYPER    REG(GPIOA_BASE + 0x04)
#define GPIOA_OSPEEDR   REG(GPIOA_BASE + 0x08)
#define GPIOA_PUPDR     REG(GPIOA_BASE + 0x0C)
#define GPIOA_ODR       REG(GPIOA_BASE + 0x14)

#define GPIOC_MODER     REG(GPIOC_BASE + 0x00)
#define GPIOC_PUPDR     REG(GPIOC_BASE + 0x0C)
#define GPIOC_IDR       REG(GPIOC_BASE + 0x10)

typedef struct{
	volatile uint32_t MODER;
	volatile uint32_t OTYPER;
	volatile uint32_t OSPEEDR;
	volatile uint32_t PUPDR;
	volatile uint32_t IDR;
	volatile uint32_t ODR;
	volatile uint32_t BSRR;
	volatile uint32_t LCKR;
	volatile uint32_t AFRL;
	volatile uint32_t AFRH;
} gpio_regs_t;

#define GPIOA ((gpio_regs_t *) GPIOA_BASE)
#define GPIOB ((gpio_regs_t *) GPIOB_BASE)
#define GPIOC ((gpio_regs_t *) GPIOC_BASE)
#define GPIOD ((gpio_regs_t *) GPIOD_BASE)

typedef enum{
	GPIO_MODE_IN, GPIO_MODE_OUT, GPIO_MODE_AF, GPIO_MODE_ANALOG
} gpio_mode_t;

typedef enum{
	GPIO_PULL_NONE, GPIO_PULL_UP, GPIO_PULL_DOWN
} gpio_pull_t;

void delay(volatile uint32_t n){
	while(n--);
}

void gpio_set_mode(gpio_regs_t *port, uint16_t pin, gpio_mode_t mode){
	uint32_t v = port->MODER;

	v &= ~(3 << (pin*2));
	v |= ((uint32_t)mode << (pin*2));

	port->MODER = v;
}

void gpio_set_pull(gpio_regs_t *port, uint16_t pin, gpio_pull_t pull){
	uint32_t v = port->PUPDR;
	v &= ~(3 << (pin*2));
	v |= ((uint32_t) pull << (pin *2));
	port->PUPDR = v;
}

void gpio_write(gpio_regs_t *port, uint16_t pin, bool level){
	port->BSRR = level ? (1 << pin) : (1 << (pin + 16));
}

bool gpio_read(const gpio_regs_t *port, uint16_t pin){
	return (port->IDR >> pin) & 1;
}

void gpio_toggle(gpio_regs_t *port, uint16_t pin){
	uint32_t mask = 1 << pin;
	uint32_t odr = port->ODR;
	port->BSRR = ((odr & mask) << 16) | (~odr &mask);
}





int main(void)
{
	// AHB1 RCC clock enable
	RCC_AHB1ENR |= (1 << 0);     // GPIOA clock enable
	RCC_AHB1ENR |= (1 << 2);     // GPIOC clock enable

/*
	// ===== PA5 : LED (output) =====
	// MODER: 01 Output mode
	GPIOA_MODER |= (1 << 10);
	GPIOA_MODER &= ~(1 << 11);

	// OTYPER: 0 Push-pull
	GPIOA_OTYPER &= ~(1 << 5);

	// OSPEEDR: 01 Medium speed
	GPIOA_OSPEEDR |= (1 << 10);
	GPIOA_OSPEEDR &= ~(1 << 11);

	// PUPDR: 01 Pull-up
	GPIOA_PUPDR |= (1 << 10);
	GPIOA_PUPDR &= ~(1 << 11);

	// ===== PC13 : USER button (input) =====
	// MODER: 00 Input mode
	GPIOC_MODER &= ~(1 << 26);
	GPIOC_MODER &= ~(1 << 27);

	// PUPDR: 01 Pull-up
	GPIOC_PUPDR |= (1 << 26);
	GPIOC_PUPDR &= ~(1 << 27);

	GPIOA->MODER |= (1 << 10); // GPIOA Base Address, MODER
	GPIOA->MODER &= ~(1 << 11); // GPIOA Base Address, MODER
	GPIOA->OTYPER &= ~(1 << 5); // push pull
	GPIOA->OSPEEDR |= (1 << 10); // Medium speed
	GPIOA->OSPEEDR &= ~(1 << 11); // Medium speed
	GPIOA->PUPDR |= (1 << 10); // Pull-up
	GPIOA->PUPDR &= ~(1 << 11); // Pull-up

	GPIOC->MODER &= ~(1 << 26);
	GPIOC->MODER &= ~(1 << 27);
	GPIOC->PUPDR |= (1 << 26); // Pull-up
	GPIOC->PUPDR &= ~(1 << 27); // Pull-up
*/

	gpio_set_mode(GPIOA, 5, GPIO_MODE_OUT);
	gpio_set_mode(GPIOA, 13, GPIO_MODE_IN);
	gpio_set_pull(GPIOA, 13, GPIO_PULL_UP);
	while(1)
	{
/*
		if((GPIOC->IDR & (1 << 13)) == 0){   // button pressed (Low)
			GPIOA->ODR |= (1 << 5);          // LED ON
		}else{
			GPIOA->ODR &= ~(1 << 5);         // LED OFF
		}
*/

		if(gpio_read(GPIOC, 13) == 0){
				gpio_write(GPIOA, 5, 1);
			}else{
				gpio_write(GPIOA, 5, 0);
			}
		}

	return 0;
}
