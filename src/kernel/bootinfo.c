#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "bootloader.h"

e820_entry_t mmap_entries[128]; // Assuming a maximum of 128 entries, adjust as needed

void dump_mmap(const struct bootinfo* info) {
    printf_serial("Memory Map:\n");
    for (uint32_t i = 0; i < info->mmap.mmap_count; i++) {
        e820_entry_t* entry = &info->mmap.mmap[i];
        printf_serial("Entry %u: Base=%p, Length=%#x, Type=%u\n",
                      i, entry->base_addr, entry->length, entry->type);
    }
}

void copy_bootinfo(struct bootinfo* dest, const struct bootinfo* src) {
    (void)src; // silence unused parameter warning if we don't use src for something
    dest->mmap.mmap_count = *(uint32_t*)(MMAP_COUNT_ADDR);
    dest->mmap.mmap = mmap_entries; // Point to the local array for mmap entries
    memcpy(mmap_entries, (void*)(MMAP_ENTRIES_ADDR), dest->mmap.mmap_count * sizeof(e820_entry_t)); // Copy mmap entries from the source address
    // dump_mmap(dest); // Dump the memory map for debugging
    dest->boot_drive.drive = *(uint8_t*)(BOOT_DRIVE_ADDR); // Copy the boot drive information
    dest->boot_drive.fs = FS_FAT16; // Assuming FAT16 for now, adjust as needed
    dest->boot_drive.kernel_partition_lba = *(uint32_t*)(KERN_LBA_ADDR); // Copy the kernel partition LBA
    // printf_serial("Kernel partition LBA: %u\n", dest->boot_drive.kernel_partition_lba);
    memcpy(&dest->video, (void*)(VIDEO_INFO_ADDR), sizeof(struct videoInfo)); // Copy video information
    // since we're converting from a 32 bit pointer to a 64 bit pointer we need to do some casting
    dest->video.framebuffer = (uint32_t*)(uintptr_t)(*((uint32_t*)(VIDEO_INFO_ADDR+0x1C))); // Copy the framebuffer address, ensuring proper pointer size
    dest->video.screen_size = dest->video.height * dest->video.pitch; // Calculate the screen size in bytes
    dest->video.screen_size_dqwords = (dest->video.screen_size + 7) / 8; // Calculate the screen size in double quadwords (64-bit units)
    // I don't need elf info, so we'll leave it zeroed out
    memset(&dest->elf, 0, sizeof(struct elf_info));
    // printf_serial("framebuffer: %p\n", dest->video.framebuffer);
}