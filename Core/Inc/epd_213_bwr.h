#ifndef EPD_213_BWR_H
#define EPD_213_BWR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32g4xx_hal.h"

#define EPD213_WIDTH       122U
#define EPD213_HEIGHT      250U
#define EPD213_WIDTH_BYTES ((EPD213_WIDTH + 7U) / 8U)
#define EPD213_BUF_LEN     (EPD213_WIDTH_BYTES * EPD213_HEIGHT)

typedef struct
{
  SPI_HandleTypeDef *hspi;
  GPIO_TypeDef *cs_port;
  uint16_t cs_pin;
  GPIO_TypeDef *dc_port;
  uint16_t dc_pin;
  GPIO_TypeDef *res_port;
  uint16_t res_pin;
  GPIO_TypeDef *busy_port;
  uint16_t busy_pin;
} EPD213_HandleTypeDef;

HAL_StatusTypeDef EPD213_Init(const EPD213_HandleTypeDef *epd);
HAL_StatusTypeDef EPD213_Clear(const EPD213_HandleTypeDef *epd);
HAL_StatusTypeDef EPD213_TestPattern(const EPD213_HandleTypeDef *epd);
HAL_StatusTypeDef EPD213_ShowText(const EPD213_HandleTypeDef *epd, const char *text, uint16_t x, uint16_t y, uint8_t red_text);
HAL_StatusTypeDef EPD213_Sleep(const EPD213_HandleTypeDef *epd);
uint8_t EPD213_IsBusy(const EPD213_HandleTypeDef *epd);

#ifdef __cplusplus
}
#endif

#endif /* EPD_213_BWR_H */
