/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-11-09     xiangxistu   first version
 * 2020-04-20     chenyaxing   creat uart instance from strings input
 */

#include <stm32f4xx.h>
#include <string.h>
#include <stdlib.h>

static UART_HandleTypeDef handle;

/* stm32 config class */
struct stm32_uart_config
{
    const char *name;
    USART_TypeDef *Instance;
    IRQn_Type irq_type;

    GPIO_TypeDef *tx_port;
    GPIO_TypeDef *rx_port;
    uint32_t tx_pin;
    uint32_t rx_pin;
};

static struct stm32_uart_config *_uart_config = NULL;

static long stm32_uart_clk_enable(struct stm32_uart_config *config)
{
    switch ((uint32_t) config->Instance)
    {
#ifdef USART1
    case (uint32_t) USART1:
        __HAL_RCC_USART1_CLK_ENABLE()
        ;
        break;
#endif
#ifdef USART2
    case (uint32_t) USART2:
        __HAL_RCC_USART2_CLK_ENABLE()
        ;
        break;
#endif
#ifdef USART3
    case (uint32_t) USART3:
        __HAL_RCC_USART3_CLK_ENABLE()
        ;
        break;
#endif
#if defined(SOC_SERIES_STM32F0) || defined(SOC_SERIES_STM32L0) || \
   defined(SOC_SERIES_STM32G0)
#ifdef USART4
        case (uint32_t)USART4:
        __HAL_RCC_USART4_CLK_ENABLE();
        break;
#endif
#else
#ifdef UART4
    case (uint32_t) UART4:
        __HAL_RCC_UART4_CLK_ENABLE()
        ;
        break;
#endif
#endif

#if defined(SOC_SERIES_STM32F0) || defined(SOC_SERIES_STM32L0) || \
   defined(SOC_SERIES_STM32G0)
#ifdef USART5
        case (uint32_t)USART5:
        __HAL_RCC_USART5_CLK_ENABLE();
        break;
#endif
#else
#ifdef UART5
    case (uint32_t) UART5:
        __HAL_RCC_UART5_CLK_ENABLE()
        ;
        break;
#endif
#endif

#ifdef USART6
    case (uint32_t) USART6:
        __HAL_RCC_USART6_CLK_ENABLE()
        ;
        break;
#endif

#if defined(SOC_SERIES_STM32F0)
#ifdef USART7
        case (uint32_t)USART7:
        __HAL_RCC_USART7_CLK_ENABLE();
        break;
#endif
#else
#ifdef UART7
    case (uint32_t) UART7:
        __HAL_RCC_UART7_CLK_ENABLE()
        ;
        break;
#endif
#endif

#if defined(SOC_SERIES_STM32F0)
#ifdef USART8
        case (uint32_t)USART8:
        __HAL_RCC_USART8_CLK_ENABLE();
        break;
#endif
#else
#ifdef UART8
    case (uint32_t) UART8:
        __HAL_RCC_UART8_CLK_ENABLE()
        ;
        break;
#endif
#endif
#ifdef LPUART1
        case (uint32_t)LPUART1:
        __HAL_RCC_LPUART1_CLK_ENABLE();
        break;
#endif
    default:
        return -1;
    }

    return 0;
}

static long stm32_gpio_clk_enable(GPIO_TypeDef *gpiox)
{
    /* gpio ports clock enable */
    switch ((uint32_t) gpiox)
    {
#if defined(__HAL_RCC_GPIOA_CLK_ENABLE)
    case (uint32_t) GPIOA:
        __HAL_RCC_GPIOA_CLK_ENABLE()
        ;
        break;
#endif
#if defined(__HAL_RCC_GPIOB_CLK_ENABLE)
    case (uint32_t) GPIOB:
        __HAL_RCC_GPIOB_CLK_ENABLE()
        ;
        break;
#endif
#if defined(__HAL_RCC_GPIOC_CLK_ENABLE)
    case (uint32_t) GPIOC:
        __HAL_RCC_GPIOC_CLK_ENABLE()
        ;
        break;
#endif
#if defined(__HAL_RCC_GPIOD_CLK_ENABLE)
    case (uint32_t) GPIOD:
        __HAL_RCC_GPIOD_CLK_ENABLE()
        ;
        break;
#endif
#if defined(__HAL_RCC_GPIOE_CLK_ENABLE)
    case (uint32_t) GPIOE:
        __HAL_RCC_GPIOE_CLK_ENABLE()
        ;
        break;
#endif
#if defined(__HAL_RCC_GPIOF_CLK_ENABLE)
    case (uint32_t) GPIOF:
        __HAL_RCC_GPIOF_CLK_ENABLE()
        ;
        break;
#endif
#if defined(__HAL_RCC_GPIOG_CLK_ENABLE)
    case (uint32_t) GPIOG:
        __HAL_RCC_GPIOG_CLK_ENABLE()
        ;
        break;
#endif
#if defined(__HAL_RCC_GPIOH_CLK_ENABLE)
    case (uint32_t) GPIOH:
        __HAL_RCC_GPIOH_CLK_ENABLE()
        ;
        break;
#endif
#if defined(__HAL_RCC_GPIOI_CLK_ENABLE)
    case (uint32_t) GPIOI:
        __HAL_RCC_GPIOI_CLK_ENABLE()
        ;
        break;
#endif
#if defined(__HAL_RCC_GPIOJ_CLK_ENABLE)
    case (uint32_t) GPIOJ:
        __HAL_RCC_GPIOJ_CLK_ENABLE()
        ;
        break;
#endif
#if defined(__HAL_RCC_GPIOK_CLK_ENABLE)
    case (uint32_t) GPIOK:
        __HAL_RCC_GPIOK_CLK_ENABLE()
        ;
        break;
#endif
    default:
        return -1;
    }

    return 0;
}

static long stm32_gpio_configure(struct stm32_uart_config *config)
{
    int uart_num = 0;
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    uart_num = config->name[4] - '0';

    /* gpio ports clock enable */
    stm32_gpio_clk_enable(config->tx_port);
    if (config->tx_port != config->rx_port)
    {
        stm32_gpio_clk_enable(config->rx_port);
    }

    /* rx pin initialize */
    GPIO_InitStruct.Pin = config->tx_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
#if defined(SOC_SERIES_STM32F2) || defined(SOC_SERIES_STM32F4) || \
    defined(SOC_SERIES_STM32F7) || defined(SOC_SERIES_STM32G4) || \
    defined(SOC_SERIES_STM32L1) || defined(SOC_SERIES_STM32L4)
#define GPIO_AF7   ((uint8_t)0x07)
#define GPIO_AF8   ((uint8_t)0x08)
    /* uart1-3 -> AF7, uart4-8 -> AF8 */
    if (uart_num <= 3)
    {
        GPIO_InitStruct.Alternate = GPIO_AF7;
    }
    else
    {
        GPIO_InitStruct.Alternate = GPIO_AF8;
    }
#endif
    HAL_GPIO_Init(config->tx_port, &GPIO_InitStruct);

    /* rx pin initialize */
    GPIO_InitStruct.Pin = config->rx_pin;
    HAL_GPIO_Init(config->rx_port, &GPIO_InitStruct);

    return 0;
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    /* if this uart is shell function */
    if (huart == &handle)
    {
        stm32_gpio_configure(_uart_config);
    }
}

static long stm32_configure(struct stm32_uart_config *config)
{
    stm32_uart_clk_enable(config);

    handle.Instance = config->Instance;
    handle.Init.BaudRate = 115200;
    handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    handle.Init.Mode = UART_MODE_TX_RX;
    handle.Init.OverSampling = UART_OVERSAMPLING_16;
    handle.Init.WordLength = UART_WORDLENGTH_8B;
    handle.Init.StopBits = UART_STOPBITS_1;
    handle.Init.Parity = UART_PARITY_NONE;

    if (HAL_UART_Init(&handle) != HAL_OK)
    {
        return -1;
    }

    return 0;
}

char * up_string(char * str)
{
    char *ret = str;
    while (*str != '\0')
    {
        if ((*str >= 'a') && (*str <= 'z'))
        {
            *str = *str - 32;
            str++;
        }
        else
            str++;
    }
    return ret;
}

char * up_char(char * c)
{
    if ((*c >= 'a') && (*c <= 'z'))
    {
        *c = *c - 32;
    }
    return 0;
}

int rt_creat_usart_config(struct stm32_uart_config * uart_cfg, char* uart_name, char *tx_name, char* rx_name)
{
    int tx_pin_num = atoi((char*) &tx_name[1]);
    int rx_pin_num = atoi((char*) &rx_name[1]);
    char tx_port_name = tx_name[0];
    char rx_port_name = rx_name[0];
    up_char(&tx_port_name);
    up_char(&rx_port_name);

    if ((strcasecmp(uart_name, "uart1") == 0) || (strcasecmp(uart_name, "usart1") == 0))
    {
#ifdef USART1
        uart_cfg->name = "uart1";
        uart_cfg->Instance = USART1;
        uart_cfg->irq_type = USART1_IRQn;
#endif
    }
    else if (((strcasecmp(uart_name, "uart2") == 0) || (strcasecmp(uart_name, "usart2") == 0)))
    {
#ifdef USART2
        uart_cfg->name = "uart2";
        uart_cfg->Instance = USART2;
        uart_cfg->irq_type = USART2_IRQn;
#endif
    }
    else if (((strcasecmp(uart_name, "uart3") == 0) || (strcasecmp(uart_name, "usart3") == 0)))
    {
#ifdef USART3
        uart_cfg->name = "uart3";
        uart_cfg->Instance = USART3;
        uart_cfg->irq_type = USART3_IRQn;
#endif
    }
    else if (((strcasecmp(uart_name, "uart4") == 0) || (strcasecmp(uart_name, "usart4") == 0)))
    {
#ifdef UART4
        uart_cfg->name = "uart4";
        uart_cfg->Instance = UART4;
        uart_cfg->irq_type = UART4_IRQn;
#endif
    }
    else if (((strcasecmp(uart_name, "uart5") == 0) || (strcasecmp(uart_name, "usart5") == 0)))
    {
#ifdef UART5
        uart_cfg->name = "uart5";
        uart_cfg->Instance = UART5;
        uart_cfg->irq_type = UART5_IRQn;
#endif
    }
    else if (((strcasecmp(uart_name, "uart6") == 0) || (strcasecmp(uart_name, "usart6") == 0)))
    {
#ifdef USART6
        uart_cfg->name = "uart6";
        uart_cfg->Instance = USART6;
        uart_cfg->irq_type = USART6_IRQn;
#endif
    }
    else if (((strcasecmp(uart_name, "uart7") == 0) || (strcasecmp(uart_name, "usart7") == 0)))
    {
#ifdef UART7
        uart_cfg->name = "uart7";
        uart_cfg->Instance = UART7;
        uart_cfg->irq_type = UART7_IRQn;
#endif
    }
    else if (((strcasecmp(uart_name, "uart8") == 0) || (strcasecmp(uart_name, "usart8") == 0)))
    {
#ifdef UART8
        uart_cfg->name = "uart8";
        uart_cfg->Instance = UART8;
        uart_cfg->irq_type = UART8_IRQn;
#endif
    }
    uart_cfg->tx_port = ((GPIO_TypeDef *) ((uint32_t) GPIOA
            + (uint32_t) (tx_port_name - 'A') * ((uint32_t) GPIOB - (uint32_t) GPIOA)));
    uart_cfg->rx_port = ((GPIO_TypeDef *) ((uint32_t) GPIOA
            + (uint32_t) (rx_port_name - 'A') * ((uint32_t) GPIOB - (uint32_t) GPIOA)));
    uart_cfg->tx_pin = (GPIO_PIN_0 << tx_pin_num);
    uart_cfg->rx_pin = (GPIO_PIN_0 << rx_pin_num);

    return 0;
}

int rt_hw_usart_init(char* uart_name, char *tx_name, char* rx_name)
{
    struct stm32_uart_config uart_cfg;

    _uart_config = &uart_cfg;
    rt_creat_usart_config(&uart_cfg, uart_name, tx_name, rx_name);
    stm32_configure(&uart_cfg);

    return 0;
}

void print_char(char c)
{
    HAL_UART_Transmit(&handle, (uint8_t *) (&c), 1, 1);
}
