/**
 * DM2 — Práctica 2: Señales Físicas
 * Módulo ADC — Implementación
 *
 * STM32F103C8T6 (Blue Pill)
 * Framework: STM32Cube HAL (PlatformIO)
 *
 * Actividades en este archivo:
 *   Act. 8  → adc_oversample_16()
 *   Act. 9  → adc_read_filtered() — oversampling
 *   Act. 10 → adc_read_filtered() — detección de saturación
 *   Act. 11 → diag_update() + diag_report_if_due()
 */

#include "adc.h"
#include <stdio.h>
#include <string.h>

/* ── Estado interno ──────────────────────────────────────────────────────── */
static adc_status_t _status = ADC_OK;

/* ── Variables de diagnóstico ────────────────────────────────────────────── */
static uint16_t diag_min         = 16380;
static uint16_t diag_max         = 0;
static uint32_t diag_count       = 0;
static uint32_t diag_last_report = 0;

/* ── adc_init ────────────────────────────────────────────────────────────── */
void adc_init(void)
{
    /* Calibración de hardware: reduce el error de offset del ADC.
     * Ejecutar una vez después de HAL_ADC_Init() y antes de la primera lectura.
     * Referencia: RM0008 §11.4 (ADC calibration)                            */
    HAL_ADCEx_Calibration_Start(&hadc1);
}

/* ── adc_read_raw ────────────────────────────────────────────────────────── */
uint16_t adc_read_raw(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    return (uint16_t)HAL_ADC_GetValue(&hadc1);
}

/* ── adc_to_volts ────────────────────────────────────────────────────────── */
float adc_to_volts(uint16_t code)
{
    /* code viene de adc_read_filtered(): 14-bit (máximo = 4095 × 4 = 16380) */
    return (float)code * (3.3f / (4095.0f * 4.0f));
}

/* ── adc_get_status ──────────────────────────────────────────────────────── */
adc_status_t adc_get_status(void)
{
    return _status;
}

/* ── adc_oversample_16 (función interna) ─────────────────────────────────── */
static uint16_t adc_oversample_16(void)
{
    /* TODO (Act. 8): Implementar oversampling ×16 con decimación.
     *
     * Fundamento matemático:
     *   Promediando N muestras independientes, la varianza del ruido
     *   se divide por N. La resolución efectiva aumenta en ½·log₂(N) bits.
     *   Con N=16 → varianza / 16, +2 bits efectivos (12 → 14 bits).
     *
     * Pasos:
     *   1. Acumular 16 llamadas a adc_read_raw() en un uint32_t.
     *   2. Dividir por 4 (equivale a desplazar 2 bits: √16 = 4).
     *   3. Retornar como uint16_t.
     *
     * Verificación de rango:
     *   Suma máxima = 4095 × 16 = 65520 → cabe en uint32_t (no en uint16_t).
     *   Resultado tras dividir por 4 = máximo 16380 → cabe en uint16_t.
     *
     * Sin HAL_Delay() entre muestras: las muestras consecutivas tienen
     * ruido térmico independiente, lo que permite el promediado estadístico.
     */

    return 0; /* TODO: reemplazar con la implementación */
}

/* ── adc_read_filtered ───────────────────────────────────────────────────── */
uint16_t adc_read_filtered(void)
{
    /* TODO (Act. 9 + Act. 10):
     *
     * Act. 9 — Oversampling:
     *   Llamar a adc_oversample_16() y guardar el resultado en una variable.
     *
     * Act. 10 — Detección de saturación:
     *   Comparar el valor con ADC_CLAMP_MIN y ADC_CLAMP_MAX.
     *
     *   Si val <= ADC_CLAMP_MIN:
     *     _status = ADC_SATURADO_MIN;
     *     Enviar "WARN: ADC_SAT_MIN\r\n" por UART (solo al detectar, no en spam)
     *
     *   Si val >= ADC_CLAMP_MAX:
     *     _status = ADC_SATURADO_MAX;
     *     Enviar "WARN: ADC_SAT_MAX\r\n" por UART
     *
     *   Si no:
     *     _status = ADC_OK;
     *
     *   Retornar el valor (con o sin saturación — el loop debe seguir).
     *
     * Pista para evitar spam:
     *   static uint8_t last_sat = 0;
     *   if (!last_sat) { HAL_UART_Transmit(...); }
     *   last_sat = 1; (y ponerlo a 0 cuando _status == ADC_OK)
     */

    return 0; /* TODO: reemplazar con la implementación */
}

/* ── diag_update ─────────────────────────────────────────────────────────── */
void diag_update(uint16_t val)
{
    /* TODO (Act. 11 — parte 1):
     *
     * Actualizar las estadísticas con la muestra actual:
     *   - Si val < diag_min  → actualizar diag_min
     *   - Si val > diag_max  → actualizar diag_max
     *   - Incrementar diag_count
     *
     * Esta función debe ser rápida (sin UART, sin delays).
     */
}

/* ── diag_report_if_due ──────────────────────────────────────────────────── */
void diag_report_if_due(void)
{
    /* TODO (Act. 11 — parte 2):
     *
     * Enviar un reporte de diagnóstico por UART cada 1000 ms.
     *
     * Formato exacto (para poder parsearlo en Python):
     *   "DIAG min=%u max=%u n=%lu\r\n"
     *
     * Algoritmo:
     *   uint32_t now = HAL_GetTick();
     *   if (now - diag_last_report >= 1000) {
     *       // formatear y transmitir
     *       // resetear diag_min = 16380, diag_max = 0, diag_count = 0
     *       diag_last_report = now;
     *   }
     *
     * Interpretar el reporte:
     *   n  ≈ 200–400 → tasa de muestreo del loop principal
     *   max - min    → rango de variación en el período (en LSB 14-bit)
     *   Si max - min > 200 con el pot fijo → problema eléctrico (GND ruidoso,
     *   cable flotante, Vref con ripple)
     */
}
