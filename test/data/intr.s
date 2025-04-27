start:   
        jmp after_text

welcome_strp:   .word welcome_str
welcome_str:    .asciz "Welcome to the clock test!\r\n"

s_done_strp:   .word s_done_str
s_done_str:    .asciz "Clock ticked 1 minute\r\n"

rtc_strp:   .word rtc_str
rtc_str:    .asciz "RTC! \r\n"

clock_set_strp:   .word clock_set_str
clock_set_str:    .asciz "Clock set to 0:0:0\r\n"


#################### Function PrintString #####################
# input: ldt pointer to string

print_exit:   .word 0
printstr: 

        swap sl dt 
        stt print_exit
        swap sl dt 

        rclr dx
loop:   lbyt
        skp da ueq 0
        jmp printexit
        jpl putch
        rinc dx
        jmp loop

printexit: 
        # restore link register
        ldt print_exit
        swap sl dt # restore link register              
        exit
        

putch:
        copy sa dd        # Save A
        iox 0306          # Get status
        bskp one 030 da   # Is it ready to transfer?
        jmp .-2         # nope, wait
        copy sd da        # Restore A
        iox 0305
        exit

#################### END PrintString #####################

# TEXT




after_text:

        # Say welcome 
        ldt welcome_strp        
        jpl printstr        


        #### INITALIZE RTC ####

        # clear the second counter
        saa 0       
        sta sec
        
        # Set up the RTC
        saa 1
        iox 013 # Enable interrupts

        # read RTC status register
        iox 012

        #clear readyfortransger
        iox 010 

        ldt clock_set_strp 
        jpl printstr

        # Set up the RTC interrupt handler        
        lda lev13_ptr  
        .word 0153552         # IRW 150 DP        


        #### INITALIZE INTERRUPT SUBSYSTEM  ####

        .word 0150005 # TRA IIC (Unlock IIC)
        .word 0150015 # TRA PES (Unlock PES and PEA)

        # enable interrupts
        lda int_levels

        # TRR PIE (Priority Interrupt Enable)
        .word 0150107         # TRR PIE

        # TRR IIE (Internal Interrupt Interrupt Enable) - for jmp to level 14
        lda iie_sources
        .word 0150105         # TRR IIE
      

        # Set up program counter (P) for level 14 (internal interrupt handler)
        lda lev14_ptr  
        .word 0153562         # IRW 160 DP        

        ion # turn on interrupts
        
main_loop:
        lda minute
        skp da eql 0  # loop until one sec has been ticked
        jmp done       
        jmp main_loop

done:
        iof

        ldt s_done_strp 
        jpl printstr

        
        wait


        
int_levels:  .word 0b0111110000011010  # levels 1, 3, 4, 10, 11, 12, 13 and 14
iie_sources: .word 0b011111011110 # all, except Z flag


# ################### LEVEL 13 #####################
lev13_ptr:        .word lev13
lev13:    
        

          # filter interrupts ID to 2 bits
        ident pl13
        sat 7       
        rand st da

        # Add identy to P
        radd sa dp

        # ID = 0, error
        jmp err13 # ID = 0
        jmp rtc01 # ID = 1
        jmp rtc_noop # ID = 2
        jmp rtc_noop # ID = 3
        jmp rtc_noop # ID = 4
        jmp rtc_noop # ID = 5
        jmp rtc_noop # ID = 6
        jmp rtc_noop # ID = 7
        

rtc01:
        # ldt rtc_strp 
        # jpl printstr

        // RTC 1

        # Increase tick count
        lda tick
        aaa 1
        sta tick

        ldt ticks_pr_sec
        
        # skp dt lss sa (if tick >= ticks_pr_sec) update
        .word 0142456
        # if tick is greater than 50, skip, else increase
        jmp rtc01_noinc
        
        # Increment second
        stz tick # clear tick counter

        lda sec
        aaa 1
        sta sec

        sat 60
        # skp da gre st
        .word 0141065 # (if sec >= 60) skip, else exit
        # if sec is greater than 60, skip, else increase
        jmp rtc01_noinc
        # else, increment minute
        stz sec # clear sec counter
        lda minute
        aaa 1
        sta minute


rtc01_noinc:
        # reenable rtc interrupts
        saa 1
        iox 013
        # reset counter
        iox 010

        jmp cont13

rtc_noop: # No operation on these clocks
        // RTC 2-7
        jmp cont13

err13:
        # something odd happened
        # Unable to identify device at level 13

cont13:
        wait # Relinquish priority
        jmp lev13


# LEVEL 13 DATA
tick:  .word 00
ticks_pr_sec: .word 062 # =decimal 50

sec: .word 00
minute: .word 00
hour: .word 00

#################### END OF LEVEL 13 #####################

#################### LEVEL 14 #####################
lev14_ptr:        .word lev14
lev14:    
        
        .word 0150005  # tra iic
        radd sa dp # computed GO TO - add A reg to P reg

        jmp error
        radd sa dp 

	#  MC  |   1      |    1     | Monitor Call
	#  PV  |   2      |    2     | Protect Violation. Page number is found in the Paging Status Register.
	#  PF  |   3      |    3     | Page fault. Page not in memory.
	#  II  |   4      |    4     | lllegal instruction. Not implemented instruction.	
	#  Z   |   5      |    5     | Error indicator. The Z indicator is set.
	#  PI  |   6      |    6     | Privileged instruction. 
	#  IOX |   7      |    7     | IOX error. No answer from external device.
	#  PTY |   8      |    10    | Memory parity error
	#  MOR |   9      |    11    | Memory out of range Addressing non-existent memory.
	#  POW |   10     |    12    | Power fail interrupt

        jmp  error_14 # 0 error not assigned 
        jmp  mc_14  # 1 monitor call 
        jmp  pv_14  # 2 protection violation
        jmp  pf_14  # 3 page fault
        jmp  ii_14  # 4 illegal instruction
        jmp  zz_14  # 5 error indicator
        jmp  pi_14  # 6 privileged instruction
        jmp  iox_14 # 7 IOX error
        jmp  pty_14 # 8 memory parity error
        jmp  mor_14 # 9 memory out of range
        jmp  pow_14 # 10 power fail interrupt
        

error_14:
        jmp exit14
mc_14:
        jmp exit14
pv_14:
        jmp exit14
pf_14:  
        jmp exit14      
ii_14:
        jmp exit14
zz_14:
        jmp exit14
pi_14:
        jmp exit14
iox_14:
        jmp exit14
pty_14:
        jmp exit14
mor_14:
        jmp exit14
pow_14:
        jmp exit14
error:
        # unknown IIC  ??
        jmp exit14

exit14: wait
        jmp lev14

#################### END OF LEVEL 14 #####################




