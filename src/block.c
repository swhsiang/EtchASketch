#include "block.h"

#define max(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b;      \
  })

#define min(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a < _b ? _a : _b;      \
  })


Block *block_init(uint16_t block_x, uint16_t block_y, uint16_t thick_x, uint16_t thick_y,
                  uint16_t color) {
  assert(block_x >= 0);
  assert(block_x + thick_x < X_AVA_MAX);
  assert(block_y >= 0);
  assert(block_y + thick_y < Y_AVA_MAX);

  Block *block = malloc(sizeof(Block));
  block->block_x = block_x;
  block->block_y = block_y;
  block->color = color;
  block->thick_x = thick_x;
  block->thick_y = thick_y;

  return block;
}

void show_block_valid(Block *block) {
	if ((block->block_x + block->thick_x > 240 - 1) || (block->block_y + block->thick_y > 320 - 1)) {

		return;
	}
  // Set column range.
  hspi_cmd(SPI1, 0x2A);
  hspi_w16(SPI1, (uint16_t) min(block->block_x, 240 - block->thick_x ));
  hspi_w16(SPI1, (uint16_t) min(block->block_x + block->thick_x - 1, 240 - 1) );

  // Set row range.
  hspi_cmd(SPI1, 0x2B);
  hspi_w16(SPI1, (uint16_t) min(block->block_y, 320 - block->thick_y)  );
  hspi_w16(SPI1, (uint16_t) min(block->block_y + block->thick_y - 1, 320  - 1) );
  // Set 'write to RAM'
  hspi_cmd(SPI1, 0x2C);

  uint16_t i = 0;
  for (i = 0; i < (block->thick_x * block->thick_y); i++) {
    hspi_w16(SPI1, block->color);
  }
}

void show_block(Block *block) {
  show_block_valid(block);
}
