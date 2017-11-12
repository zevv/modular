#ifndef os_flash_h
#define os_flash_h

void flash_init(void);
void flash_read(uint32_t addr, void *buf, size_t len);
void flash_write(uint32_t addr, const void *buf, size_t len);
void flash_load(uint32_t addr, const void *buf, size_t len);
void flash_write_buf1(uint32_t addr);

#endif

