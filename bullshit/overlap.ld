ENTRY(_start)

PHDRS
{
    text  PT_LOAD FLAGS(5);
    ro    PT_LOAD FLAGS(4);
    alias PT_LOAD FLAGS(4);
}

SECTIONS
{
    . = 0x400000 + SIZEOF_HEADERS;

    .text : {
        *(.text)
    } :text

    RO = ALIGN(ADDR(.text) + SIZEOF(.text), 0x1000);

    .rodata RO : {
        *(.rodata)
    } :ro

    .alias RO : {
        *(.alias)
    } :alias
}
