/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/select.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include "AccelSensor.h"
#include "sensors.h"

#define FETCH_FULL_EVENT_BEFORE_RETURN	1
#define IGNORE_EVENT_TIME				10000000

#define	EVENT_TYPE_ACCEL_X	ABS_X
#define	EVENT_TYPE_ACCEL_Y	ABS_Y
#define	EVENT_TYPE_ACCEL_Z	ABS_Z

#define ACCEL_CONVERT		((GRAVITY_EARTH) / 1024)
#define CONVERT_ACCEL_X		ACCEL_CONVERT
#define CONVERT_ACCEL_Y		ACCEL_CONVERT
#define CONVERT_ACCEL_Z		ACCEL_CONVERT

#define SYSFS_I2C_SLAVE_PATH	"/device/device/"
#define SYSFS_INPUT_DEV_PATH	"/device/"

#define GSENSOR_FILE_FOR_HAL_PATH  "/persist/gs_cali_data"
//LINE <gsensor> <DATE20150211> <gsensor cali> limi.zhang
#define GSENSOR_DRV_FILE_PATH  "/persist/gs_cali_data_drv"

/*****************************************************************************/

AccelSensor::AccelSensor()
	: SensorBase(NULL, "accelerometer"),
	  mEnabled(0),
	  mInputReader(4),
	  mHasPendingEvent(false),
	  mEnabledTime(0),
	  cali_flag(0)
{
	int ret=-1;
    
	mPendingEvent.version = sizeof(sensors_event_t);
	mPendingEvent.sensor = SENSORS_ACCELERATION_HANDLE;
	mPendingEvent.type = SENSOR_TYPE_ACCELEROMETER;
	memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));
	if(is_file(GSENSOR_FILE_FOR_HAL_PATH)&& (cali_flag == 0)){
		    ret=gsensor_read_file();
		    if(ret ==0){
		    cali_flag=1;
		    ALOGE("acc.x= %f, acc.y=%f acc.z=%f \n",acc[0],acc[1],acc[2]); 		 
		}		
	}

	if (data_fd) {
		strlcpy(input_sysfs_path, "/sys/class/input/", sizeof(input_sysfs_path));
		strlcat(input_sysfs_path, input_name, sizeof(input_sysfs_path));
		strlcat(input_sysfs_path, SYSFS_I2C_SLAVE_PATH, sizeof(input_sysfs_path));
		input_sysfs_path_len = strlen(input_sysfs_path);
#ifdef TARGET_8610
		if (access(input_sysfs_path, F_OK)) {
			input_sysfs_path_len -= strlen(SYSFS_I2C_SLAVE_PATH);
			strcpy(&input_sysfs_path[input_sysfs_path_len],
					SYSFS_INPUT_DEV_PATH);
			input_sysfs_path_len += strlen(SYSFS_INPUT_DEV_PATH);
		}
#endif
		enable(0, 1);
	}
}

AccelSensor::AccelSensor(char *name)
	: SensorBase(NULL, "accelerometer"),
	  mEnabled(0),
	  mInputReader(4),
	  mHasPendingEvent(false),
	  mEnabledTime(0),
	  cali_flag(0)
{
	int ret=-1;
    
	mPendingEvent.version = sizeof(sensors_event_t);
	mPendingEvent.sensor = SENSORS_ACCELERATION_HANDLE;
	mPendingEvent.type = SENSOR_TYPE_ACCELEROMETER;
	memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));
	if(is_file(GSENSOR_FILE_FOR_HAL_PATH)&& (cali_flag == 0)){
		    ret=gsensor_read_file();
		    if(ret ==0){
		    cali_flag=1;
		    ALOGE("acc.x= %f, acc.y=%f acc.z=%f \n",acc[0],acc[1],acc[2]); 		 
		}		
	}

	if (data_fd) {
		strlcpy(input_sysfs_path, SYSFS_CLASS, sizeof(input_sysfs_path));
		strlcat(input_sysfs_path, name, sizeof(input_sysfs_path));
		strlcat(input_sysfs_path, "/", sizeof(input_sysfs_path));
		input_sysfs_path_len = strlen(input_sysfs_path);
		ALOGI("The accel sensor path is %s",input_sysfs_path);
		enable(0, 1);
	}
}

AccelSensor::AccelSensor(SensorContext *context)
	: SensorBase(NULL, NULL),
	  mEnabled(0),
	  mInputReader(4),
	  mHasPendingEvent(false),
	  mEnabledTime(0),
	  cali_flag(0)
{
	int ret=-1;
    
	mPendingEvent.version = sizeof(sensors_event_t);
	mPendingEvent.sensor = context->sensor->handle;
	mPendingEvent.type = SENSOR_TYPE_ACCELEROMETER;
	memset(mPendingEvent.data, 0, sizeof(mPendingEvent.data));
	if(is_file(GSENSOR_FILE_FOR_HAL_PATH)&& (cali_flag == 0)){
		    ret=gsensor_read_file();
		    if(ret ==0){
		    cali_flag=1;
		    ALOGE("acc.x= %f, acc.y=%f acc.z=%f \n",acc[0],acc[1],acc[2]); 		 
		}		
	}

	strlcpy(input_sysfs_path, context->enable_path, sizeof(input_sysfs_path));
	input_sysfs_path_len = strlen(input_sysfs_path);
	data_fd = context->data_fd;
	ALOGI("The accel sensor path is %s",input_sysfs_path);
	enable(0, 1);
}

AccelSensor::~AccelSensor() {
	if (mEnabled) {
		enable(0, 0);
	}
}

int AccelSensor::setInitialState() {
	struct input_absinfo absinfo_x;
	struct input_absinfo absinfo_y;
	struct input_absinfo absinfo_z;
	float value;
	int ret=-1;
	float temp_data;
	
	if (!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ACCEL_X), &absinfo_x) &&
		!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ACCEL_Y), &absinfo_y) &&
		!ioctl(data_fd, EVIOCGABS(EVENT_TYPE_ACCEL_Z), &absinfo_z)) {
		value = absinfo_x.value;
		temp_data = value * CONVERT_ACCEL_X;
		if(cali_flag==1) temp_data +=(acc[0]);				
		mPendingEvent.data[0] = temp_data;
		value = absinfo_y.value;
		temp_data = value * CONVERT_ACCEL_Y;
		if(cali_flag==1) temp_data +=(acc[1]);	
		mPendingEvent.data[1] = temp_data;
		value = absinfo_z.value;
		temp_data = value * CONVERT_ACCEL_Z;
		if(cali_flag==1) temp_data +=(acc[2]);
		mPendingEvent.data[2] = temp_data;
		mHasPendingEvent = true;
	}
	return 0;
}

//BEGIN <gsensor> <DATE20150211> <gsensor cali> limi.zhang
static bool is_file(const char *path)
{
    struct stat statbuf;
    if(lstat(path, &statbuf) ==0)
        return S_ISREG(statbuf.st_mode) != 0;
    return false;
}
//END <gsensor> <DATE20150211> <gsensor cali> limi.zhang
 int AccelSensor::gsensor_read_file(){

             FILE *fp;

           fp= fopen(GSENSOR_FILE_FOR_HAL_PATH,"r");  
 
            if(fp == NULL)  
            {  
		    ALOGE("open %s error. \n",GSENSOR_FILE_FOR_HAL_PATH);  
		  return -1;
            }  
            else  
            {  
          
                  fscanf(fp, "%f, %f, %f", &acc[0], &acc[1],&acc[2]);  
		    ALOGE("acc->x =%f,acc->y=%f,acc->z=%f\n",acc[0],acc[1],acc[2]);		  
                  fclose(fp);  
            	}
        //BEGIN <gsensor> <DATE20150211> <gsensor cali> limi.zhang    	
	    if(is_file(GSENSOR_DRV_FILE_PATH)) {
			remove(GSENSOR_DRV_FILE_PATH);
         }		
        //END <gsensor> <DATE20150211> <gsensor cali> limi.zhang
     return 0;


  }

bool AccelSensor:: is_file(const char *path)
{
    struct stat statbuf;
    if(lstat(path, &statbuf) ==0)
        return S_ISREG(statbuf.st_mode) != 0;
    return false;
}


int AccelSensor::enable(int32_t, int en) {
	int flags = en ? 1 : 0;
	char propBuf[PROPERTY_VALUE_MAX];
	property_get("sensors.accel.loopback", propBuf, "0");
	if (strcmp(propBuf, "1") == 0) {
		ALOGE("sensors.accel.loopback is set");
		mEnabled = flags;
		return 0;
	}

	if (flags != mEnabled) {
		int fd;
		strlcpy(&input_sysfs_path[input_sysfs_path_len],
				SYSFS_ENABLE, SYSFS_MAXLEN);
		fd = open(input_sysfs_path, O_RDWR);
		if (fd >= 0) {
			char buf[2];
			int err;
			buf[1] = 0;
			if (flags) {
				buf[0] = '1';
				mEnabledTime = getTimestamp() + IGNORE_EVENT_TIME;
			} else {
				buf[0] = '0';
			}
			err = write(fd, buf, sizeof(buf));
			close(fd);
			mEnabled = flags;
			setInitialState();
			return 0;
		}
		ALOGE("AccelSensor: failed to open %s", input_sysfs_path);
		return -1;
	}
	return 0;
}

bool AccelSensor::hasPendingEvents() const {
	return mHasPendingEvent;
}

int AccelSensor::setDelay(int32_t, int64_t delay_ns)
{
	int fd;
	char propBuf[PROPERTY_VALUE_MAX];
	property_get("sensors.accel.loopback", propBuf, "0");
	if (strcmp(propBuf, "1") == 0) {
		ALOGE("sensors.accel.loopback is set");
		return 0;
	}
	int delay_ms = delay_ns / 1000000;
	strlcpy(&input_sysfs_path[input_sysfs_path_len],
			SYSFS_POLL_DELAY, SYSFS_MAXLEN);
	fd = open(input_sysfs_path, O_RDWR);
	if (fd >= 0) {
		char buf[80];
		sprintf(buf, "%d", delay_ms);
		write(fd, buf, strlen(buf)+1);
		close(fd);
		return 0;
	}
	return -1;
}

int AccelSensor::readEvents(sensors_event_t* data, int count)
{
	float temp_data;
	if (count < 1)
		return -EINVAL;

	if (mHasPendingEvent) {
		mHasPendingEvent = false;
		mPendingEvent.timestamp = getTimestamp();
		*data = mPendingEvent;
		return mEnabled ? 1 : 0;
	}

	ssize_t n = mInputReader.fill(data_fd);
	if (n < 0)
		return n;

	int numEventReceived = 0;
	input_event const* event;

#if FETCH_FULL_EVENT_BEFORE_RETURN
again:
#endif
	while (count && mInputReader.readEvent(&event)) {
		int type = event->type;
		if (type == EV_ABS) {
			float value = event->value;
//			ALOGE("AccelSensor: Original: code = %d; value = %f ", event->code, value);			
			
			if (event->code == EVENT_TYPE_ACCEL_X) {
				temp_data = value * CONVERT_ACCEL_X;
//				ALOGE("AccelSensor: Before: code = %d; value = %f ", event->code, temp_data);			
				if(cali_flag==1) temp_data +=(acc[0]);				
				mPendingEvent.data[0] = temp_data;
			} else if (event->code == EVENT_TYPE_ACCEL_Y) {
				temp_data = value * CONVERT_ACCEL_Y;
//				ALOGE("AccelSensor: Before: code = %d; value = %f ", event->code, temp_data);							
				if(cali_flag==1) temp_data +=(acc[1]);			    
				mPendingEvent.data[1] = temp_data;
			} else if (event->code == EVENT_TYPE_ACCEL_Z) {
				temp_data = value * CONVERT_ACCEL_Z;
//				ALOGE("AccelSensor: Before: code = %d; value = %f ", event->code, temp_data);							
				if(cali_flag==1) temp_data +=(acc[2]);			    
				mPendingEvent.data[2] = temp_data;
			}
//			ALOGE("AccelSensor: After: code = %d; value = %f ", event->code, temp_data);			
		} else if (type == EV_SYN) {
			mPendingEvent.timestamp = timevalToNano(event->time);
			if (mEnabled) {
				if (mPendingEvent.timestamp >= mEnabledTime) {
					*data++ = mPendingEvent;
					numEventReceived++;
				}
				count--;
			}
		} else {
			ALOGE("AccelSensor: unknown event (type=%d, code=%d)",
					type, event->code);
		}
		mInputReader.next();
	}

#if FETCH_FULL_EVENT_BEFORE_RETURN
	/* if we didn't read a complete event, see if we can fill and
	   try again instead of returning with nothing and redoing poll. */
	if (numEventReceived == 0 && mEnabled == 1) {
		n = mInputReader.fill(data_fd);
		if (n)
			goto again;
	}
#endif

	return numEventReceived;
}

int AccelSensor::calibrate(int32_t handle, struct cal_cmd_t *para,
				struct cal_result_t *cal_result)
{
	int fd;
	char temp[3][LENGTH];
	char buf[3 * LENGTH];
	char *token, *strsaveptr, *endptr;
	int i, err;
	off_t offset;
	int para1 = 0;
	if (para == NULL || cal_result == NULL) {
		ALOGE("Null pointer calibrate parameters\n");
		return -1;
	}
	para1 = CMD_CAL(para->axis, para->apply_now);
	strlcpy(&input_sysfs_path[input_sysfs_path_len],
			SYSFS_CALIBRATE, SYSFS_MAXLEN);
	fd = open(input_sysfs_path, O_RDWR);
	if (fd >= 0) {
		snprintf(buf, sizeof(buf), "%d", para1);
		write(fd, buf, strlen(buf)+1);
	} else {
		ALOGE("open %s failed\n", input_sysfs_path);
		return -1;
	}
	if (fd >= 0) {
		offset = lseek(fd, 0, SEEK_SET);
		char *p = buf;
		memset(buf, 0, sizeof(buf));
		err = read(fd, buf, sizeof(buf)-1);
		if(err < 0) {
			ALOGE("read error\n");
			close(fd);
			return err;
		}
		for(i = 0; i < sizeof(temp) / LENGTH; i++, p = NULL) {
			token = strtok_r(p, ",", &strsaveptr);
			if(token == NULL)
				break;
			if(strlen(token) > LENGTH - 1) {
				ALOGE("token is too long\n");
				close(fd);
				return -1;
			}
			strlcpy(temp[i], token, sizeof(temp[i]));
		}
		close(fd);
		for(int i = 0; i < sizeof(temp) / LENGTH; i++) {
			cal_result->offset[i] = strtol(temp[i], &endptr, 10);
			if (cal_result->offset[i] == LONG_MAX || cal_result->offset[i] == LONG_MIN) {
				ALOGE("cal_result->offset[%d] error value\n", i);
				return -1;
			}
			if (endptr == temp[i]) {
				ALOGE("No digits were found\n");
				return -1;
			}
		}
		return 0;
	} else {
		ALOGE("open %s error\n", input_sysfs_path);
		return -1;
	}
	return 0;
}

int AccelSensor::initCalibrate(int32_t handle, struct cal_result_t *cal_result)
{
	int fd, i, err;
	char buf[LENGTH];
	int arry[] = {CMD_W_OFFSET_X, CMD_W_OFFSET_Y, CMD_W_OFFSET_Z};
	if (cal_result == NULL) {
		ALOGE("Null pointer initcalibrate parameter\n");
		return -1;
	}
	strlcpy(&input_sysfs_path[input_sysfs_path_len],
			SYSFS_CALIBRATE, SYSFS_MAXLEN);
	fd = open(input_sysfs_path, O_RDWR);
	if (fd >= 0) {
		int para1 = 0;
		for(i = 0; i < sizeof(arry) / sizeof(int); ++i) {
			para1 = SET_CMD_H(cal_result->offset[i], arry[i]);
			snprintf(buf, sizeof(buf), "%d", para1);
			err = write(fd, buf, strlen(buf)+1);
			if(err < 0) {
				ALOGE("write error\n");
				close(fd);
				return err;
			}

			memset(buf, 0, sizeof(buf));
			para1 = SET_CMD_L(cal_result->offset[i], arry[i]);
			snprintf(buf, sizeof(buf), "%d", para1);
			err = write(fd, buf, strlen(buf)+1);
			if(err < 0) {
				ALOGE("write error\n");
				close(fd);
				return err;
			}
		}
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "%d", CMD_COMPLETE);
		err = write(fd, buf, strlen(buf)+1);
		if(err < 0) {
			ALOGE("write error\n");
			close(fd);
			return err;
		}
		close(fd);
		return 0;
	}
	ALOGE("open %s error\n", input_sysfs_path);
	return -1;
}
