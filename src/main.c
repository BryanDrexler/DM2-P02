/**
 * DM2 — Práctica 2: Señales Físicas
 * ADC · Ruido · Filtrado · Telemetría
 *
 * STM32F103C8T6 (Blue Pill)
 * Framework: STM32Cube HAL (PlatformIO)
 * Board: bluepill_f103c8
 *
 * Pin map:
 *   PA0  → ADC1_IN0 (potenciómetro / divisor resistivo)
 *   PA9  → USART1_TX (al conversor USB-Serie de la PC)
 *   PA10 → USART1_RX
 *
 * Actividades en este archivo:
 *   Act. 3  → Verificar UART (código ya provisto)
 *   Act. 4  → stream_adc()    ← TODO
 *   Act. 5  → medir_ruido()   ← TODO
 *   Act. 11 → Loop principal  ← TODO
 *
 * Referencia: RM0008 (STM32F103 Reference Manual)
 */

#include "stm32f1xx_hal.h"
#include "adc.h"
#include <stdio.h>
#include <string.h>

/* ── Handles HAL — usados también desde adc.c (extern en adc.h) ─────────── */
ADC_HandleTypeDef  hadc1;
UART_HandleTypeDef huart1;

/* ── Número de muestras para streaming y medición de ruido ──────────────── */
#define N_MUESTRAS 1000

/* ── Prototipos internos ─────────────────────────────────────────────────── */
static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);

void stream_adc(void);
void medir_ruido(void);

/* ═══════════════════════════════════════════════════════════════════════════
 * main
 * ═══════════════════════════════════════════════════════════════════════════ */
int main(void)
{
    /* ── Inicialización del sistema ─────────────────────────────────────── */
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_ADC1_Init();
    adc_init();   /* calibración de hardware del ADC */

    /* ── Act. 3 — Verificar UART ────────────────────────────────────────── */
    /* Debe aparecer "hello" en el terminal serie antes de continuar.        */
    /* Sin UART funcionando, las actividades 4–11 son ciegas.               */
    char hello[] = "hello\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)hello, strlen(hello), 100);

    /* ── Act. 4 — Stream de muestras ───────────────────────────────────── */
    /* Descomentar para enviar N_MUESTRAS y visualizar en plot_serial.py    */
    /* stream_adc(); */

    /* ── Act. 5 — Medir ruido ───────────────────────────────────────────── */
    /* Descomentar para calcular media, varianza, min, max                  */
    /* medir_ruido(); */

    /* ── Acts. 9–11 — Loop principal ───────────────────────────────────── */
    while (1) {
        /* TODO (Act. 11): Una vez implementadas adc_read_filtered(),
         * diag_update() y diag_report_if_due() en adc.c, reemplazar
         * el cuerpo de este loop con:
         *
         *   uint16_t v = adc_read_filtered();
         *   diag_update(v);
         *   diag_report_if_due();
         *
         * No agregar HAL_Delay() aquí — el loop debe correr tan rápido
         * como sea posible para maximizar la tasa de muestreo.
         */
        HAL_Delay(1);   /* placeholder — eliminar cuando implementes el TODO */
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Act. 4 — stream_adc
 * Enviar N_MUESTRAS lecturas crudas a la PC por UART.
 * ═══════════════════════════════════════════════════════════════════════════ */
void stream_adc(void)
{
    /* TODO (Act. 4):
     *
     * Enviar N_MUESTRAS (1000) valores ADC por UART, uno por línea.
     *
     * Para cada muestra:
     *   1. Leer con adc_read_raw()
     *   2. Formatear: snprintf(buf, sizeof(buf), "%u\r\n", val)
     *   3. Transmitir: HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), 10)
     *   4. Esperar: HAL_Delay(5)   ← OK aquí (es un stream de debug, no loop de control)
     *
     * Verificar con:
     *   Linux/Mac: screen /dev/ttyUSB0 115200  (o minicom)
     *   Windows:   PuTTY → Serial → COMx → 115200
     *   Python:    python3 tools/plot_serial.py
     *
     * Qué observar:
     *   - Pot al mínimo  → valores cerca de 0
     *   - Pot al máximo  → valores cerca de 4095
     *   - Pot fijo       → ¿es estable? ¿cuánto varía?
     */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Act. 5 — medir_ruido
 * Tomar N_MUESTRAS y calcular estadísticas.
 * ═══════════════════════════════════════════════════════════════════════════ */
void medir_ruido(void)
{
    /* TODO (Act. 5):
     *
     * Tomar N_MUESTRAS muestras y calcular: media, varianza, min, max.
     *
     * Fórmula de varianza (sin división flotante):
     *   var = E[x²] - E[x]²
     *       = (suma2 / N) - (suma / N)²
     *
     * Variables necesarias:
     *   uint32_t suma  = 0;   // suma de valores
     *   uint32_t suma2 = 0;   // suma de cuadrados
     *   uint16_t min_val = 4095, max_val = 0;
     *
     * Advertencia de overflow:
     *   suma2 acumula v² con v ≤ 4095 → máximo = 4095² × 1000 ≈ 1.67 × 10¹⁰
     *   → no cabe en uint32_t (máx ~4.3 × 10⁹). Usar uint64_t para suma2,
     *   o calcular en bloques de 100 muestras.
     *   Alternativa simple: uint32_t suma2 con N=100 muestras (4095²×100 = 1.67×10⁹ ✓)
     *
     * Formato del mensaje de salida:
     *   "media=%lu var=%ld min=%u max=%u\r\n"
     *
     * Anotar en el README los tres escenarios:
     *   var_reposo     = ___   (pot fijo, cable corto)
     *   var_mano_cable = ___   (mano tocando el cable del ADC)
     *   var_mano_gnd   = ___   (mano tocando GND → ¿por qué baja el ruido?)
     */
}

/* ═══════════════════════════════════════════════════════════════════════════
 * Configuración del sistema — no modificar
 * ═══════════════════════════════════════════════════════════════════════════ */

void HAL_MspInit(void)
{
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    /* Deshabilitar JTAG para liberar PB3/PB4/PA15 como GPIO */
    __HAL_AFIO_REMAP_SWJ_NOJTAG();
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef       RCC_OscInit    = {0};
    RCC_ClkInitTypeDef       RCC_ClkInit    = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit  = {0};

    /* HSI interno a 8 MHz — sin PLL.
     * Ventaja: funciona en todas las placas (el cristal externo no siempre
     * está soldado en los clones Blue Pill).
     * SYSCLK = HCLK = PCLK2 = 8 MHz, PCLK1 = 4 MHz                        */
    RCC_OscInit.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInit.HSIState            = RCC_HSI_ON;
    RCC_OscInit.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInit.PLL.PLLState        = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&RCC_OscInit);

    RCC_ClkInit.ClockType      = RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK
                               | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInit.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInit.AHBCLKDivider  = RCC_SYSCLK_DIV1;   /* HCLK  = 8 MHz */
    RCC_ClkInit.APB1CLKDivider = RCC_HCLK_DIV2;     /* PCLK1 = 4 MHz (límite TIM2/3) */
    RCC_ClkInit.APB2CLKDivider = RCC_HCLK_DIV1;     /* PCLK2 = 8 MHz (USART1, ADC1) */
    HAL_RCC_ClockConfig(&RCC_ClkInit, FLASH_LATENCY_0);

    /* ADC clock = PCLK2 / 2 = 4 MHz (límite máximo del ADC: 14 MHz) */
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection    = RCC_ADCPCLK2_DIV2;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_Init = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* PA0: entrada analógica para ADC1_IN0 (potenciómetro) */
    GPIO_Init.Pin  = GPIO_PIN_0;
    GPIO_Init.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_Init);

    /* PA9 (USART1_TX): salida alternate function push-pull */
    GPIO_Init.Pin   = GPIO_PIN_9;
    GPIO_Init.Mode  = GPIO_MODE_AF_PP;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_Init);

    /* PA10 (USART1_RX): entrada flotante */
    GPIO_Init.Pin  = GPIO_PIN_10;
    GPIO_Init.Mode = GPIO_MODE_INPUT;
    GPIO_Init.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_Init);
}

static void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    __HAL_RCC_ADC1_CLK_ENABLE();

    hadc1.Instance                   = ADC1;
    hadc1.Init.ScanConvMode          = ADC_SCAN_DISABLE;
    hadc1.Init.ContinuousConvMode    = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion       = 1;
    HAL_ADC_Init(&hadc1);

    /* Canal 0 (PA0): sampling time = 28.5 ciclos.
     * Con pot (Rs ≤ 5 kΩ): τ = 5k × 4pF = 20 ns → t_min = 9τ = 180 ns.
     * 28.5 ciclos @ 4 MHz = 7.1 μs — con amplio margen.                    */
    sConfig.Channel      = ADC_CHANNEL_0;
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

static void MX_USART1_UART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();

    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart1);
}
