objdump 可以印出每個符號名稱，但我的 elfdump 只印出了符號表字串 (strtab) 中的名稱，卻沒有印 (shstrtab) 的。

似乎和 info 欄位有關

* st_info

  This member specifies the symbol's type and binding attributes.  A
  list of the values and meanings appears below. The following code
  shows how to manipulate the values.

    #define ELF32_ST_BIND(i)	((i)>>4)
    #define ELF32_ST_TYPE(i)	((i)&0xf)
    #define ELF32_ST_INFO(b, t)	(((b)<<4)+((t)&0xf))

A symbol's binding determines the linkage visibility and behavior.

+ Figure 1-17: Symbol Binding, ELF32_ST_BIND

  Name        Value
  ====        =====
  STB_LOCAL       0
  STB_GLOBAL      1
  STB_WEAK        2
  STB_LOPROC     13
  STB_HIPROC     15

* STB_LOCAL

  Local symbols are not visible outside the object file containing
  their definition. Local symbols of the same name may exist in
  multiple files without interfering with each other.

* STB_GLOBAL

  Global symbols are visible to all object files being combined. One
  file's definition of a global symbol will satisfy another file's
  undefined reference to the same global symbol.

* STB_WEAK

  Weak symbols resemble global symbols, but their definitions have
  lower precedence.

* STB_LOPROC through STB_HIPROC

  Values in this inclusive range are reserved for processor-specific
  semantics.

A symbol's type provides a general classification for the associated
entity.

+ Figure 1-18: Symbol Types, ELF32_ST_TYPE

  Name         Value
  ====         =====
  STT_NOTYPE       0
  STT_OBJECT       1
  STT_FUNC         2
  STT_SECTION      3 // info&0b1111 = 3 代表段落
  STT_FILE         4
  STT_LOPROC      13
  STT_HIPROC      15

* STT_NOTYPE

  The symbol's type is not specified.

* STT_OBJECT

  The symbol is associated with a data object, such as a variable, an
  array, etc.

* STT_FUNC

  The symbol is associated with a function or other executable code.

* STT_SECTION

  The symbol is associated with a section. Symbol table entries of
  this type exist primarily for relocation and normally have STB_LOCAL
  binding.

* STT_FILE

  Conventionally, the symbol's name gives the name of the source file
  associated with the object file. A file symbol has STB_LOCAL
  binding, its section index is SHN_ABS, and it precedes the other
  STB_LOCAL symbols for the file, if it is present.

* STT_LOPROC through STT_HIPROC

  Values in this inclusive range are reserved for processor-specific
  semantics.



```
.symtab
name: value:0x0 size:0 info:0 other:0 shndx:0x0
name: value:0x80000000 size:0 info:3 other:0 shndx:0x1  // info:3 代表段落 STT_SECTION
name: value:0x800080e4 size:0 info:3 other:0 shndx:0x2  
name: value:0x800080ef size:0 info:3 other:0 shndx:0x3  
name: value:0x0 size:0 info:3 other:0 shndx:0x4
name: value:0x0 size:0 info:3 other:0 shndx:0x5
name:C:\Users\user\AppData\Local\Temp\ccNpw8Ob.o value:0x0 size:0 info:4 other:0 shndx:0xfff1
name:STACK_SIZE value:0x2000 size:0 info:0 other:0 shndx:0xfff1 // info:0 STT_NOTYPE
name:stacks value:0x80000028 size:0 info:0 other:0 shndx:0x1
name:park value:0x80000020 size:0 info:0 other:0 shndx:0x1
name:os.c value:0x0 size:0 info:4 other:0 shndx:0xfff1  // info:4 代表 STT_FILE
name:lib_putc value:0x80008028 size:76 info:18 other:0 shndx:0x1 // info:2 代表 STT_FUNC, bind:1 代表 GLOBAL
name:lib_puts value:0x80008074 size:80 info:18 other:0 shndx:0x1, // bind:1 代表 GLOBAL
name:os_main value:0x800080c4 size:32 info:18 other:0 shndx:0x1
name:_start value:0x80000000 size:0 info:16 other:0 shndx:0x1
```

objdump

```
$ riscv64-unknown-elf-objdump -t ../obj/oshello.elf

../obj/oshello.elf:     file format elf32-littleriscv

SYMBOL TABLE:
80000000 l    d  .text  00000000 .text
800080e4 l    d  .rodata        00000000 .rodata
800080ef l    d  .data  00000000 .data
00000000 l    d  .riscv.attributes      00000000 .riscv.attributes
00000000 l    d  .comment       00000000 .comment
00000000 l    df *ABS*  00000000 C:\Users\user\AppData\Local\Temp\ccNpw8Ob.o
00002000 l       *ABS*  00000000 STACK_SIZE
80000028 l       .text  00000000 stacks
80000020 l       .text  00000000 park
00000000 l    df *ABS*  00000000 os.c
80008028 g     F .text  0000004c lib_putc
80008074 g     F .text  00000050 lib_puts
800080c4 g     F .text  00000020 os_main
80000000 g       .text  00000000 _start
```