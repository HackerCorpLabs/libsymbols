stabs "hello.c",100,0,0,LL10
LL10:
    .stabs "int:t1=r1;-32768;32767;",128,0,0,0
    .stabs "char:t2=r2;0;127;",128,0,0,0
    .stabs "short:t3=r1;-32768;32767;",128,0,0,0
    .stabs "long:t4=r1;2147483648;2147483647;",128,0,0,0
    .stabs "long long:t5=r1;-9223372036854775808;9223372036854775807;",128,0,0,0
    .stabs "unsigned char:t6=r1;0;255;",128,0,0,0
    .stabs "unsigned short:t7=r1;0;65535;",128,0,0,0
    .stabs "unsigned int:t8=r1;0;65535;",128,0,0,0
    .stabs "unsigned long:t9=r1;0;4294967295;",128,0,0,0
    .stabs "unsigned long long:t10=r1;0;-1;",128,0,0,0
    .stabs "float:t11=r1;4;0;",128,0,0,0
    .stabs "double:t12=r1;8;0;",128,0,0,0
    .stabs "long double:t13=r1;12;0;",128,0,0,0
    .stabs "void:t14=r14",128,0,0,0
    .text
    .globl _main
_main:
    copy sl da
    jpl i [.word csav]
    .word 02
    .stabs "_main:F1",36,0,0,_main

    .stabn 68,0,2,LL11-_main
LL11:

L249:
L253:
    .stabn 68,0,3,LL12-_main
LL12:

    ldt [.word L257]
    rclr dx
    stt ,b -1
    stx ,b -2
    jpl i [.word _printf]
    .stabn 68,0,4,LL13-_main
LL13:

L251:
    jmp i [.word cret]
    .data
L257:
    .ascii "Hello World\012\0"
    .stabs "",100,0,0,LL14
LL14: