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

void show_block_x_y_overflow(Block *block) {
  uint16_t i = 0, height = 0, width = 0;

  // original place
  // Set column range.
  width = X_AVA_MAX - block->block_x + 1;
  hspi_cmd(SPI1, 0x2A);
  hspi_w16(SPI1, (uint16_t)(block->block_x));
  hspi_w16(SPI1, (uint16_t)(X_AVA_MAX - 1));

  // Set row range.
  height = Y_AVA_MAX - block->block_y + 1;
  hspi_cmd(SPI1, 0x2B);
  hspi_w16(SPI1, (uint16_t)(block->block_y));
  hspi_w16(SPI1, (uint16_t)(Y_AVA_MAX - 1));

  // Set 'write to RAM'
  hspi_cmd(SPI1, 0x2C);

  for (i = 0; i < (width * height); i++) {
    hspi_w16(SPI1, block->color);
  }
}

void show_block_y_overflow(Block *block) {
  uint16_t i = 0, height = 0;

  // original place
  // Set column range.
  hspi_cmd(SPI1, 0x2A);
  hspi_w16(SPI1, (uint16_t)(block->block_x));
  hspi_w16(SPI1, (uint16_t)(block->block_x + block->thick_x - 1) % COLUMN_NUM);

  // Set row range.
  height = Y_AVA_MAX - block->block_y + 1;
  hspi_cmd(SPI1, 0x2B);
  hspi_w16(SPI1, (uint16_t)(block->block_y));
  hspi_w16(SPI1, (uint16_t)(Y_AVA_MAX - 1));

  // Set 'write to RAM'
  hspi_cmd(SPI1, 0x2C);

  for (i = 0; i < (block->thick_x * (height)); i++) {
    hspi_w16(SPI1, block->color);
  }

}

// block x, y overflow
// block x overflow or y overflow
// block ok

void show_block_valid(Block *block) {
  // Set column range.
  hspi_cmd(SPI1, 0x2A);
  hspi_w16(SPI1, (uint16_t)(block->block_x));
  hspi_w16(SPI1, (uint16_t)(block->block_x + block->thick_x - 1));

  // Set row range.
  hspi_cmd(SPI1, 0x2B);
  hspi_w16(SPI1, (uint16_t)(block->block_y));
  hspi_w16(SPI1, (uint16_t)(block->block_y + block->thick_y - 1));
  // Set 'write to RAM'
  hspi_cmd(SPI1, 0x2C);

  uint16_t i = 0;
  for (i = 0; i < (block->thick_x * block->thick_y); i++) {
    hspi_w16(SPI1, block->color);
  }
}

void show_block_x_overflow(Block *block) {
  uint16_t i = 0, width = 0;

  // original place
  // Set column range.
  hspi_cmd(SPI1, 0x2A);
  hspi_w16(SPI1, (uint16_t)(block->block_x));
  hspi_w16(SPI1, (uint16_t)(X_AVA_MAX - 1));
  width = X_AVA_MAX - block->block_x + 1;

  // Set row range.
  hspi_cmd(SPI1, 0x2B);
  hspi_w16(SPI1, (uint16_t)(block->block_y));
  hspi_w16(SPI1, (uint16_t)block->block_y + block->thick_y - 1);
  // Set 'write to RAM'
  hspi_cmd(SPI1, 0x2C);

  for (i = 0; i < (block->thick_y * (width)); i++) {
    hspi_w16(SPI1, block->color);
  }
}

// FIXME should stop drawing pixels which is exit the screen
void show_block(Block *block) {
  int16_t x = (block->block_x + block->thick_x) >= X_AVA_MAX;
  int16_t y = (block->block_y + block->thick_y) >= Y_AVA_MAX;

  if (x != 0 && y != 0) {
    show_block_x_y_overflow(block);
  } else if (x != 0 && y == 0) {
    show_block_x_overflow(block);
  } else if (x == 0 && y != 0) {
    show_block_y_overflow(block);
  } else {
    show_block_valid(block);
  }
}

void block_up_down(Block *block, uint16_t dist, uint8_t direction) {
  // assert(dist < Y_AVA_MAX);
  // assert(direction == DOWN || direction == UP);
	// FIXME should move the block pixel by pixel
  if (direction == DOWN) {
	  block->block_y = min(block->block_y - dist, Y_AVA_MAX - 1);
  } else {
    block->block_y = (block->block_y + dist) % Y_AVA_MAX;
  }
  show_block(block);
}

void block_left_right(Block *block, uint16_t dist, uint8_t direction) {
  // assert(dist < X_AVA_MAX);
  // assert(direction == RIGHT || direction == LEFT);
  if (direction == RIGHT) {
    block->block_x = min(block->block_x - dist, X_AVA_MAX - 1);
  } else {
    block->block_x = (block->block_x + dist) % X_AVA_MAX;
  }
  show_block(block);
}
