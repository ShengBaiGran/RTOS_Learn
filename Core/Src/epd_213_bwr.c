#include "epd_213_bwr.h"

#include <string.h>

#define EPD213_SPI_TIMEOUT_MS       1000U
#define EPD213_BUSY_TIMEOUT_MS      20000U
#define EPD213_BUSY_ACTIVE_STATE    GPIO_PIN_SET
#define EPD213_RAM_HEIGHT_INTERNAL  296U
#define EPD213_RAM_Y_END            (EPD213_RAM_HEIGHT_INTERNAL - 1U)

typedef struct
{
  char ch;
  uint8_t col[5];
} EPD213_Glyph5x7;

static const uint8_t k_epd213_glyph_question[5] = {0x02U, 0x01U, 0x51U, 0x09U, 0x06U};
static const EPD213_Glyph5x7 k_epd213_font5x7[] =
{
  {' ', {0x00U, 0x00U, 0x00U, 0x00U, 0x00U}},
  {'!', {0x00U, 0x00U, 0x5FU, 0x00U, 0x00U}},
  {'.', {0x00U, 0x60U, 0x60U, 0x00U, 0x00U}},
  {',', {0x00U, 0x80U, 0x60U, 0x00U, 0x00U}},
  {':', {0x00U, 0x36U, 0x36U, 0x00U, 0x00U}},
  {'-', {0x08U, 0x08U, 0x08U, 0x08U, 0x08U}},
  {'_', {0x40U, 0x40U, 0x40U, 0x40U, 0x40U}},
  {'/', {0x20U, 0x10U, 0x08U, 0x04U, 0x02U}},
  {'+', {0x08U, 0x08U, 0x3EU, 0x08U, 0x08U}},
  {'=', {0x14U, 0x14U, 0x14U, 0x14U, 0x14U}},
  {'(', {0x00U, 0x1CU, 0x22U, 0x41U, 0x00U}},
  {')', {0x00U, 0x41U, 0x22U, 0x1CU, 0x00U}},
  {'0', {0x3EU, 0x51U, 0x49U, 0x45U, 0x3EU}},
  {'1', {0x00U, 0x42U, 0x7FU, 0x40U, 0x00U}},
  {'2', {0x42U, 0x61U, 0x51U, 0x49U, 0x46U}},
  {'3', {0x21U, 0x41U, 0x45U, 0x4BU, 0x31U}},
  {'4', {0x18U, 0x14U, 0x12U, 0x7FU, 0x10U}},
  {'5', {0x27U, 0x45U, 0x45U, 0x45U, 0x39U}},
  {'6', {0x3CU, 0x4AU, 0x49U, 0x49U, 0x30U}},
  {'7', {0x01U, 0x71U, 0x09U, 0x05U, 0x03U}},
  {'8', {0x36U, 0x49U, 0x49U, 0x49U, 0x36U}},
  {'9', {0x06U, 0x49U, 0x49U, 0x29U, 0x1EU}},
  {'A', {0x7EU, 0x11U, 0x11U, 0x11U, 0x7EU}},
  {'B', {0x7FU, 0x49U, 0x49U, 0x49U, 0x36U}},
  {'C', {0x3EU, 0x41U, 0x41U, 0x41U, 0x22U}},
  {'D', {0x7FU, 0x41U, 0x41U, 0x22U, 0x1CU}},
  {'E', {0x7FU, 0x49U, 0x49U, 0x49U, 0x41U}},
  {'F', {0x7FU, 0x09U, 0x09U, 0x09U, 0x01U}},
  {'G', {0x3EU, 0x41U, 0x49U, 0x49U, 0x3AU}},
  {'H', {0x7FU, 0x08U, 0x08U, 0x08U, 0x7FU}},
  {'I', {0x00U, 0x41U, 0x7FU, 0x41U, 0x00U}},
  {'J', {0x20U, 0x40U, 0x41U, 0x3FU, 0x01U}},
  {'K', {0x7FU, 0x08U, 0x14U, 0x22U, 0x41U}},
  {'L', {0x7FU, 0x40U, 0x40U, 0x40U, 0x40U}},
  {'M', {0x7FU, 0x02U, 0x0CU, 0x02U, 0x7FU}},
  {'N', {0x7FU, 0x04U, 0x08U, 0x10U, 0x7FU}},
  {'O', {0x3EU, 0x41U, 0x41U, 0x41U, 0x3EU}},
  {'P', {0x7FU, 0x09U, 0x09U, 0x09U, 0x06U}},
  {'Q', {0x3EU, 0x41U, 0x51U, 0x21U, 0x5EU}},
  {'R', {0x7FU, 0x09U, 0x19U, 0x29U, 0x46U}},
  {'S', {0x46U, 0x49U, 0x49U, 0x49U, 0x31U}},
  {'T', {0x01U, 0x01U, 0x7FU, 0x01U, 0x01U}},
  {'U', {0x3FU, 0x40U, 0x40U, 0x40U, 0x3FU}},
  {'V', {0x1FU, 0x20U, 0x40U, 0x20U, 0x1FU}},
  {'W', {0x7FU, 0x20U, 0x18U, 0x20U, 0x7FU}},
  {'X', {0x63U, 0x14U, 0x08U, 0x14U, 0x63U}},
  {'Y', {0x03U, 0x04U, 0x78U, 0x04U, 0x03U}},
  {'Z', {0x61U, 0x51U, 0x49U, 0x45U, 0x43U}}
};

static HAL_StatusTypeDef EPD213_WriteCommand(const EPD213_HandleTypeDef *epd, uint8_t cmd);
static HAL_StatusTypeDef EPD213_WriteData(const EPD213_HandleTypeDef *epd, uint8_t data);
static HAL_StatusTypeDef EPD213_WriteDataBuffer(const EPD213_HandleTypeDef *epd, const uint8_t *buf, uint32_t len);
static HAL_StatusTypeDef EPD213_WaitBusy(const EPD213_HandleTypeDef *epd, uint32_t timeout_ms);
static void EPD213_Reset(const EPD213_HandleTypeDef *epd);
static HAL_StatusTypeDef EPD213_SetPos(const EPD213_HandleTypeDef *epd, uint16_t x, uint16_t y);
static HAL_StatusTypeDef EPD213_Update(const EPD213_HandleTypeDef *epd);
static HAL_StatusTypeDef EPD213_Display(const EPD213_HandleTypeDef *epd, const uint8_t *bw, const uint8_t *red);
static const uint8_t *EPD213_FindGlyph(char c);
static void EPD213_DrawChar5x7(uint8_t *buf, uint16_t x, uint16_t y, const uint8_t glyph[5]);
static void EPD213_SetPixel(uint8_t *buf, uint16_t x, uint16_t y, uint8_t colored);

uint8_t EPD213_IsBusy(const EPD213_HandleTypeDef *epd)
{
  if (epd == NULL)
  {
    return 0U;
  }
  return (HAL_GPIO_ReadPin(epd->busy_port, epd->busy_pin) == EPD213_BUSY_ACTIVE_STATE) ? 1U : 0U;
}

HAL_StatusTypeDef EPD213_Init(const EPD213_HandleTypeDef *epd)
{
  HAL_StatusTypeDef status;

  if ((epd == NULL) || (epd->hspi == NULL))
  {
    return HAL_ERROR;
  }

  EPD213_Reset(epd);
  status = EPD213_WaitBusy(epd, EPD213_BUSY_TIMEOUT_MS);
  if (status != HAL_OK)
  {
    return status;
  }

  status = EPD213_WriteCommand(epd, 0x12U); /* SWRESET */
  if (status != HAL_OK)
  {
    return status;
  }
  HAL_Delay(100U);
  status = EPD213_WaitBusy(epd, EPD213_BUSY_TIMEOUT_MS);
  if (status != HAL_OK)
  {
    return status;
  }

  status = EPD213_WriteCommand(epd, 0x01U); /* Driver output control */
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, 0x27U);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, 0x01U);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, 0x01U);
  if (status != HAL_OK)
  {
    return status;
  }

  status = EPD213_WriteCommand(epd, 0x11U); /* Data entry mode */
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, 0x01U);
  if (status != HAL_OK)
  {
    return status;
  }

  status = EPD213_WriteCommand(epd, 0x44U); /* RAM x start/end */
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, 0x00U);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, 0x0FU); /* (15 + 1) * 8 = 128 */
  if (status != HAL_OK)
  {
    return status;
  }

  status = EPD213_WriteCommand(epd, 0x45U); /* RAM y start/end */
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, 0x27U);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, 0x01U);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, 0x00U);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, 0x00U);
  if (status != HAL_OK)
  {
    return status;
  }

  status = EPD213_WriteCommand(epd, 0x3CU); /* Border waveform */
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, 0x05U);
  if (status != HAL_OK)
  {
    return status;
  }

  status = EPD213_WriteCommand(epd, 0x21U); /* Display update control */
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, 0x00U);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, 0x80U);
  if (status != HAL_OK)
  {
    return status;
  }

  status = EPD213_WriteCommand(epd, 0x18U); /* Internal temperature sensor */
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, 0x80U);
  if (status != HAL_OK)
  {
    return status;
  }

  status = EPD213_SetPos(epd, 0U, 0U);
  if (status != HAL_OK)
  {
    return status;
  }

  /* Power on sequence */
  status = EPD213_WriteCommand(epd, 0x22U);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, 0xF8U);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteCommand(epd, 0x20U);
  if (status != HAL_OK)
  {
    return status;
  }

  return EPD213_WaitBusy(epd, EPD213_BUSY_TIMEOUT_MS);
}

HAL_StatusTypeDef EPD213_Clear(const EPD213_HandleTypeDef *epd)
{
  static uint8_t bw[EPD213_BUF_LEN];
  static uint8_t red[EPD213_BUF_LEN];

  memset(bw, 0xFF, sizeof(bw));
  memset(red, 0xFF, sizeof(red));
  return EPD213_Display(epd, bw, red);
}

HAL_StatusTypeDef EPD213_TestPattern(const EPD213_HandleTypeDef *epd)
{
  static uint8_t bw[EPD213_BUF_LEN];
  static uint8_t red[EPD213_BUF_LEN];
  uint16_t x;
  uint16_t y;

  memset(bw, 0xFF, sizeof(bw));
  memset(red, 0xFF, sizeof(red));

  for (y = 0U; y < EPD213_HEIGHT; y++)
  {
    for (x = 0U; x < EPD213_WIDTH; x++)
    {
      if (x < (EPD213_WIDTH / 3U))
      {
        EPD213_SetPixel(bw, x, y, 1U);  /* black */
      }
      else if (x < ((EPD213_WIDTH * 2U) / 3U))
      {
        EPD213_SetPixel(red, x, y, 1U); /* red */
      }
      else if (((x + y) % 16U) < 8U)
      {
        EPD213_SetPixel(bw, x, y, 1U);  /* checker pattern */
      }
    }
  }

  return EPD213_Display(epd, bw, red);
}

HAL_StatusTypeDef EPD213_ShowText(const EPD213_HandleTypeDef *epd, const char *text, uint16_t x, uint16_t y, uint8_t red_text)
{
  static uint8_t bw[EPD213_BUF_LEN];
  static uint8_t red[EPD213_BUF_LEN];
  uint16_t cursor_x;
  uint16_t cursor_y;
  uint16_t origin_x;
  uint8_t *target;
  char c;
  const uint8_t *glyph;

  if ((epd == NULL) || (text == NULL))
  {
    return HAL_ERROR;
  }

  if ((x >= EPD213_WIDTH) || (y >= EPD213_HEIGHT))
  {
    return HAL_ERROR;
  }

  memset(bw, 0xFF, sizeof(bw));
  memset(red, 0xFF, sizeof(red));

  target = (red_text != 0U) ? red : bw;
  cursor_x = x;
  cursor_y = y;
  origin_x = x;

  while (*text != '\0')
  {
    c = *text++;
    if (c == '\n')
    {
      cursor_x = origin_x;
      cursor_y = (uint16_t)(cursor_y + 8U);
      if ((cursor_y + 7U) >= EPD213_HEIGHT)
      {
        break;
      }
      continue;
    }

    if ((c >= 'a') && (c <= 'z'))
    {
      c = (char)(c - ('a' - 'A'));
    }

    if ((cursor_x + 5U) >= EPD213_WIDTH)
    {
      cursor_x = origin_x;
      cursor_y = (uint16_t)(cursor_y + 8U);
      if ((cursor_y + 7U) >= EPD213_HEIGHT)
      {
        break;
      }
    }

    glyph = EPD213_FindGlyph(c);
    EPD213_DrawChar5x7(target, cursor_x, cursor_y, glyph);
    cursor_x = (uint16_t)(cursor_x + 6U);
  }

  return EPD213_Display(epd, bw, red);
}

HAL_StatusTypeDef EPD213_Sleep(const EPD213_HandleTypeDef *epd)
{
  HAL_StatusTypeDef status;

  status = EPD213_WriteCommand(epd, 0x22U);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, 0x83U);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteCommand(epd, 0x20U);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WaitBusy(epd, EPD213_BUSY_TIMEOUT_MS);
  if (status != HAL_OK)
  {
    return status;
  }

  status = EPD213_WriteCommand(epd, 0x10U);
  if (status != HAL_OK)
  {
    return status;
  }
  return EPD213_WriteData(epd, 0x01U);
}

static HAL_StatusTypeDef EPD213_Display(const EPD213_HandleTypeDef *epd, const uint8_t *bw, const uint8_t *red)
{
  HAL_StatusTypeDef status;
  uint32_t i;
  static uint8_t red_inv[EPD213_BUF_LEN];

  if ((epd == NULL) || (bw == NULL) || (red == NULL))
  {
    return HAL_ERROR;
  }

  status = EPD213_SetPos(epd, 0U, 0U);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteCommand(epd, 0x24U);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteDataBuffer(epd, bw, EPD213_BUF_LEN);
  if (status != HAL_OK)
  {
    return status;
  }

  status = EPD213_SetPos(epd, 0U, 0U);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteCommand(epd, 0x26U);
  if (status != HAL_OK)
  {
    return status;
  }

  for (i = 0U; i < EPD213_BUF_LEN; i++)
  {
    red_inv[i] = (uint8_t)(~red[i]);
  }
  status = EPD213_WriteDataBuffer(epd, red_inv, EPD213_BUF_LEN);
  if (status != HAL_OK)
  {
    return status;
  }

  return EPD213_Update(epd);
}

static const uint8_t *EPD213_FindGlyph(char c)
{
  uint32_t i;
  uint32_t count;

  count = (uint32_t)(sizeof(k_epd213_font5x7) / sizeof(k_epd213_font5x7[0]));
  for (i = 0U; i < count; i++)
  {
    if (k_epd213_font5x7[i].ch == c)
    {
      return k_epd213_font5x7[i].col;
    }
  }
  return k_epd213_glyph_question;
}

static void EPD213_DrawChar5x7(uint8_t *buf, uint16_t x, uint16_t y, const uint8_t glyph[5])
{
  uint16_t col;
  uint16_t row;
  uint8_t bits;

  if (buf == NULL)
  {
    return;
  }

  for (col = 0U; col < 5U; col++)
  {
    bits = glyph[col];
    for (row = 0U; row < 7U; row++)
    {
      if ((bits & (1U << row)) != 0U)
      {
        EPD213_SetPixel(buf, (uint16_t)(x + col), (uint16_t)(y + row), 1U);
      }
    }
  }
}

static HAL_StatusTypeDef EPD213_Update(const EPD213_HandleTypeDef *epd)
{
  HAL_StatusTypeDef status;

  status = EPD213_WriteCommand(epd, 0x22U);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, 0xF7U);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteCommand(epd, 0x20U);
  if (status != HAL_OK)
  {
    return status;
  }
  return EPD213_WaitBusy(epd, EPD213_BUSY_TIMEOUT_MS);
}

static HAL_StatusTypeDef EPD213_SetPos(const EPD213_HandleTypeDef *epd, uint16_t x, uint16_t y)
{
  HAL_StatusTypeDef status;
  uint8_t x_addr;
  uint16_t y_addr;

  x_addr = (uint8_t)(x / 8U);
  y_addr = (uint16_t)(EPD213_RAM_Y_END - y);

  status = EPD213_WriteCommand(epd, 0x4EU);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, x_addr);
  if (status != HAL_OK)
  {
    return status;
  }

  status = EPD213_WriteCommand(epd, 0x4FU);
  if (status != HAL_OK)
  {
    return status;
  }
  status = EPD213_WriteData(epd, (uint8_t)(y_addr & 0xFFU));
  if (status != HAL_OK)
  {
    return status;
  }
  return EPD213_WriteData(epd, (uint8_t)((y_addr >> 8U) & 0x01U));
}

static void EPD213_Reset(const EPD213_HandleTypeDef *epd)
{
  HAL_GPIO_WritePin(epd->res_port, epd->res_pin, GPIO_PIN_RESET);
  HAL_Delay(50U);
  HAL_GPIO_WritePin(epd->res_port, epd->res_pin, GPIO_PIN_SET);
  HAL_Delay(50U);
}

static HAL_StatusTypeDef EPD213_WaitBusy(const EPD213_HandleTypeDef *epd, uint32_t timeout_ms)
{
  uint32_t tick_start;

  tick_start = HAL_GetTick();
  while (EPD213_IsBusy(epd) != 0U)
  {
    if ((HAL_GetTick() - tick_start) >= timeout_ms)
    {
      return HAL_TIMEOUT;
    }
    HAL_Delay(1U);
  }
  return HAL_OK;
}

static HAL_StatusTypeDef EPD213_WriteCommand(const EPD213_HandleTypeDef *epd, uint8_t cmd)
{
  HAL_StatusTypeDef status;

  HAL_GPIO_WritePin(epd->dc_port, epd->dc_pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(epd->cs_port, epd->cs_pin, GPIO_PIN_RESET);
  status = HAL_SPI_Transmit(epd->hspi, &cmd, 1U, EPD213_SPI_TIMEOUT_MS);
  if (status != HAL_OK)
  {
    HAL_GPIO_WritePin(epd->cs_port, epd->cs_pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(epd->dc_port, epd->dc_pin, GPIO_PIN_SET);
    return status;
  }
  HAL_GPIO_WritePin(epd->cs_port, epd->cs_pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(epd->dc_port, epd->dc_pin, GPIO_PIN_SET);
  return HAL_OK;
}

static HAL_StatusTypeDef EPD213_WriteData(const EPD213_HandleTypeDef *epd, uint8_t data)
{
  HAL_StatusTypeDef status;

  HAL_GPIO_WritePin(epd->dc_port, epd->dc_pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(epd->cs_port, epd->cs_pin, GPIO_PIN_RESET);
  status = HAL_SPI_Transmit(epd->hspi, &data, 1U, EPD213_SPI_TIMEOUT_MS);
  if (status != HAL_OK)
  {
    HAL_GPIO_WritePin(epd->cs_port, epd->cs_pin, GPIO_PIN_SET);
    return status;
  }
  HAL_GPIO_WritePin(epd->cs_port, epd->cs_pin, GPIO_PIN_SET);
  return HAL_OK;
}

static HAL_StatusTypeDef EPD213_WriteDataBuffer(const EPD213_HandleTypeDef *epd, const uint8_t *buf, uint32_t len)
{
  HAL_StatusTypeDef status;

  HAL_GPIO_WritePin(epd->dc_port, epd->dc_pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(epd->cs_port, epd->cs_pin, GPIO_PIN_RESET);
  status = HAL_SPI_Transmit(epd->hspi, (uint8_t *)buf, len, EPD213_SPI_TIMEOUT_MS);
  if (status != HAL_OK)
  {
    HAL_GPIO_WritePin(epd->cs_port, epd->cs_pin, GPIO_PIN_SET);
    return status;
  }
  HAL_GPIO_WritePin(epd->cs_port, epd->cs_pin, GPIO_PIN_SET);
  return HAL_OK;
}

static void EPD213_SetPixel(uint8_t *buf, uint16_t x, uint16_t y, uint8_t colored)
{
  uint32_t index;
  uint8_t bit_mask;

  if ((x >= EPD213_WIDTH) || (y >= EPD213_HEIGHT))
  {
    return;
  }

  index = ((uint32_t)y * EPD213_WIDTH_BYTES) + (x / 8U);
  bit_mask = (uint8_t)(0x80U >> (x % 8U));

  if (colored != 0U)
  {
    buf[index] &= (uint8_t)(~bit_mask);
  }
  else
  {
    buf[index] |= bit_mask;
  }
}
