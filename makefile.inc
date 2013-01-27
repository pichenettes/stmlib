# Copyright 2012 Olivier Gillet.
#
# Author: Olivier Gillet (ol.gillet@gmail.com)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 
# See http://creativecommons.org/licenses/MIT/ for more information.

# ------------------------------------------------------------------------------
# Files and directories
# ------------------------------------------------------------------------------

TOOLCHAIN_PATH = /usr/local/arm/
TOOLCHAIN_BIN  = $(TOOLCHAIN_PATH)bin/
STM32_PATH     = stmlib/third_party/STM

PGM_SERIAL_PORT = /dev/tty.usbserial-A100S09O
PGM_SERIAL_BAUD_RATE = 115200
PGM_SERIAL_VERIFY = -v

# --- Do not modify anything below this point ----------------------------------

BUILD_ROOT     = build/
BUILD_DIR      = $(BUILD_ROOT)$(TARGET)/

CC             = $(TOOLCHAIN_BIN)arm-none-eabi-gcc
CXX            = $(TOOLCHAIN_BIN)arm-none-eabi-g++
AS             = $(TOOLCHAIN_BIN)arm-none-eabi-as
OBJCOPY        = $(TOOLCHAIN_BIN)arm-none-eabi-objcopy
OBJDUMP        = $(TOOLCHAIN_BIN)arm-none-eabi-objdump
AR             = $(TOOLCHAIN_BIN)arm-none-eabi-ar
SIZE           = $(TOOLCHAIN_BIN)arm-none-eabi-size
NM             = $(TOOLCHAIN_BIN)arm-none-eabi-nm
REMOVE         = rm -f
CAT            = cat
FORMAT_SIZE    = | figlet | cowsay -n -f moose
FORMAT_RAMSIZE = | figlet | cowsay -n -f small

STM32LOADER_PATH = stmlib/programming/serial/
OPENOCD_SCRIPTS_PATH = stmlib/programming/jtag/


# ------------------------------------------------------------------------------
# Files and directories for the user code
# ------------------------------------------------------------------------------

VPATH          = $(PACKAGES)

CC_FILES       = $(notdir $(wildcard $(patsubst %,%/*.cc,$(PACKAGES))))
C_FILES        = $(notdir $(wildcard $(patsubst %,%/*.c,$(PACKAGES))))
AS_FILES       = $(notdir $(wildcard $(patsubst %,%/*.as,$(PACKAGES))))

TARGET_BIN     = $(BUILD_DIR)$(TARGET).bin
TARGET_ELF     = $(BUILD_DIR)$(TARGET).elf
TARGET_HEX     = $(BUILD_DIR)$(TARGET).hex
COMBO_HEX      = $(BUILD_DIR)$(TARGET)_bootloader_combo.hex
COMBO_BIN      = $(BUILD_DIR)$(TARGET)_bootloader_combo.bin
BOOTLOADER_HEX = $(BUILD_ROOT)$(BOOTLOADER)/$(BOOTLOADER).hex
TARGET_SIZE    = $(BUILD_DIR)$(TARGET).size
TARGETS        = $(BUILD_DIR)$(TARGET).*
DEP_FILE       = $(BUILD_DIR)depends.mk


# ------------------------------------------------------------------------------
# Flags for gcc/binutils
# ------------------------------------------------------------------------------

ARCHFLAGS      = -mcpu=cortex-m3 -mthumb
INCFLAGS       = \
			-I. \
			-DGCC_ARMCM3 \
			-DF_CPU=$(F_CPU) \
			-DF_CRYSTAL=$(F_CRYSTAL) \
			-DUSE_STDPERIPH_DRIVER \
			-D$(SYSCLOCK)=$(F_CPU)

ifeq ($(DENSITY),hd)
INCFLAGS += -DSTM32F10X_HD
else ifeq ($(DENSITY),md)
INCFLAGS += -DSTM32F10X_MD
else
INCFLAGS += -DSTM32F10X_CL
endif

ifeq ($(APPLICATION),TRUE)
INCFLAGS += -DAPPLICATION
LD_SUFFIX = _application
BASE_ADDRESS = 0x08004000
else
BASE_ADDRESS = 0x08000000
LD_SUFFIX =
endif

STARTUP_OBJ   = $(BUILD_DIR)startup_stm32f10x_$(DENSITY).o
LINKER_SCRIPT = stmlib/linker_scripts/stm32f10x_flash_$(DENSITY)$(LD_SUFFIX).ld

CCFLAGS         = \
			-g -O2 -Wall -Werror \
			-fasm \
			-fdata-sections \
			-finline \
			-finline-functions-called-once \
			-ffunction-sections \
			-fno-unroll-loops \
			-fshort-enums \
			-fno-move-loop-invariants \
			$(INCFLAGS) \
			$(ARCHFLAGS)

CPPFLAGS      = -fno-exceptions -fno-rtti

ASFLAGS       = $(ARCHFLAGS)

LDFLAGS       = \
			-Wl,-Map=$(BUILD_DIR)$(TARGET).map \
			-Wl,--gc-sections \
			-T $(LINKER_SCRIPT) -g \
			$(ARCHFLAGS) \
			$(INCFLAGS) \
			-L$(STM32_PATH)


# ------------------------------------------------------------------------------
# Files and directories for the system firmware
# ------------------------------------------------------------------------------

FW_BUILD_DIR = $(BUILD_ROOT)stm_firmware/
FW_CORE_DIR = $(STM32_PATH)/CMSIS/CM3
FW_STARTUP_DIR = $(FW_CORE_DIR)/startup/gcc

FW_STDDRIVER_DIR = $(STM32_PATH)/STM32F10x_StdPeriph_Driver
FW_STDDRIVER_SRCDIR = $(FW_STDDRIVER_DIR)/src
FW_STDDRIVER_INCDIR = $(FW_STDDRIVER_DIR)/inc

FW_USB_DIR = $(STM32_PATH)/STM32_USB-FS-Device_Driver
FW_USB_SRCDIR = $(FW_USB_DIR)/src
FW_USB_INCDIR = $(FW_USB_DIR)/inc

INCFLAGS += -I$(STM32_PATH) -I$(FW_CORE_DIR) -I$(FW_STARTUP_DIR) -I$(FW_STDDRIVER_INCDIR)
VPATH += $(FW_CORE_DIR) $(FW_STARTUP_DIR) $(FW_STDDRIVER_SRCDIR) $(STM32_PATH) $(FW_BUILD_DIR)

CORE_C_FILES = $(notdir $(wildcard $(patsubst %,%/*.c,$(FW_CORE_DIR))))
DRIVERS_C_FILES = $(notdir $(wildcard $(patsubst %,%/*.c,$(FW_STDDRIVER_SRCDIR))))
USB_C_FILES = $(notdir $(wildcard $(patsubst %,%/*.c,$(FW_USB_SRCDIR))))

C_FILES += $(CORE_C_FILES)
C_FILES += $(DRIVERS_C_FILES)

ifeq ($(USB),enabled)
INCFLAGS += -I$(FW_USB_INCDIR)
VPATH += $(FW_USB_SRCDIR)
C_FILES += $(USB_C_FILES)
endif


# ------------------------------------------------------------------------------
# Source compiling and dependency analysis
# ------------------------------------------------------------------------------

$(BUILD_DIR)%.o: %.s
	$(CC) -c -x assembler-with-cpp $(ASFLAGS) $< -o $@

$(BUILD_DIR)%.o: %.c
	$(CC) -c $(CCFLAGS) -std=c99 $< -o $@

$(BUILD_DIR)%.o: %.cc
	$(CXX) -c $(CCFLAGS) $(CPPFLAGS) $< -o $@

$(BUILD_DIR)%.d: %.s
	$(CC) -c -x assembler-with-cpp -MM $(ASFLAGS) $< -MF $@ -MT $(@:.d=.o)

$(BUILD_DIR)%.d: %.c
	$(CC) -MM $(CCFLAGS) $< -MF $@ -MT $(@:.d=.o)

$(BUILD_DIR)%.d: %.cc
	$(CXX) -MM $(CCFLAGS) $(CPPFLAGS) $< -MF $@ -MT $(@:.d=.o)


# ------------------------------------------------------------------------------
# Object file conversion
# ------------------------------------------------------------------------------

$(BUILD_DIR)%.hex: $(BUILD_DIR)%.elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)%.bin: $(BUILD_DIR)%.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD_DIR)%.lss: $(BUILD_DIR)%.elf
	$(OBJDUMP) -h -S $< > $@

$(BUILD_DIR)%.sym: $(BUILD_DIR)%.elf
	$(NM) -n $< > $@


# ------------------------------------------------------------------------------
# What to build
# ------------------------------------------------------------------------------

OBJ_FILES      = $(CC_FILES:.cc=.o) $(C_FILES:.c=.o) $(AS_FILES:.S=.o)
OBJS           = $(patsubst %,$(BUILD_DIR)%,$(OBJ_FILES)) $(STARTUP_OBJ)
DEPS           = $(OBJS:.o=.d)

all:	$(BUILD_DIR) $(TARGET_HEX)


# ------------------------------------------------------------------------------
# Resources
# ------------------------------------------------------------------------------

RESOURCE_COMPILER = stmlib/tools/resources_compiler.py

resources:	$(TARGET)/resources.cc

$(TARGET)/resources.cc:	$(wildcard $(RESOURCES)/*.py)
		python $(RESOURCE_COMPILER) $(RESOURCES)/resources.py


# ------------------------------------------------------------------------------
# Main rules
# ------------------------------------------------------------------------------

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET_ELF):  $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET_ELF) $(OBJS)

$(DEP_FILE):  $(BUILD_DIR) $(DEPS)
	cat $(DEPS) > $(DEP_FILE)

bin:	$(TARGET_BIN)

hex:	$(TARGET_HEX)

clean:
	$(REMOVE) $(OBJS) $(TARGETS) $(DEP_FILE) $(DEPS)

depends:  $(DEPS)
	cat $(DEPS) > $(DEP_FILE)

$(TARGET_SIZE):  $(TARGET_ELF)
	$(SIZE) $(TARGET_ELF) > $(TARGET_SIZE)

$(BUILD_DIR)$(TARGET).top_symbols: $(TARGET_ELF)
	$(NM) $(TARGET_ELF) --size-sort -C -f bsd -r > $@

size: $(TARGET_SIZE)
	cat $(TARGET_SIZE) | awk '{ print $$1+$$2 }' | tail -n1 $(FORMAT_SIZE)

ramsize: $(TARGET_SIZE)
	cat $(TARGET_SIZE) | awk '{ print $$2+$$3 }' | tail -n1 $(FORMAT_RAMSIZE)

size_report:  build/$(TARGET)/$(TARGET).lss build/$(TARGET)/$(TARGET).top_symbols

.PHONY: all bin clean depends resources

include $(DEP_FILE)


# ------------------------------------------------------------------------------
# Bootloader merging
# ------------------------------------------------------------------------------

$(COMBO_BIN):  $(TARGET_HEX) $(BOOTLOADER_HEX)
	cat  $(TARGET_HEX) $(BOOTLOADER_HEX) | \
			awk -f stmlib/programming/merge_hex.awk > $(COMBO_HEX)
	$(OBJCOPY) -I ihex -O binary $(COMBO_HEX) $(COMBO_BIN)


# ------------------------------------------------------------------------------
# Firmware flashing
# ------------------------------------------------------------------------------

upload:	$(UPLOAD_COMMAND)

upload_serial: $(TARGET_BIN)
	@python $(STM32LOADER_PATH)stm32loader.py \
		-p $(PGM_SERIAL_PORT) \
		-b $(PGM_SERIAL_BAUD_RATE) \
		-e \
		-a $(BASE_ADDRESS) \
		-w $(TARGET_BIN) $(PGM_SERIAL_VERIFY)

image_serial:
	@python $(STM32LOADER_PATH)stm32loader.py \
		-p $(PGM_SERIAL_PORT) \
		-b $(PGM_SERIAL_BAUD_RATE) \
		-r -l 18000 image.bin

upload_serial_no_erase: $(TARGET_BIN)
	@python $(STM32LOADER_PATH)stm32loader.py \
		-p $(PGM_SERIAL_PORT) \
		-b $(PGM_SERIAL_BAUD_RATE) \
		-a $(BASE_ADDRESS) \
		-w $(TARGET_BIN) $(PGM_SERIAL_VERIFY)

upload_combo_serial: $(TARGET_HEX)
	@python $(STM32LOADER_PATH)stm32loader.py \
		-p $(PGM_SERIAL_PORT) \
		-b $(PGM_SERIAL_BAUD_RATE) \
		-e \
		-w $(COMBO_BIN) $(PGM_SERIAL_VERIFY)

upload_jtag: $(TARGET_BIN)
	openocd \
		-f $(OPENOCD_SCRIPTS_PATH)interface.cfg \
		-f $(OPENOCD_SCRIPTS_PATH)stm32.cfg \
		-f $(OPENOCD_SCRIPTS_PATH)prelude.cfg \
		-c "flash write_bank 0 $(TARGET_BIN) 0x0" \
		-f $(OPENOCD_SCRIPTS_PATH)postlude.cfg

upload_combo_jtag: $(COMBO_BIN)
	openocd \
		-f $(OPENOCD_SCRIPTS_PATH)interface.cfg \
		-f $(OPENOCD_SCRIPTS_PATH)stm32.cfg \
		-f $(OPENOCD_SCRIPTS_PATH)prelude.cfg \
		-c "flash write_bank 0 $(COMBO_BIN) 0x0" \
		-f $(OPENOCD_SCRIPTS_PATH)postlude.cfg

# Phony target for killing the system FTDI driver on OS X.
kill_ftdi:
	sudo kextunload -b com.FTDI.driver.FTDIUSBSerialDriver

reload_ftdi:
	sudo kextload -b com.FTDI.driver.FTDIUSBSerialDriver

.PHONY: upload upload_serial upload_serial_no_erase upload_jtag
.PHONY: upload_bundle_serial upload_jtag
.PHONY: kill_ftdi reload_ftdi
