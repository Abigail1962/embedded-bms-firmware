#ifndef STM32F4XX_H
#define STM32F4XX_H

#include <stdint.h>

/* Mocked Register Structures for STM32F4 Peripherals */

typedef struct {
    volatile uint32_t MODER;    /* GPIO port mode register */
    volatile uint32_t OTYPER;   /* GPIO port output type register */
    volatile uint32_t OSPEEDR;  /* GPIO port output speed register */
    volatile uint32_t PUPDR;    /* GPIO port pull-up/pull-down register */
    volatile uint32_t IDR;      /* GPIO port input data register */
    volatile uint32_t ODR;      /* GPIO port output data register */
    volatile uint32_t BSRR;     /* GPIO port bit set/reset register */
} GPIO_TypeDef;

typedef struct {
    volatile uint32_t SR;       /* ADC status register */
    volatile uint32_t CR1;      /* ADC control register 1 */
    volatile uint32_t CR2;      /* ADC control register 2 */
    volatile uint32_t SMPR1;    /* ADC sample time register 1 */
    volatile uint32_t SMPR2;    /* ADC sample time register 2 */
    volatile uint32_t DR;       /* ADC regular data register */
} ADC_TypeDef;

typedef struct {
    volatile uint32_t CR;       /* DMA stream x control register */
    volatile uint32_t NDTR;     /* DMA stream x number of data register */
    volatile uint32_t PAR;      /* DMA stream x peripheral address register */
    volatile uint32_t M0AR;     /* DMA stream x memory 0 address register */
} DMA_Stream_TypeDef;

typedef struct {
    volatile uint32_t LISR;     /* DMA low interrupt status register */
    volatile uint32_t HISR;     /* DMA high interrupt status register */
    volatile uint32_t LIFCR;    /* DMA low interrupt flag clear register */
    volatile uint32_t HIFCR;    /* DMA high interrupt flag clear register */
    DMA_Stream_TypeDef Stream[8];
} DMA_TypeDef;

typedef struct {
    volatile uint32_t CR1;      /* I2C Control register 1 */
    volatile uint32_t CR2;      /* I2C Control register 2 */
    volatile uint32_t OAR1;     /* I2C Own address register 1 */
    volatile uint32_t OAR2;     /* I2C Own address register 2 */
    volatile uint32_t DR;       /* I2C Data register */
    volatile uint32_t SR1;      /* I2C Status register 1 */
    volatile uint32_t SR2;      /* I2C Status register 2 */
    volatile uint32_t CCR;      /* I2C Clock control register */
    volatile uint32_t TRISE;    /* I2C Rise time register */
} I2C_TypeDef;

typedef struct {
    volatile uint32_t SR;       /* USART Status register */
    volatile uint32_t DR;       /* USART Data register */
    volatile uint32_t BRR;      /* USART Baud rate register */
    volatile uint32_t CR1;      /* USART Control register 1 */
    volatile uint32_t CR2;      /* USART Control register 2 */
    volatile uint32_t CR3;      /* USART Control register 3 */
} USART_TypeDef;

typedef struct {
    volatile uint32_t CR1;      /* TIM control register 1 */
    volatile uint32_t DIER;     /* TIM interrupt enable register */
    volatile uint32_t SR;       /* TIM status register */
    volatile uint32_t CNT;      /* TIM counter register */
    volatile uint32_t PSC;      /* TIM prescaler register */
    volatile uint32_t ARR;      /* TIM auto-reload register */
} TIM_TypeDef;

/* Peripheral Instantiations (Mocked in C source files for local PC execution) */
extern GPIO_TypeDef *GPIOA;
extern GPIO_TypeDef *GPIOB;
extern ADC_TypeDef *ADC1;
extern DMA_TypeDef *DMA2;
extern I2C_TypeDef *I2C1;
extern USART_TypeDef *USART1;
extern TIM_TypeDef *TIM2;

/* Bit Definitions to match real STM32 peripheral headers */
#define ADC_CR2_ADON        ((uint32_t)0x00000001)
#define ADC_CR2_SWSTART     ((uint32_t)0x40000000)
#define ADC_CR2_DMA         ((uint32_t)0x00000100)
#define ADC_CR2_DDS         ((uint32_t)0x00000200)

#define DMA_SxCR_EN         ((uint32_t)0x00000001)
#define DMA_SxCR_TCIE       ((uint32_t)0x00000010)
#define DMA_SxCR_MINC       ((uint32_t)0x00000080)
#define DMA_SxCR_CIRC       ((uint32_t)0x00000100)
#define DMA_SxCR_DIR_0      ((uint32_t)0x00000040) // Memory-to-peripheral
#define DMA_SxCR_PSIZE_0    ((uint32_t)0x00000100) // 16-bit
#define DMA_SxCR_MSIZE_0    ((uint32_t)0x00000400) // 16-bit

#define I2C_CR1_PE          ((uint32_t)0x00000001)
#define I2C_CR1_START       ((uint32_t)0x00000100)
#define I2C_CR1_STOP        ((uint32_t)0x00000200)
#define I2C_CR1_ACK         ((uint32_t)0x00000400)

#define I2C_SR1_SB          ((uint32_t)0x00000001) // Start bit
#define I2C_SR1_ADDR        ((uint32_t)0x00000002) // Address sent
#define I2C_SR1_TXE         ((uint32_t)0x00000080) // Data register empty
#define I2C_SR1_RXNE        ((uint32_t)0x00000040) // Data register not empty

#define USART_CR1_UE        ((uint32_t)0x00002000)
#define USART_CR1_TE        ((uint32_t)0x00000008)
#define USART_CR1_RE        ((uint32_t)0x00000004)
#define USART_CR1_RXNEIE    ((uint32_t)0x00000020)

#define USART_SR_TXE        ((uint32_t)0x00000080)
#define USART_SR_RXNE       ((uint32_t)0x00000020)

#define TIM_CR1_CEN         ((uint32_t)0x00000001)
#define TIM_DIER_UIE        ((uint32_t)0x00000001)
#define TIM_SR_UIF          ((uint32_t)0x00000001)

#endif /* STM32F4XX_H */
