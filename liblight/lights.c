/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2014 The  Linux Foundation. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


// #define LOG_NDEBUG 0

#include <cutils/log.h>

#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

#include <sys/ioctl.h>
#include <sys/types.h>

#include <hardware/lights.h>

/******************************************************************************/

static pthread_once_t g_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static struct light_state_t g_notification;
static struct light_state_t g_battery;
static int g_attention = 0;

char const*const RED_LED_FILE
        = "/sys/class/leds/red/brightness";

char const*const GREEN_LED_FILE
        = "/sys/class/leds/green/brightness";

char const*const BLUE_LED_FILE
        = "/sys/class/leds/blue/brightness";

char const*const LCD_FILE
        = "/sys/class/leds/lcd-backlight/brightness";

char const*const BUTTON_FILE
        = "/sys/class/leds/button-backlight/brightness";

char const*const RED_BLINK_FILE
        = "/sys/class/leds/red/blink";

char const*const GREEN_BLINK_FILE
        = "/sys/class/leds/green/blink";

char const*const BLUE_BLINK_FILE
        = "/sys/class/leds/blue/blink";

//wangyanhui add start 20141120
/* RED LED */
char const*const RED_TRIGGER_FILE
        = "/sys/class/leds/red/trigger";

char const*const RED_DELAY_ON_FILE
        = "/sys/class/leds/red/delay_on";

char const*const RED_DELAY_OFF_FILE
        = "/sys/class/leds/red/delay_off";

/* GREEN LED */
char const*const GREEN_TRIGGER_FILE
        = "/sys/class/leds/green/trigger";

char const*const GREEN_DELAY_ON_FILE
        = "/sys/class/leds/green/delay_on";

char const*const GREEN_DELAY_OFF_FILE
        = "/sys/class/leds/green/delay_off";

/* BLUE LED */
char const*const BLUE_TRIGGER_FILE
        = "/sys/class/leds/blue/trigger";

char const*const BLUE_DELAY_ON_FILE
        = "/sys/class/leds/blue/delay_on";

char const*const BLUE_DELAY_OFF_FILE
        = "/sys/class/leds/blue/delay_off";

//wangyanhui add end 20141120


/**
 * device methods
 */

void init_globals(void)
{
    // init the mutex
    pthread_mutex_init(&g_lock, NULL);
}

static int
write_int(char const* path, int value)
{
    int fd;
    static int already_warned = 0;

    fd = open(path, O_RDWR);
    if (fd >= 0) {
        char buffer[20];
        int bytes = snprintf(buffer, sizeof(buffer), "%d\n", value);
        ssize_t amt = write(fd, buffer, (size_t)bytes);
        close(fd);
        return amt == -1 ? -errno : 0;
    } else {
        if (already_warned == 0) {
            ALOGE("write_int failed to open %s\n", path);
            already_warned = 1;
        }
        return -errno;
    }
}

static int
is_lit(struct light_state_t const* state)
{
    return state->color & 0x00ffffff;
}

static int
rgb_to_brightness(struct light_state_t const* state)
{
    int color = state->color & 0x00ffffff;
    return ((77*((color>>16)&0x00ff))
            + (150*((color>>8)&0x00ff)) + (29*(color&0x00ff))) >> 8;
}

static int
set_light_backlight(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int err = 0;
    int brightness = rgb_to_brightness(state);
    if(!dev) {
        return -1;
    }
    pthread_mutex_lock(&g_lock);
    err = write_int(LCD_FILE, brightness);
    pthread_mutex_unlock(&g_lock);
    return err;
}

//wangyanhui add start 20141120
int led_wait_delay(int ms) 
{
	struct timespec req = {.tv_sec = 0, .tv_nsec = ms*1000000};
	struct timespec rem;
	int ret = nanosleep(&req, &rem);

	while(ret)
	{
		if(errno == EINTR)
		{
			req.tv_sec  = rem.tv_sec;
			req.tv_nsec = rem.tv_nsec;
			ret = nanosleep(&req, &rem);
		}
		else
		{
			perror("nanosleep");
			return errno;
		}
	}
	return 0;
}

static int
write_str(char const* path, char *str)
{
    int fd;

    fd = open(path, O_WRONLY);
    if (fd >= 0) {
        char buffer[20];
        int bytes = sprintf(buffer, "%s", str);
        int amt = write(fd, buffer, bytes);
        close(fd);
        return amt == -1 ? 10 : 0;
    } else {
        return -errno;
    }
}

static int
blink_red(int level, int onMS, int offMS)
{
	static int preStatus = 0; // 0: off, 1: blink, 2: no blink
	int nowStatus;
	int i = 0;

	if (level == 0)
		nowStatus = 0;
	else if (onMS && offMS)
		nowStatus = 1;
	else
		nowStatus = 2;

	if (preStatus == nowStatus)
		return -1;

#if 0
	ALOGD("blink_red, level=%d, onMS=%d, offMS=%d\n", level, onMS, offMS);
#endif
	if (nowStatus == 0) {
        	write_int(RED_LED_FILE, 0);
	}
	else if (nowStatus == 1) {
//        	write_int(RED_LED_FILE, level); // default full brightness
		int ret = 0;
		ret = write_str(RED_TRIGGER_FILE, "timer");
		while (((access(RED_DELAY_OFF_FILE, F_OK) == -1) || (access(RED_DELAY_OFF_FILE, R_OK|W_OK) == -1)) && i<10) {
			//ALOGD("RED_DELAY_OFF_FILE doesn't exist or cannot write!!\n");
			led_wait_delay(5);//sleep 5ms for wait kernel LED class create led delay_off/delay_on node of fs
			i++;
		}
		write_int(RED_DELAY_OFF_FILE, offMS);
		write_int(RED_DELAY_ON_FILE, onMS);
	}
	else {
		write_str(RED_TRIGGER_FILE, "none");
        	write_int(RED_LED_FILE, 255); // default full brightness
	}

	preStatus = nowStatus;

	return 0;
}

static int
blink_green(int level, int onMS, int offMS)
{
	static int preStatus = 0; // 0: off, 1: blink, 2: no blink
	int nowStatus;
	int i = 0;

	if (level == 0)
		nowStatus = 0;
	else if (onMS && offMS)
		nowStatus = 1;
	else
		nowStatus = 2;

	if (preStatus == nowStatus)
		return -1;

#if 0
	ALOGD("blink_green, level=%d, onMS=%d, offMS=%d\n", level, onMS, offMS);
#endif
	if (nowStatus == 0) {
        	write_int(GREEN_LED_FILE, 0);
	}
	else if (nowStatus == 1) {
//        	write_int(GREEN_LED_FILE, level); // default full brightness
		write_str(GREEN_TRIGGER_FILE, "timer");
		while (((access(GREEN_DELAY_OFF_FILE, F_OK) == -1) || (access(GREEN_DELAY_OFF_FILE, R_OK|W_OK) == -1)) && i<10) {
			//ALOGD("GREEN_DELAY_OFF_FILE doesn't exist or cannot write!!\n");
			led_wait_delay(5);//sleep 5ms for wait kernel LED class create led delay_off/delay_on node of fs
			i++;
		}
		write_int(GREEN_DELAY_OFF_FILE, offMS);
		write_int(GREEN_DELAY_ON_FILE, onMS);
	}
	else {
		write_str(GREEN_TRIGGER_FILE, "none");
        	write_int(GREEN_LED_FILE, 255); // default full brightness
	}

	preStatus = nowStatus;

	return 0;
}

static int
blink_blue(int level, int onMS, int offMS)
{
	static int preStatus = 0; // 0: off, 1: blink, 2: no blink
	int nowStatus;
	int i = 0;

	if (level == 0)
		nowStatus = 0;
	else if (onMS && offMS)
		nowStatus = 1;
	else
		nowStatus = 2;

	if (preStatus == nowStatus)
		return -1;

#if 0
	ALOGD("blink_blue, level=%d, onMS=%d, offMS=%d\n", level, onMS, offMS);
#endif
	if (nowStatus == 0) {
        	write_int(BLUE_LED_FILE, 0);
	}
	else if (nowStatus == 1) {
		write_str(BLUE_TRIGGER_FILE, "timer");
		while (((access(BLUE_DELAY_OFF_FILE, F_OK) == -1) || (access(BLUE_DELAY_OFF_FILE, R_OK|W_OK) == -1)) && i<10) {
			//ALOGD("BLUE_DELAY_OFF_FILE doesn't exist or cannot write!!\n");
			i++;
		}
		write_int(BLUE_DELAY_OFF_FILE, offMS);
		write_int(BLUE_DELAY_ON_FILE, onMS);
	}
	else {
		write_str(BLUE_TRIGGER_FILE, "none");
        	write_int(BLUE_LED_FILE, 255); // default full brightness
	}

	preStatus = nowStatus;

	return 0;
}
//wangyanhui end 20141120

static int
set_speaker_light_locked(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int red, green, blue;
    int blink;
    int onMS, offMS;
    unsigned int colorRGB;

    if(!dev) {
        return -1;
    }

    switch (state->flashMode) {
        case LIGHT_FLASH_TIMED:
            onMS = state->flashOnMS;
            offMS = state->flashOffMS;
            break;
        case LIGHT_FLASH_NONE:
        default:
            onMS = 0;
            offMS = 0;
            break;
    }

    colorRGB = state->color;

#if 0
    ALOGD("set_speaker_light_locked mode %d, colorRGB=%08X, onMS=%d, offMS=%d\n",
            state->flashMode, colorRGB, onMS, offMS);
#endif

    red = (colorRGB >> 16) & 0xFF;
    green = (colorRGB >> 8) & 0xFF;
    blue = colorRGB & 0xFF;

//wangyanhui modify start 20141120
    if (red) {
        blink_green(0, 0, 0);
        blink_blue(0, 0, 0);
        blink_red(red, onMS, offMS);
    }
    else if (green) {
        blink_red(0, 0, 0);
        blink_blue(0, 0, 0);
        blink_green(green, onMS, offMS);
    }
    else if (blue) {
        blink_red(0, 0, 0);
        blink_green(0, 0, 0);
        blink_blue(blue, onMS, offMS);
    }
    else {
        blink_red(0, 0, 0);
        blink_green(0, 0, 0);
        blink_blue(0, 0, 0);
    }
#if 0
    if (onMS > 0 && offMS > 0) {
        blink = 1;
    } else {
        blink = 0;
    }


    if (blink) {
        if (red) {
            if (write_int(RED_BLINK_FILE, blink))
                write_int(RED_LED_FILE, 0);
	}
        if (green) {
            if (write_int(GREEN_BLINK_FILE, blink))
                write_int(GREEN_LED_FILE, 0);
	}
        if (blue) {
            if (write_int(BLUE_BLINK_FILE, blink))
                write_int(BLUE_LED_FILE, 0);
	}
    } else {
        write_int(RED_LED_FILE, red);
        write_int(GREEN_LED_FILE, green);
        write_int(BLUE_LED_FILE, blue);
    }
#endif	
//wangyanhui modify end 20141120
    return 0;
}

static void
handle_speaker_battery_locked(struct light_device_t* dev)
{
    if (is_lit(&g_battery)) {
        set_speaker_light_locked(dev, &g_battery);
    } else {
        set_speaker_light_locked(dev, &g_notification);
    }
}

static int
set_light_battery(struct light_device_t* dev,
        struct light_state_t const* state)
{
    pthread_mutex_lock(&g_lock);
    g_battery = *state;
    handle_speaker_battery_locked(dev);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

static int
set_light_notifications(struct light_device_t* dev,
        struct light_state_t const* state)
{
    pthread_mutex_lock(&g_lock);
    g_notification = *state;
    handle_speaker_battery_locked(dev);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

static int
set_light_attention(struct light_device_t* dev,
        struct light_state_t const* state)
{
    pthread_mutex_lock(&g_lock);
    if (state->flashMode == LIGHT_FLASH_HARDWARE) {
        g_attention = state->flashOnMS;
    } else if (state->flashMode == LIGHT_FLASH_NONE) {
        g_attention = 0;
    }
    handle_speaker_battery_locked(dev);
    pthread_mutex_unlock(&g_lock);
    return 0;
}

static int
set_light_buttons(struct light_device_t* dev,
        struct light_state_t const* state)
{
    int err = 0;
    if(!dev) {
        return -1;
    }
    pthread_mutex_lock(&g_lock);
    err = write_int(BUTTON_FILE, state->color & 0xFF);
    pthread_mutex_unlock(&g_lock);
    return err;
}

/** Close the lights device */
static int
close_lights(struct light_device_t *dev)
{
    if (dev) {
        free(dev);
    }
    return 0;
}


/******************************************************************************/

/**
 * module methods
 */

/** Open a new instance of a lights device using name */
static int open_lights(const struct hw_module_t* module, char const* name,
        struct hw_device_t** device)
{
    int (*set_light)(struct light_device_t* dev,
            struct light_state_t const* state);

    if (0 == strcmp(LIGHT_ID_BACKLIGHT, name))
        set_light = set_light_backlight;
    else if (0 == strcmp(LIGHT_ID_BATTERY, name))
        set_light = set_light_battery;
    else if (0 == strcmp(LIGHT_ID_NOTIFICATIONS, name))
        set_light = set_light_notifications;
    else if (0 == strcmp(LIGHT_ID_BUTTONS, name))
        set_light = set_light_buttons;
    else if (0 == strcmp(LIGHT_ID_ATTENTION, name))
        set_light = set_light_attention;
    else
        return -EINVAL;

    pthread_once(&g_init, init_globals);

    struct light_device_t *dev = malloc(sizeof(struct light_device_t));

    if(!dev)
        return -ENOMEM;

    memset(dev, 0, sizeof(*dev));

    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t*)module;
    dev->common.close = (int (*)(struct hw_device_t*))close_lights;
    dev->set_light = set_light;

    *device = (struct hw_device_t*)dev;
    return 0;
}

static struct hw_module_methods_t lights_module_methods = {
    .open =  open_lights,
};

/*
 * The lights Module
 */
struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .version_major = 1,
    .version_minor = 0,
    .id = LIGHTS_HARDWARE_MODULE_ID,
    .name = "lights Module",
    .author = "Google, Inc.",
    .methods = &lights_module_methods,
};
