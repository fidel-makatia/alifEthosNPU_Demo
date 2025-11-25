/**
 * Linker Script for Alif Ensemble E8
 * Cortex-M55 + Ethos-U55
 * 
 * Memory Map:
 *   MRAM (Flash): 0x80000000 - 0x803FFFFF (4MB)
 *   SRAM0:        0x20000000 - 0x2003FFFF (256KB)
 *   SRAM1:        0x20040000 - 0x2007FFFF (256KB)
 */

ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x80000000, LENGTH = 1M
    SRAM0 (rwx) : ORIGIN = 0x20000000, LENGTH = 256K
    SRAM1 (rwx) : ORIGIN = 0x20040000, LENGTH = 256K
}

_Min_Heap_Size = 0x4000;
_Min_Stack_Size = 0x4000;

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } >FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
        _etext = .;
    } >FLASH

    .rodata :
    {
        . = ALIGN(4);
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } >FLASH

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } >FLASH

    .ARM :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } >FLASH

    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } >SRAM0 AT> FLASH

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = _ebss;
    } >SRAM0

    /* RTT Control Block - must be in RAM for J-Link to find it */
    .rtt_cb :
    {
        . = ALIGN(4);
        *(.rtt_cb)
        . = ALIGN(4);
    } >SRAM0

    .tensor_arena (NOLOAD) :
    {
        . = ALIGN(16);
        _tensor_arena_start = .;
        . = . + 128K;
        _tensor_arena_end = .;
    } >SRAM1

    ._user_heap (NOLOAD) :
    {
        . = ALIGN(8);
        PROVIDE(end = .);
        PROVIDE(_end = .);
        . = . + _Min_Heap_Size;
        . = ALIGN(8);
    } >SRAM0

    ._user_stack (NOLOAD) :
    {
        . = ALIGN(8);
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
        _estack = .;
    } >SRAM0

    /DISCARD/ :
    {
        libc.a(*)
        libm.a(*)
        libgcc.a(*)
    }

    .ARM.attributes 0 : { *(.ARM.attributes) }
}
