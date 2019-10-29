#
# This file is subject to the terms and conditions of the GNU General Public
# License.  See the file "COPYING" in the main directory of this archive
# for more details.
#
# Copyright © 2003 Atheros Communications, Inc.,  All Rights Reserved.
#

# Makefile for Atheros boards
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#

#.S.s:
#	$(CPP) $(CFLAGS) $< -o $*.s
#.S.o:
#	$(CC) $(CFLAGS) -D__ASSEMBLY__ -c $< -o $*.o

obj-y	+=	prom.o	\
		gpio.o	\
		irq.o	\
		setup.o	\
		misc.o \
		platform.o

ifdef CONFIG_ATH_TEST_TIMER
obj-y	+=	test-timers.o
else
obj-y	+=	wdt.o
endif

obj-$(CONFIG_MACH_AR724x) += 724x.o
obj-$(CONFIG_MACH_AR933x) += 933x.o
obj-$(CONFIG_MACH_AR934x) += 934x.o
obj-$(CONFIG_MACH_QCA955x) += 955x.o
obj-$(CONFIG_MACH_QCA953x) += 953x.o

ifndef CONFIG_ATH_EMULATION
obj-$(CONFIG_MACH_AR934x) += otp.o
obj-$(CONFIG_MACH_QCA955x) += otp.o
obj-$(CONFIG_MACH_AR934x) += clksw.o
endif

obj-$(CONFIG_ATH_HWCS) 	  += hwcs.o

ifneq ($(strip $(CONFIG_ATH_DDR_RELEASE_TIMER)),0) #{
obj-y 	  += timers.o
else #}{
ifdef CONFIG_ATH_TURN_ON_DDR_HOG #{
obj-y 	  += timers.o
endif #}
endif #}

ifdef CONFIG_CUS100
CFLAGS_gpio.o += -DCONFIG_CUS100=1
endif

#ifndef JUMPSTART_GPIO
#ifneq ($(CONFIG_JUMPSTART_GPIO_SUPPORT),)
#JUMPSTART_GPIO=$(CONFIG_JUMPSTART_GPIO_NUMBER)
#else
#ifndef CONFIG_MACH_QCA953x 
#JUMPSTART_GPIO=12
#endif
#endif
#endif

ifndef WPS_LED_GPIO_PIN
ifneq ($(CONFIG_AP_WPS_LED_GPIO_SUPPORT),)
WPS_LED_GPIO_PIN=$(CONFIG_AP_WPS_LED_GPIO_NUMBER)
else
ifndef CONFIG_MACH_QCA953x 
WPS_LED_GPIO_PIN=5
endif
endif
endif

ifdef WPS_LED_GPIO_PIN
CFLAGS_gpio.o += -DWPS_LED_GPIO=$(WPS_LED_GPIO_PIN)
endif

#ifdef JUMPSTART_GPIO
#CFLAGS_gpio.o += -DJUMPSTART_GPIO=$(JUMPSTART_GPIO)
#endif

ifdef CONFIG_I2S
CFLAGS_gpio.o += -DCONFIG_I2S=$(CONFIG_I2S)
endif

ifneq ($(CONFIG_AP_USB_LED_GPIO_SUPPORT),)
CFLAGS_gpio.o += -DAP_USB_LED_GPIO=$(CONFIG_AP_USB_LED_GPIO_NUMBER)
else
ifdef AP_USB_LED_GPIO
CFLAGS_gpio.o += -DAP_USB_LED_GPIO=$(AP_USB_LED_GPIO)
endif
endif

ifdef AP_READY_LED_GPIO
CFLAGS_gpio.o += -DAP_READY_LED_GPIO=$(AP_READY_LED_GPIO)
endif

ifdef POWER_ON_RLED_GPIO
CFLAGS_gpio.o += -DPOWER_ON_RLED_GPIO=$(POWER_ON_RLED_GPIO)
endif

ifdef POWER_ON_GLED_GPIO
CFLAGS_gpio.o += -DPOWER_ON_GLED_GPIO=$(POWER_ON_GLED_GPIO)
endif

ifdef AP_RESET_GPIO
CFLAGS_gpio.o += -DAP_RESET_GPIO=$(AP_RESET_GPIO)
endif

ifeq ($(BUILD_EMU), 24)
EXTRA_CFLAGS += -DCONFIG_HORNET_EMULATION_WLAN_HARDI
endif

ifeq ($(CONFIG_HORNET_XTAL), 40)
EXTRA_CFLAGS += -DCONFIG_40MHZ_XTAL_SUPPORT
endif

#EXTRA_CFLAGS				+=	-fno-schedule-insns -fno-schedule-insns2
