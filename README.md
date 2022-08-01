# tiny-crc32

tiny (but slow) CRC32 function in only 32 bytes written in x86-64 assembly

## source code

see `crc32_asm()` in `crc32.c`

## limitations

* very slow (about 100 times slower than zlib's CRC32 implementation)
* only available in System V x86-64 ABI
* not portable at all (although a equivalent C implementation is provided)

## compile & test

```
gcc -Os -Wall -o test test.c crc32.c -lz
./test
```

## objdump

```
0000000000000000 <crc32_asm>:
   0:	31 c0                	xor    %eax,%eax
   2:	48 83 ee 01          	sub    $0x1,%rsi
   6:	72 17                	jb     1f <crc32_asm+0x1f>
   8:	32 07                	xor    (%rdi),%al
   a:	48 ff c7             	inc    %rdi
   d:	31 c9                	xor    %ecx,%ecx
   f:	b1 08                	mov    $0x8,%cl
  11:	f9                   	stc    
  12:	d1 d8                	rcr    %eax
  14:	72 05                	jb     1b <crc32_asm+0x1b>
  16:	35 20 83 b8 ed       	xor    $0xedb88320,%eax
  1b:	e2 f4                	loop   11 <crc32_asm+0x11>
  1d:	eb e3                	jmp    2 <crc32_asm+0x2>
  1f:	c3                   	retq   
```

## license

the unlicense
