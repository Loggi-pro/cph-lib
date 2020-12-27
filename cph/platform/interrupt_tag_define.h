//https://www.avrfreaks.net/forum/static-library-interrupt-routine
//Problem: ISR from static library not link with sources, becouse they both - weak symbols
//Solution: create tag (variable) inside ISR file, and provide -Wl,-u,tag_<name> to link options
//In this case, you could use the PROVIDE_ENTRY macro inside the ISR like this:
//ISR(INT0_vect) {
//  PROVIDE_ENTRY(INT0); //create tag=tag_INT0
//  PORTB = 0xAA;
//} 
#pragma once
#if defined(__ASSEMBLER__)
    #define PROVIDE_SYM(name)   GLOBAL_SYM name
    #define REQUEST_SYM(name)   .global name
    .macro GLOBAL_SYM _name
        .global \_name
    \_name:
    .endm
#else
    #define PROVIDE_SYM(name)   __asm__ __volatile__(".global " #name "\n" #name ":\n"::)
    #define REQUEST_SYM(name)   __asm__ __volatile__(".global " #name "\n"::)
#endif
#define PROVIDE_ENTRY(name)     PROVIDE_SYM(tag_##name)
#define REQUEST_ENTRY(name)     REQUEST_SYM(tag_##name)