/**
 * DM2 — Práctica 2: Señales Físicas
 * Módulo ADC — Interfaz pública
 *
 * STM32F103C8T6 (Blue Pill) · ADC1 canal 0 (PA0)
 * Framework: STM32Cube HAL (PlatformIO)
 *
 * Esta interfaz es la que el resto del firmware ve.
 * La implementación vive en adc.c.
 */
#pragma once

#include <stdint.h>
#include "stm32f1xx_hal.h"

/* ── Handles HAL ─────────────────────────────────────────────────────────── */
/* Definidos en main.c — declarados extern para que adc.c los use           */
extern ADC_HandleTypeDef  hadc1;
extern UART_HandleTypeDef huart1;

/* ── Constantes ──────────────────────────────────────────────────────────── */
#define ADC_RESOLUTION   4095U   /* 12-bit: 0–4095 */
#define ADC_VREF_MV      3300U   /* Vref = VDD = 3.3 V */

/* Umbrales de saturación — en escala 14-bit (salida de adc_read_filtered) */
#define ADC_CLAMP_MIN     819U   /* < 5%  → probablemente 0 V               */
#define ADC_CLAMP_MAX   15561U   /* > 95% → probablemente Vcc               */

/* ── Tipos ───────────────────────────────────────────────────────────────── */
typedef enum {
    ADC_OK,
    ADC_SATURADO_MIN,
    ADC_SATURADO_MAX
} adc_status_t;

/* ── API pública ─────────────────────────────────────────────────────────── */

/**
 * @brief Inicializar el módulo ADC. Llamar desde main() antes de leer.
 *        Ejecuta calibración de hardware.
 */
void adc_init(void);

/**
 * @brief Leer una muestra cruda (12-bit, 0–4095).
 */
uint16_t adc_read_raw(void);

/**
 * @brief Leer valor filtrado con oversampling ×16 (14-bit, 0–16380).
 *        Detecta saturación y actualiza el estado interno.
 * @return Valor filtrado (Act. 9+10).
 */
uint16_t adc_read_filtered(void);

/**
 * @brief Convertir un código 14-bit de adc_read_filtered() a volts.
 */
float adc_to_volts(uint16_t code);

/**
 * @brief Obtener el último estado de saturación.
 */
adc_status_t adc_get_status(void);

/**
 * @brief Actualizar el registro de diagnóstico con una nueva muestra.
 *        Llamar en cada iteración del loop principal (Act. 11).
 */
void diag_update(uint16_t val);

/**
 * @brief Enviar reporte de diagnóstico por UART si pasó ≥ 1 segundo.
 *        Llamar en cada iteración del loop principal (Act. 11).
 */
void diag_report_if_due(void);


//uint16_t adc_oversample_16(void);
