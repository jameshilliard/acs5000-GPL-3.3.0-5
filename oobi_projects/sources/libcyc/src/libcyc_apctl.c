/* -*- linux-c -*- */
/*****************************************************************
 * File: libcyc_apctl.c
 *
 * Copyright (C) 2005-2006 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * Description: libcyc_apctl
 *
 *****************************************************************
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#include <cyc.h>
#include <cy_apctl.h>

#define SENSOR_PATH "/proc/sys/dev/sensors"
#define FAN1_FILENAME "fan1"
#define FAN2_FILENAME "fan2"
#define FAN3_FILENAME "fan3"
#define TEMP_FPGA_FILENAME "temp1"
#define TEMP_PWRSUP_FILENAME "temp2"
#define TEMP_CPU_FILENAME "temp3"

enum dev_led_ioctls {
	DEV_LED_HAS_DUAL_PS,
	DEV_LED_PS_STATUS,
	DEV_LED_BUZZER_OFF,
	DEV_LED_BUZZER_ON,
};

static int parse_lm85_fan_speed(struct cyc_info *, enum cyc_fanid);
static char *get_lm85_dirname(struct cyc_info *, const char *);

static char *get_lm85_dirname(struct cyc_info *cyc, const char *sensor_path)
{
	struct dirent *lm85_dir;
	DIR *dir = NULL;
	char *dirname = NULL;
	char debug_str[MAX_STR_LEN];

	snprintf(debug_str, MAX_STR_LEN,
		 "%s sensor_path: %s", __func__, sensor_path);
	libcyc_debug(cyc, debug_str);

	if ((dir=opendir(sensor_path)) == NULL) {
		perror("get_lm85_dirname");
		return(NULL);
	}

	while ((lm85_dir=readdir(dir))) {
		if (strncmp(lm85_dir->d_name,"lm85",4) == 0) {
			dirname=strdup(lm85_dir->d_name);
			break;
		}
	}

	closedir(dir);

	if (dirname) return(dirname);

	return(NULL);
}

static int parse_lm85_fan_speed(struct cyc_info *cyc, enum cyc_fanid fan)
{
	FILE *fp;
	char fan_filepath[MAX_STR_LEN];
	char line[MAX_STR_LEN];
	int val1, val2;
	char *lm85_dirname = NULL;
	char debug_str[MAX_STR_LEN];

	if ((lm85_dirname = get_lm85_dirname(cyc, SENSOR_PATH)) == NULL) {
		return(ERROR);
	} 

	snprintf(debug_str, MAX_STR_LEN,
		 "%s fan: %d lm85_dirname: %s",
		 __func__, fan, lm85_dirname);
	libcyc_debug(cyc, debug_str);

	switch(fan) {
	case FAN_1:
		snprintf(fan_filepath, MAX_STR_LEN, "%s/%s/%s",
			 SENSOR_PATH, lm85_dirname, FAN1_FILENAME);
		break;
	case FAN_2:
		snprintf(fan_filepath, MAX_STR_LEN, "%s/%s/%s",
			SENSOR_PATH, lm85_dirname, FAN2_FILENAME);
		break;
	case FAN_3:
		snprintf(fan_filepath, MAX_STR_LEN, "%s/%s/%s",
			SENSOR_PATH, lm85_dirname, FAN3_FILENAME);
		break;
	default:
		return(ERROR);
	}

	free(lm85_dirname);
	lm85_dirname=NULL;
                                
        if ((fp=fopen(fan_filepath,"r")) == NULL) {
                perror("parse_conntrack: fopen");
		snprintf(debug_str, MAX_STR_LEN,
			 "%s open of %s failed",
			 __func__, fan_filepath);
		libcyc_debug(cyc, debug_str);
                return(ERROR);
        }

	fgets(line, MAX_STR_LEN, fp);
	fclose(fp);

	snprintf(debug_str, MAX_STR_LEN,
		 "%s got line: %s from %s",
		 __func__, line, fan_filepath);
	libcyc_debug(cyc, debug_str);

	sscanf(line,"%d %d", &val1, &val2);
	
	return(val2);
}

int libcyc_apctl_get_fan_speed(struct cyc_info *cyc, enum cyc_fanid fan)
{
        int fd;
        int fan_spd=ERROR;
	char debug_str[MAX_STR_LEN];
	int model = ERROR;

        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	snprintf(debug_str, MAX_STR_LEN, "%s fan: %d",
		 __func__, fan);
	libcyc_debug(cyc, debug_str);

	model = libcyc_kvm_get_model(cyc);

        if ((model == MODEL_KVMNET) ||
	    (model == MODEL_KVM)) {
                int r_fan_spd = ERROR;
        
                if ((fd = libcyc_open_dev(cyc, APCTL_DEVFILE, O_RDONLY)) == -1) {
                        return(ERROR);
                }
                
                ioctl(fd, CYAPCTLGETFANSPD, &r_fan_spd);
		libcyc_close_dev(cyc, fd);
                
                switch(fan) {
                case FAN_1:
                        fan_spd = (r_fan_spd % 0x10) * 1000;
                        break;
                case FAN_2:
                        fan_spd = (r_fan_spd / 0x10) * 1000;
                        break;
                case FAN_3:
                        fan_spd = ERROR;
                        break;
                default:
                        fan_spd = ERROR;
                }
        } else if ((model == MODEL_KVMNET_PLUS) ||
		   (model == MODEL_ONSITE)) {
		fan_spd=parse_lm85_fan_speed(cyc, fan);
        }
        
        return(fan_spd);
}

int libcyc_apctl_set_alarm(struct cyc_info *cyc, int alarm_state)
{
        int fd;
	char debug_str[MAX_STR_LEN];
        int model = ERROR;
	int errors = 0;

        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	snprintf(debug_str, MAX_STR_LEN, "%s alarm state: %d",
		__func__, alarm_state);
	libcyc_debug(cyc, debug_str);

	model = libcyc_kvm_get_model(cyc);
        
        if ((model == MODEL_KVMNET) ||
            (model == MODEL_KVM) ||
	    (model == MODEL_ONSITE) ||
	    (model == MODEL_KVMNET_PLUS)) {
                if ((fd = libcyc_open_dev(cyc, APCTL_DEVFILE, O_WRONLY)) == -1) {
                        return(ERROR);
                }

                ioctl(fd, CYAPCTLSETALRM, &alarm_state);
                libcyc_close_dev(cyc, fd);
	} else if (libcyc_prod_is_cs(cyc) == TRUE) {
		int buzzer = 1;

		if ((fd = libcyc_open_dev(cyc, LED_DEVFILE, O_RDONLY)) == -1) {
			return(ERROR);
		}
		
		if (alarm_state) {
			if (ioctl(fd, DEV_LED_BUZZER_ON, &buzzer)) {
				errors++;
				goto error;
			}
		} else {
			if (ioctl(fd, DEV_LED_BUZZER_OFF, &buzzer)) {
				errors++;
				goto error;
			}
		}
		
	error:
		libcyc_close_dev(cyc, fd);
		if (errors) return(ERROR);
	} else {
		return(ERROR);
	}

	return(0);
}

int libcyc_apctl_get_alarm(struct cyc_info *cyc)
{
        int fd;
	int alarm_state=0;
	int model = ERROR;

        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	model = libcyc_kvm_get_model(cyc);

        if ((model == MODEL_KVMNET) ||
            (model == MODEL_KVM) ||
            (model == MODEL_ONSITE) ||
            (model == MODEL_KVMNET_PLUS)) {
                if ((fd = libcyc_open_dev(cyc, APCTL_DEVFILE, O_RDONLY)) == -1) {
                        return(ERROR);
                }

                ioctl(fd, CYAPCTLGETALRM, &alarm_state);
                libcyc_close_dev(cyc, fd);
        }

	if (alarm_state) return(ON);

        return(OFF);
}

int libcyc_apctl_set_gpled(struct cyc_info *cyc, int led_state)
{
	int fd;
	int model = ERROR;
        
        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	model = libcyc_kvm_get_model(cyc);

        if ((model == MODEL_KVMNET) ||
            (model == MODEL_KVM) ||
            (model == MODEL_ONSITE) ||
            (model == MODEL_KVMNET_PLUS)) {
                if ((fd = libcyc_open_dev(cyc, APCTL_DEVFILE, O_WRONLY)) == -1) {
                        return(ERROR);
                }

                ioctl(fd, CYAPCTLSETGPLED, &led_state);
                libcyc_close_dev(cyc, fd);
	} else {
		return(ERROR);
	}

	return(0);
}

int libcyc_apctl_get_gpled(struct cyc_info *cyc)
{
        int fd;
	int led_state=0;
	int model = ERROR;

        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);
	
	model = libcyc_kvm_get_model(cyc);

        if ((model == MODEL_KVMNET) ||
            (model == MODEL_KVM) ||
            (model == MODEL_ONSITE) ||
            (model == MODEL_KVMNET_PLUS)) {
                if ((fd = libcyc_open_dev(cyc, APCTL_DEVFILE, O_RDONLY)) == -1) {
                        return(ERROR);
                }

                ioctl(fd, CYAPCTLGETGPLED, &led_state);
                libcyc_close_dev(cyc, fd);
        }

	if (led_state) return(ON);

        return(OFF);
}

int libcyc_apctl_set_hdleden(struct cyc_info *cyc, int led_state)
{
	int fd;
        int model = ERROR;

        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	model = libcyc_kvm_get_model(cyc);

        if ((model == MODEL_KVMNET) ||
            (model == MODEL_KVM) ||
	    (model == MODEL_ONSITE)) {
                if ((fd = libcyc_open_dev(cyc, APCTL_DEVFILE, O_WRONLY)) == -1) {
                        return(ERROR);
                }

                ioctl(fd, CYAPCTLSETHDLEDEN, &led_state);
                libcyc_close_dev(cyc, fd);
	} else {
		return(ERROR);
	}

	return(0);
}

int libcyc_apctl_get_hdleden(struct cyc_info *cyc)
{
        int fd;
	int led_state=0;
	int model = ERROR;

        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	model = libcyc_kvm_get_model(cyc);

        if ((model == MODEL_KVMNET) ||
            (model == MODEL_KVM) ||
            (model == MODEL_ONSITE)) {
                if ((fd = libcyc_open_dev(cyc, APCTL_DEVFILE, O_RDONLY)) == -1) {
                        return(ERROR);
                }

                ioctl(fd, CYAPCTLGETHDLEDEN, &led_state);
                libcyc_close_dev(cyc, fd);
        } else if (model == MODEL_KVMNET_PLUS) {
		/* This is always enabled for KVM/netPlus */
		return(ON);
	}

	if (led_state) return(ON);

        return(OFF);
}

int libcyc_apctl_set_cpuledorngen(struct cyc_info *cyc, int led_state)
{
	int fd;
	int model = ERROR;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	model = libcyc_kvm_get_model(cyc);

        if ((model == MODEL_KVMNET) ||
            (model == MODEL_KVM) ||
	    (model == MODEL_ONSITE)) {
                if ((fd = libcyc_open_dev(cyc, APCTL_DEVFILE, O_WRONLY)) == -1) {
                        return(ERROR);
                }

                ioctl(fd, CYAPCTLSETCPULEDORNGEN, &led_state);
                libcyc_close_dev(cyc, fd);
	} else {
		return(ERROR);
	}

	return(0);
}

int libcyc_apctl_get_cpuledorngen(struct cyc_info *cyc)
{
        int fd;
	int led_state=0;
	int model = ERROR;

        if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	model = libcyc_kvm_get_model(cyc);

        if ((model == MODEL_KVMNET) ||
            (model == MODEL_KVM) ||
            (model == MODEL_ONSITE)) {
                if ((fd = libcyc_open_dev(cyc, APCTL_DEVFILE, O_RDONLY)) == -1) {
                        return(ERROR);
                }

                ioctl(fd, CYAPCTLGETCPULEDORNGEN, &led_state);
                libcyc_close_dev(cyc, fd);
	} else if (model == MODEL_KVMNET_PLUS) {
		/* This is always enabled for KVM/netPlus */
		return(ON);
	}

	if (led_state) return(ON);

        return(OFF);
}

int libcyc_apctl_get_kvm_hwinfo(struct cyc_info *cyc,
				struct kvm_hwinfo *hwinfo)
{
	int fd;
	int model = ERROR;
	int errors = 0;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);
	if (hwinfo == NULL) return(ERROR);

	model = libcyc_kvm_get_model(cyc);

	if ((fd = libcyc_open_dev(cyc, APCTL_DEVFILE, O_RDONLY)) == -1) {
		return(ERROR);
	}

	if (ioctl(fd, CYAPCTLGETSYSFPGAVER, &hwinfo->sysfpga_ver)) {
		errors++;
		goto error;
	}
	
	if (model != MODEL_KVMNET_PLUS ||
	    ((model == MODEL_ONSITE) && (cyc->prod.board_version != 2))) {
		if (ioctl(fd, CYAPCTLGETMUXFPGAVER, &hwinfo->muxfpga_ver)) {
			errors++;
			goto error;
		}
	} else {
		hwinfo->muxfpga_ver = ERROR;
	}

 error:
	libcyc_close_dev(cyc, fd);
	if (errors) return(ERROR);

	return(0);
}

int libcyc_apctl_get_dual_power_cap(struct cyc_info *cyc)
{
	int fd;
	int errors = 0;
	int dual_ps = 0;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);
	if (libcyc_prod_is_cs(cyc) == FALSE) return(ERROR);

	if ((fd = libcyc_open_dev(cyc, LED_DEVFILE, O_RDONLY)) == -1) {
		return(ERROR);
	}

	if (ioctl(fd, DEV_LED_HAS_DUAL_PS, &dual_ps)) {
		errors++;
		goto error;
	}

 error:
	libcyc_close_dev(cyc, fd);
	if (errors) return(ERROR);
	
	return(dual_ps ? TRUE : FALSE);
}

int libcyc_apctl_get_dual_power_stat(struct cyc_info *cyc)
{
	int fd;
	int ps_stat = 0;
	int kps_stat = 0;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);
	if (libcyc_prod_is_cs(cyc) == FALSE) return(ERROR);

	if ((fd = libcyc_open_dev(cyc, LED_DEVFILE, O_RDONLY)) == -1) {
		return(ERROR);
	}

	if (ioctl(fd, DEV_LED_PS_STATUS, &kps_stat)) goto error;
	
	if (kps_stat & 0x1) {
		ps_stat |= PS_1;
	}
	if (kps_stat & 0x2) {
		ps_stat |= PS_2;
	}

 error:
	libcyc_close_dev(cyc, fd);

	return(ps_stat);
}
