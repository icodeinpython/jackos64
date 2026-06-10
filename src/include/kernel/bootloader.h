#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <stdint.h>

#define MAGIC 0xB19B00B5
#define VIDEO_INFO_ADDR 0x1000
#define BOOTINFO_ADDR 0x1100
#define MMAP_COUNT_ADDR 0x2000 // uint32_t
#define MMAP_ENTRIES_ADDR 0x2010 // 16 byte aligned
#define KERN_LBA_ADDR 0x2004 // uint32_t
#define BOOT_DRIVE_ADDR 0x2008 // uint8_t



// provided information: 
// EAX: magic
// EBX: pointer to bootinfo

enum FS {
    FS_FAT12,
    FS_FAT16,
    FS_FAT32,
    FS_EXT2,
    FS_EXT3,
    FS_EXT4
};

typedef struct {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t acpi_ext;

    uint8_t padding[8]; // pad to 32 bytes total
} __attribute__((packed)) e820_entry_t;

struct drive_info {
    uint8_t drive;
    uint32_t kernel_partition_lba;
    enum FS fs;
};

struct videoInfo {
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t bytes_per_pixel;
    uint32_t pitch;
    uint32_t screen_size;
    uint32_t screen_size_dqwords;
    uint32_t* framebuffer;
};

struct elf_info {
    uint32_t entry;
    uint32_t kernel_load_addr;
    uint32_t kernel_size;
};

struct mmap {
    e820_entry_t* mmap;
    uint32_t mmap_count;
};

struct bootinfo {
    uint32_t magic;
    struct drive_info boot_drive;
    struct videoInfo video;
    struct elf_info elf;
    struct mmap mmap;
};

void copy_bootinfo(struct bootinfo* dest, const struct bootinfo* src);

#endif