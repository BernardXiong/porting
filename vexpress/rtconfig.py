import os

# toolchains options
ARCH='arm'
CPU='realview'
CROSS_TOOL='gcc'

if os.getenv('RTT_CC'):
	CROSS_TOOL = os.getenv('RTT_CC')

if  CROSS_TOOL == 'gcc':
	PLATFORM 	= 'gcc'
	EXEC_PATH 	= r'C:\Program Files (x86)\CodeSourcery\Sourcery_CodeBench_Lite_for_ARM_EABI\bin'
elif CROSS_TOOL == 'keil':
	PLATFORM 	= 'armcc'
	EXEC_PATH 	= 'C:/Keil'
elif CROSS_TOOL == 'iar':
    print '================ERROR============================'
    print 'Not support IAR yet!'
    print '================================================='
    exit(0)

if os.getenv('RTT_EXEC_PATH'):
	EXEC_PATH = os.getenv('RTT_EXEC_PATH')

BUILD = 'release'

if PLATFORM == 'gcc':
    # toolchains
    PREFIX = 'arm-none-eabi-'
    CC = PREFIX + 'gcc'
    CXX = PREFIX + 'g++'
    AS = PREFIX + 'gcc'
    AR = PREFIX + 'ar'
    LINK = PREFIX + 'gcc'
    TARGET_EXT = 'elf'
    SIZE = PREFIX + 'size'
    OBJDUMP = PREFIX + 'objdump'
    OBJCPY = PREFIX + 'objcopy'

    DEVICE = ' -march=armv7-a -marm -msoft-float'
    CFLAGS = DEVICE + ' -Wall'
    AFLAGS = ' -c' + DEVICE + ' -x assembler-with-cpp'
    LFLAGS = DEVICE + ' -Wl,--gc-sections,-Map=realview.map,-cref,-u,system_vectors -T realview.lds'

    CPATH = ''
    LPATH = ''

    if BUILD == 'debug':
        CFLAGS += ' -O0 -gdwarf-2'
        AFLAGS += ' -gdwarf-2'
    else:
        CFLAGS += ' -O2'

    POST_ACTION = OBJCPY + ' -O binary $TARGET rtthread.bin\n' + SIZE + ' $TARGET \n'

elif PLATFORM == 'armcc':
    # toolchains
    CC = 'armcc'
    CXX = 'armcc'    
    AS = 'armasm'
    AR = 'armar'
    LINK = 'armlink'
    TARGET_EXT = 'axf'

    DEVICE = ' --device DARMP'
    CFLAGS = DEVICE + ' --apcs=interwork'
    AFLAGS = DEVICE
    LFLAGS = DEVICE + ' --info sizes --info totals --info unused --info veneers --list rtthread-realview.map --scatter realview.sct'

    CFLAGS += ' -I' + EXEC_PATH + '/ARM/RV31/INC'
    LFLAGS += ' --libpath ' + EXEC_PATH + '/ARM/RV31/LIB'

    EXEC_PATH += '/arm/bin40/'

    if BUILD == 'debug':
        CFLAGS += ' -g -O0'
        AFLAGS += ' -g'
    else:
        CFLAGS += ' -O2'

    POST_ACTION = 'fromelf --bin $TARGET --output rtthread.bin \nfromelf -z $TARGET'

elif PLATFORM == 'iar':
    # toolchains
    CC = 'iccarm'
    AS = 'iasmarm'
    AR = 'iarchive'
    LINK = 'ilinkarm'
    TARGET_EXT = 'out'

    DEVICE = ' --cpu DARMP'

    CFLAGS = ''
    AFLAGS = ''
    LFLAGS = ' --config realview.icf'

    EXEC_PATH += '/arm/bin/'
    RT_USING_MINILIBC = False
    POST_ACTION = ''
