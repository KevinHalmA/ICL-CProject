b start
nop
nop
nop

request_buffer:
    .int 0
    .int 0
    .int 0
    .int 0
    .int 0
    .int 0
    .int 0
    .int 0

on_buffer:
    .int 32
    .int 0
    .int 0x00038041
    .int 8
    .int 0x0
    .int 130
    .int 0x1
    .int 0x0

off_buffer:
    .int 32
    .int 0
    .int 0x00038041
    .int 8
    .int 0x0
    .int 130
    .int 0x0
    .int 0x0

start:
    movz w0 #0x0
    ldr w1 request_addr
    ldr w2 on_addr
    ldr w3 off_addr
    ldr w4 write_reg
    ldr w5 write_status
    ldr w6 read_reg
    ldr w7 read_status

loop:
    cmp w0, #0x0
    b.eq set_on
    b set_off

set_on:
    mov w8, w2
    movz x0, #0x1
    b main

set_off:
    mov w8, w3
    movz x0, #0x0
    b main

main:
    wait_write:
        ldr w9, [w5]
        movk w10, #0x8000, lsl #16
        tst w9, w10
        b.ne wait_write

    movz w11, #32
    mov w12, w8
    mov w13, w1
    copy_loop:
        ldr w14, [w12], #4
        str w14, [w13], #4
        subs w11, w11, #4
        b.ne copy_loop

    add w15, w1, #0x8
    str w15, [w4]

    wait_read:
        ldr w9, [w7]
        movk w10, #0x8000, lsl #16
        tst w9, w10
        b.ne wait_read

    ldr w16, [w6]

    ldr w17, delay_time
    delay:
        subs w17, w17, #1
        b.ne delay

    b loop

request_addr:
    .int 0x80010

on_addr:
    .int 0x80030

off_addr:
    .int 0x80050

write_reg:
    .int 0x3f00b8a0

write_status:
    .int 0x3f00b8b8

read_reg:
    .int 0x3f00b880

read_status:
    .int 0x3f00b898

delay_time:
    .int 0x00f0000