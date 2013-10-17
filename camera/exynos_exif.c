/*
 * Copyright (C) 2013 Paul Kocialkowski
 *
 * Based on crespo libcamera and exynos4 hal libcamera:
 * Copyright 2008, The Android Open Source Project
 * Copyright 2010, Samsung Electronics Co. LTD
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <Exif.h>

#define LOG_TAG "exynos_camera"
#include <utils/Log.h>
#include <cutils/properties.h>

#include "exynos_camera.h"

int exynos_exif_attributes_create_static(struct exynos_camera *exynos_camera,
	struct exynos_exif *exif)
{
	exif_attribute_t *attributes;
	unsigned char gps_version[] = { 0x02, 0x02, 0x00, 0x00 };
	char property[PROPERTY_VALUE_MAX];
	uint32_t av;

	if (exynos_camera == NULL || exif == NULL)
		return -EINVAL;

	attributes = &exif->attributes;

	// Device

	property_get("ro.product.brand", property, EXIF_DEF_MAKER);
	strncpy((char *) attributes->maker, property,
		sizeof(attributes->maker) - 1);
	attributes->maker[sizeof(attributes->maker) - 1] = '\0';

	property_get("ro.product.model", property, EXIF_DEF_MODEL);
	strncpy((char *) attributes->model, property,
		sizeof(attributes->model) - 1);
	attributes->model[sizeof(attributes->model) - 1] = '\0';

	property_get("ro.build.id", property, EXIF_DEF_SOFTWARE);
	strncpy((char *) attributes->software, property,
		sizeof(attributes->software) - 1);
	attributes->software[sizeof(attributes->software) - 1] = '\0';

	attributes->ycbcr_positioning = EXIF_DEF_YCBCR_POSITIONING;

	attributes->fnumber.num = EXIF_DEF_FNUMBER_NUM;
	attributes->fnumber.den = EXIF_DEF_FNUMBER_DEN;

	attributes->exposure_program = EXIF_DEF_EXPOSURE_PROGRAM;

	memcpy(attributes->exif_version, EXIF_DEF_EXIF_VERSION,
		sizeof(attributes->exif_version));

	av = APEX_FNUM_TO_APERTURE((double) attributes->fnumber.num /
		attributes->fnumber.den);
	attributes->aperture.num = av;
	attributes->aperture.den = EXIF_DEF_APEX_DEN;
	attributes->max_aperture.num = av;
	attributes->max_aperture.den = EXIF_DEF_APEX_DEN;

	strcpy((char *) attributes->user_comment, EXIF_DEF_USERCOMMENTS);
	attributes->color_space = EXIF_DEF_COLOR_SPACE;
	attributes->exposure_mode = EXIF_DEF_EXPOSURE_MODE;

	// GPS version

	memcpy(attributes->gps_version_id, gps_version, sizeof(gps_version));

	attributes->compression_scheme = EXIF_DEF_COMPRESSION;
	attributes->x_resolution.num = EXIF_DEF_RESOLUTION_NUM;
	attributes->x_resolution.den = EXIF_DEF_RESOLUTION_DEN;
	attributes->y_resolution.num = EXIF_DEF_RESOLUTION_NUM;
	attributes->y_resolution.den = EXIF_DEF_RESOLUTION_DEN;
	attributes->resolution_unit = EXIF_DEF_RESOLUTION_UNIT;

	return 0;
}

int exynos_exif_attributes_create_gps(struct exynos_camera *exynos_camera,
	struct exynos_exif *exif)
{
	exif_attribute_t *attributes;
	float gps_latitude_float, gps_longitude_float, gps_altitude_float;
	int gps_timestamp_int;
	char *gps_processing_method_string;
	long gps_latitude, gps_longitude;
	long gps_altitude, gps_timestamp;
	double gps_latitude_abs, gps_longitude_abs, gps_altitude_abs;

	struct tm time_info;

	if (exynos_camera == NULL || exif == NULL)
		return -EINVAL;

	attributes = &exif->attributes;

	gps_latitude_float = exynos_param_float_get(exynos_camera, "gps-latitude");
	gps_longitude_float = exynos_param_float_get(exynos_camera, "gps-longitude");
	gps_altitude_float = exynos_param_float_get(exynos_camera, "gps-altitude");
	if (gps_altitude_float == -1)
		gps_altitude_float = (float) exynos_param_int_get(exynos_camera, "gps-altitude");
	gps_timestamp_int = exynos_param_int_get(exynos_camera, "gps-timestamp");
	gps_processing_method_string = exynos_param_string_get(exynos_camera, "gps-processing-method");

	if (gps_latitude_float == -1 || gps_longitude_float == -1 ||
		gps_altitude_float == -1 || gps_timestamp_int <= 0 ||
		gps_processing_method_string == NULL) {
		attributes->enableGps = false;
		return 0;
	}

	gps_latitude = (long) (gps_latitude_float * 10000000) / 1;
	gps_longitude = (long) (gps_longitude_float * 10000000) / 1;
	gps_altitude = (long) (gps_altitude_float * 100) / 1;
	gps_timestamp = (long) gps_timestamp_int;

	if (gps_latitude == 0 || gps_longitude == 0) {
		attributes->enableGps = false;
		return 0;
	}

	if (gps_latitude > 0)
		strcpy((char *) attributes->gps_latitude_ref, "N");
	else
		strcpy((char *) attributes->gps_latitude_ref, "S");

	if (gps_longitude > 0)
		strcpy((char *) attributes->gps_longitude_ref, "E");
	else
		strcpy((char *) attributes->gps_longitude_ref, "W");

	if (gps_altitude > 0)
		attributes->gps_altitude_ref = 0;
	else
		attributes->gps_altitude_ref = 1;


	gps_latitude_abs = fabs(gps_latitude);
	gps_longitude_abs = fabs(gps_longitude);
	gps_altitude_abs = fabs(gps_altitude);

	attributes->gps_latitude[0].num = (uint32_t) gps_latitude_abs;
	attributes->gps_latitude[0].den = 10000000;
	attributes->gps_latitude[1].num = 0;
	attributes->gps_latitude[1].den = 1;
	attributes->gps_latitude[2].num = 0;
	attributes->gps_latitude[2].den = 1;

	attributes->gps_longitude[0].num = (uint32_t) gps_longitude_abs;
	attributes->gps_longitude[0].den = 10000000;
	attributes->gps_longitude[1].num = 0;
	attributes->gps_longitude[1].den = 1;
	attributes->gps_longitude[2].num = 0;
	attributes->gps_longitude[2].den = 1;

	attributes->gps_altitude.num = (uint32_t) gps_altitude_abs;
	attributes->gps_altitude.den = 100;

	gmtime_r(&gps_timestamp, &time_info);

	attributes->gps_timestamp[0].num = time_info.tm_hour;
	attributes->gps_timestamp[0].den = 1;
	attributes->gps_timestamp[1].num = time_info.tm_min;
	attributes->gps_timestamp[1].den = 1;
	attributes->gps_timestamp[2].num = time_info.tm_sec;
	attributes->gps_timestamp[2].den = 1;
	snprintf((char *) attributes->gps_datestamp, sizeof(attributes->gps_datestamp),
		"%04d:%02d:%02d", time_info.tm_year + 1900, time_info.tm_mon + 1, time_info.tm_mday);

	attributes->enableGps = true;

	return 0;
}

int exynos_exif_attributes_create_params(struct exynos_camera *exynos_camera,
	struct exynos_exif *exif)
{
	exif_attribute_t *attributes;
	uint32_t av, tv, bv, sv, ev;
	time_t time_data;
	struct tm *time_info;
	int rotation;
	int shutter_speed;
	int exposure_time;
	int iso_speed;
	int exposure;
	int flash_results;

	int rc;

	if (exynos_camera == NULL || exif == NULL)
		return -EINVAL;

	attributes = &exif->attributes;

	// Picture size

	attributes->width = exynos_camera->picture_width;
	attributes->height = exynos_camera->picture_height;

	// Thumbnail

	attributes->widthThumb = exynos_camera->jpeg_thumbnail_width;
	attributes->heightThumb = exynos_camera->jpeg_thumbnail_height;
	attributes->enableThumb = true;

	// Orientation

	rotation = exynos_param_int_get(exynos_camera, "rotation");
	switch (rotation) {
		case 90:
			attributes->orientation = EXIF_ORIENTATION_90;
			break;
		case 180:
			attributes->orientation = EXIF_ORIENTATION_180;
			break;
		case 270:
			attributes->orientation = EXIF_ORIENTATION_270;
			break;
		case 0:
		default:
			attributes->orientation = EXIF_ORIENTATION_UP;
			break;
	}

	// Time

	time(&time_data);
	time_info = localtime(&time_data);
	strftime((char *) attributes->date_time, sizeof(attributes->date_time),
		"%Y:%m:%d %H:%M:%S", time_info);

	attributes->focal_length.num = exynos_camera->camera_focal_length;
	attributes->focal_length.den = EXIF_DEF_FOCAL_LEN_DEN;

	shutter_speed = 100;
	rc = exynos_v4l2_g_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_EXIF_TV,
		&shutter_speed);
	if (rc < 0)
		ALOGE("%s: Unable to get shutter speed", __func__);

	attributes->shutter_speed.num = shutter_speed;
	attributes->shutter_speed.den = 100;

	attributes->exposure_time.num = 1;
	attributes->exposure_time.den = APEX_SHUTTER_TO_EXPOSURE(shutter_speed);

	rc = exynos_v4l2_g_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_EXIF_ISO,
		&iso_speed);
	if (rc < 0)
		ALOGE("%s: Unable to get iso", __func__);

	attributes->iso_speed_rating = iso_speed;

	rc = exynos_v4l2_g_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_EXIF_FLASH,
		&flash_results);
	if (rc < 0)
		ALOGE("%s: Unable to get flash", __func__);

	attributes->flash = flash_results;

	rc = exynos_v4l2_g_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_EXIF_BV,
		(int *) &bv);
	if (rc < 0) {
		ALOGE("%s: Unable to get bv", __func__);
		goto bv_static;
	}

	rc = exynos_v4l2_g_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_EXIF_EBV,
		(int *) &ev);
	if (rc < 0) {
		ALOGE("%s: Unable to get ebv", __func__);
		goto bv_static;
	}

	goto bv_ioctl;

bv_static:
	exposure = exynos_param_int_get(exynos_camera, "exposure-compensation");
	if (exposure < 0)
		exposure = EV_DEFAULT;

	av = APEX_FNUM_TO_APERTURE((double) attributes->fnumber.num /
		attributes->fnumber.den);
	tv = APEX_EXPOSURE_TO_SHUTTER((double) attributes->exposure_time.num /
		attributes->exposure_time.den);
	sv = APEX_ISO_TO_FILMSENSITIVITY(iso_speed);
	bv = av + tv - sv;
	ev = exposure - EV_DEFAULT;

bv_ioctl:
	attributes->brightness.num = bv;
	attributes->brightness.den = EXIF_DEF_APEX_DEN;

	if (exynos_camera->scene_mode == SCENE_MODE_BEACH_SNOW) {
		attributes->exposure_bias.num = EXIF_DEF_APEX_DEN;
		attributes->exposure_bias.den = EXIF_DEF_APEX_DEN;
	} else {
		attributes->exposure_bias.num = ev * EXIF_DEF_APEX_DEN;
		attributes->exposure_bias.den = EXIF_DEF_APEX_DEN;
	}

	switch (exynos_camera->camera_metering) {
		case METERING_CENTER:
			attributes->metering_mode = EXIF_METERING_CENTER;
			break;
		case METERING_MATRIX:
			attributes->metering_mode = EXIF_METERING_AVERAGE;
			break;
		case METERING_SPOT:
			attributes->metering_mode = EXIF_METERING_SPOT;
			break;
		default:
			attributes->metering_mode = EXIF_METERING_AVERAGE;
			break;
	}

	if (exynos_camera->whitebalance == WHITE_BALANCE_AUTO ||
		exynos_camera->whitebalance == WHITE_BALANCE_BASE)
		attributes->white_balance = EXIF_WB_AUTO;
	else
		attributes->white_balance = EXIF_WB_MANUAL;

	switch (exynos_camera->scene_mode) {
		case SCENE_MODE_PORTRAIT:
			attributes->scene_capture_type = EXIF_SCENE_PORTRAIT;
			break;
		case SCENE_MODE_LANDSCAPE:
			attributes->scene_capture_type = EXIF_SCENE_LANDSCAPE;
			break;
		case SCENE_MODE_NIGHTSHOT:
			attributes->scene_capture_type = EXIF_SCENE_NIGHT;
			break;
		default:
			attributes->scene_capture_type = EXIF_SCENE_STANDARD;
			break;
	}

	rc = exynos_exif_attributes_create_gps(exynos_camera, exif);
	if (rc < 0) {
		ALOGE("%s: Unable to create GPS attributes", __func__);
		return -1;
	}

	return 0;
}

int exynos_exif_write_data(void *exif_data, unsigned short tag,
	unsigned short type, unsigned int count, unsigned int *offset, void *start,
	void *data, int length)
{
	unsigned char *pointer;
	int size;

	if (exif_data == NULL || data == NULL || length <= 0)
		return -EINVAL;

	pointer = (unsigned char *) exif_data;

	memcpy(pointer, &tag, sizeof(tag));
	pointer += sizeof(tag);

	memcpy(pointer, &type, sizeof(type));
	pointer += sizeof(type);

	memcpy(pointer, &count, sizeof(count));
	pointer += sizeof(count);

	if (offset != NULL && start != NULL) {
		memcpy(pointer, offset, sizeof(*offset));
		pointer += sizeof(*offset);

		memcpy((void *) ((int) start + *offset), data, count * length);
		*offset += count * length;
	} else {
		memcpy(pointer, data, count * length);
		pointer += 4;
	}

	size = (int) pointer - (int) exif_data;
	return size;
}

int exynos_exif_start(struct exynos_camera *exynos_camera, struct exynos_exif *exif)
{
	int rc;

	if (exynos_camera == NULL || exif == NULL)
		return -EINVAL;

	ALOGD("%s()", __func__);

	if (exif->enabled) {
		ALOGE("Exif was already started");
		return -1;
	}

	rc = exynos_exif_attributes_create_static(exynos_camera, exif);
	if (rc < 0) {
		ALOGE("%s: Unable to create exif attributes", __func__);
		goto error;
	}

	rc = exynos_exif_attributes_create_params(exynos_camera, exif);
	if (rc < 0) {
		ALOGE("%s: Unable to create exif parameters", __func__);
		goto error;
	}

	exif->enabled = 1;

	rc = 0;
	goto complete;

error:
	rc = -1;

complete:
	return rc;
}

void exynos_exif_stop(struct exynos_camera *exynos_camera,
	struct exynos_exif *exif)
{
	if (exynos_camera == NULL || exif == NULL)
		return;

	ALOGD("%s()", __func__);

	if (!exif->enabled) {
		ALOGE("Exif was already stopped");
		return;
	}

	if (exif->memory != NULL && exif->memory->release != NULL) {
		exif->memory->release(exif->memory);
		exif->memory = NULL;
	}

	exif->enabled = 0;
}

int exynos_exif(struct exynos_camera *exynos_camera, struct exynos_exif *exif)
{
	// Markers
	unsigned char exif_app1_marker[] = { 0xff, 0xe1 };
	unsigned char exif_app1_size[] = { 0x00, 0x00 };
	unsigned char exif_marker[] = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
	unsigned char tiff_marker[] = { 0x49, 0x49, 0x2A, 0x00, 0x08, 0x00, 0x00, 0x00 };

	unsigned char user_comment_code[] = { 0x41, 0x53, 0x43, 0x49, 0x49, 0x0, 0x0, 0x0 };
	unsigned char exif_ascii_prefix[] = { 0x41, 0x53, 0x43, 0x49, 0x49, 0x0, 0x0, 0x0 };

	void *jpeg_thumbnail_data;
	int jpeg_thumbnail_size;
	camera_memory_t *memory = NULL;
	int memory_size;
	exif_attribute_t *attributes;
	void *exif_ifd_data_start = NULL;
	void *exif_ifd_start = NULL;
	void *exif_ifd_gps = NULL;
	void *exif_ifd_thumb = NULL;
	unsigned char *pointer;
	unsigned int offset;
	unsigned int value;
	void *data;
	int count;
	int rc;

	if (exynos_camera == NULL || exif == NULL)
		return -EINVAL;

	ALOGD("%s()", __func__);

	if (!exif->enabled) {
		ALOGE("Exif was not started");
		return -1;
	}

	jpeg_thumbnail_data = exif->jpeg_thumbnail_data;
	jpeg_thumbnail_size = exif->jpeg_thumbnail_size;

	if (jpeg_thumbnail_data == NULL || jpeg_thumbnail_size <= 0) {
		ALOGE("%s: Invalid jpeg thumbnail", __func__);
		goto error;
	}

	attributes = &exif->attributes;

	memory_size = EXIF_FILE_SIZE + jpeg_thumbnail_size;

	if (EXYNOS_CAMERA_CALLBACK_DEFINED(request_memory)) {
		memory = exynos_camera->callbacks.request_memory(-1, memory_size, 1, exynos_camera->callbacks.user);
		if (memory == NULL || memory->data == NULL || memory->data == MAP_FAILED) {
			ALOGE("%s: Unable to request memory", __func__);
			goto error;
		}
	} else {
		ALOGE("%s: No memory request function!", __func__);
		goto error;
	}

	memset(memory->data, 0, memory_size);

	pointer = (unsigned char *) memory->data;
	exif_ifd_data_start = (void *) pointer;

	// Skip 4 bytes for APP1 marker
	pointer += 4;

	// Copy EXIF marker
	memcpy(pointer, exif_marker, sizeof(exif_marker));
	pointer += sizeof(exif_marker);

	// Copy TIFF marker
	memcpy(pointer, tiff_marker, sizeof(tiff_marker));
	exif_ifd_start = (void *) pointer;
	pointer += sizeof(tiff_marker);

	if (attributes->enableGps)
		value = NUM_0TH_IFD_TIFF;
	else
		value = NUM_0TH_IFD_TIFF - 1;

	memcpy(pointer, &value, NUM_SIZE);
	pointer += NUM_SIZE;

	offset = 8 + NUM_SIZE + value * IFD_SIZE + OFFSET_SIZE;

	// Write EXIF data
	count = exynos_exif_write_data(pointer, EXIF_TAG_IMAGE_WIDTH,
		EXIF_TYPE_LONG, 1, NULL, NULL, &attributes->width, sizeof(attributes->width));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_IMAGE_HEIGHT,
		EXIF_TYPE_LONG, 1, NULL, NULL, &attributes->height, sizeof(attributes->height));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_MAKE,
		EXIF_TYPE_ASCII, strlen((char *) attributes->maker) + 1, &offset, exif_ifd_start, &attributes->maker, sizeof(char));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_MODEL,
		EXIF_TYPE_ASCII, strlen((char *) attributes->model) + 1, &offset, exif_ifd_start, &attributes->model, sizeof(char));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_ORIENTATION,
		EXIF_TYPE_SHORT, 1, NULL, NULL, &attributes->orientation, sizeof(attributes->orientation));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_SOFTWARE,
		EXIF_TYPE_ASCII, strlen((char *) attributes->software) + 1, &offset, exif_ifd_start, &attributes->software, sizeof(char));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_DATE_TIME,
		EXIF_TYPE_ASCII, 20, &offset, exif_ifd_start, &attributes->date_time, sizeof(char));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_YCBCR_POSITIONING,
		EXIF_TYPE_SHORT, 1, NULL, NULL, &attributes->ycbcr_positioning, sizeof(attributes->ycbcr_positioning));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_EXIF_IFD_POINTER,
		EXIF_TYPE_LONG, 1, NULL, NULL, &offset, sizeof(offset));
	pointer += count;

	if (attributes->enableGps) {
		exif_ifd_gps = (void *) pointer;
		pointer += IFD_SIZE;
	}

	exif_ifd_thumb = (void *) pointer;
	pointer += OFFSET_SIZE;

	pointer = (unsigned char *) exif_ifd_start;
	pointer += offset;

	value = NUM_0TH_IFD_EXIF;
	memcpy(pointer, &value, NUM_SIZE);
	pointer += NUM_SIZE;

	offset += NUM_SIZE + NUM_0TH_IFD_EXIF * IFD_SIZE + OFFSET_SIZE;

	count = exynos_exif_write_data(pointer, EXIF_TAG_EXPOSURE_TIME,
		EXIF_TYPE_RATIONAL, 1, &offset, exif_ifd_start, &attributes->exposure_time, sizeof(attributes->exposure_time));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_FNUMBER,
		EXIF_TYPE_RATIONAL, 1, &offset, exif_ifd_start, &attributes->fnumber, sizeof(attributes->fnumber));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_EXPOSURE_PROGRAM,
		EXIF_TYPE_SHORT, 1, NULL, NULL, &attributes->exposure_program, sizeof(attributes->exposure_program));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_ISO_SPEED_RATING,
		EXIF_TYPE_SHORT, 1, NULL, NULL, &attributes->iso_speed_rating, sizeof(attributes->iso_speed_rating));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_EXIF_VERSION,
		EXIF_TYPE_UNDEFINED, 4, NULL, NULL, &attributes->exif_version, sizeof(char));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_DATE_TIME_ORG,
		EXIF_TYPE_ASCII, 20, &offset, exif_ifd_start, &attributes->date_time, sizeof(char));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_DATE_TIME_DIGITIZE,
		EXIF_TYPE_ASCII, 20, &offset, exif_ifd_start, &attributes->date_time, sizeof(char));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_SHUTTER_SPEED,
		EXIF_TYPE_SRATIONAL, 1, &offset, exif_ifd_start, &attributes->shutter_speed, sizeof(attributes->shutter_speed));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_APERTURE,
		EXIF_TYPE_RATIONAL, 1, &offset, exif_ifd_start, &attributes->aperture, sizeof(attributes->aperture));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_BRIGHTNESS,
		EXIF_TYPE_SRATIONAL, 1, &offset, exif_ifd_start, &attributes->brightness, sizeof(attributes->brightness));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_EXPOSURE_BIAS,
		EXIF_TYPE_SRATIONAL, 1, &offset, exif_ifd_start, &attributes->exposure_bias, sizeof(attributes->exposure_bias));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_MAX_APERTURE,
		EXIF_TYPE_RATIONAL, 1, &offset, exif_ifd_start, &attributes->max_aperture, sizeof(attributes->max_aperture));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_METERING_MODE,
		EXIF_TYPE_SHORT, 1, NULL, NULL, &attributes->metering_mode, sizeof(attributes->metering_mode));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_FLASH,
		EXIF_TYPE_SHORT, 1, NULL, NULL, &attributes->flash, sizeof(attributes->flash));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_FOCAL_LENGTH,
		EXIF_TYPE_RATIONAL, 1, &offset, exif_ifd_start, &attributes->focal_length, sizeof(attributes->focal_length));
	pointer += count;

	value = strlen((char *) attributes->user_comment) + 1;
	memmove(attributes->user_comment + sizeof(user_comment_code), attributes->user_comment, value);
	memcpy(attributes->user_comment, user_comment_code, sizeof(user_comment_code));

	count = exynos_exif_write_data(pointer, EXIF_TAG_USER_COMMENT,
		EXIF_TYPE_UNDEFINED, value + sizeof(user_comment_code), &offset, exif_ifd_start, &attributes->user_comment, sizeof(char));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_COLOR_SPACE,
		EXIF_TYPE_SHORT, 1, NULL, NULL, &attributes->color_space, sizeof(attributes->color_space));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_PIXEL_X_DIMENSION,
		EXIF_TYPE_LONG, 1, NULL, NULL, &attributes->width, sizeof(attributes->width));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_PIXEL_Y_DIMENSION,
		EXIF_TYPE_LONG, 1, NULL, NULL, &attributes->height, sizeof(attributes->height));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_EXPOSURE_MODE,
		EXIF_TYPE_SHORT, 1, NULL, NULL, &attributes->exposure_mode, sizeof(attributes->exposure_mode));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_WHITE_BALANCE,
		EXIF_TYPE_SHORT, 1, NULL, NULL, &attributes->white_balance, sizeof(attributes->white_balance));
	pointer += count;

	count = exynos_exif_write_data(pointer, EXIF_TAG_SCENCE_CAPTURE_TYPE,
		EXIF_TYPE_SHORT, 1, NULL, NULL, &attributes->scene_capture_type, sizeof(attributes->scene_capture_type));
	pointer += count;

	value = 0;
	memcpy(pointer, &value, OFFSET_SIZE);
	pointer += OFFSET_SIZE;

	// GPS
	if (attributes->enableGps) {
		pointer = (unsigned char *) exif_ifd_gps;
		count = exynos_exif_write_data(pointer, EXIF_TAG_GPS_IFD_POINTER,
			EXIF_TYPE_LONG, 1, NULL, NULL, &offset, sizeof(offset));

		pointer = (unsigned char *) exif_ifd_start + offset;

		if (attributes->gps_processing_method[0] == 0)
			value = NUM_0TH_IFD_GPS - 1;
		else
			value = NUM_0TH_IFD_GPS;

		memcpy(pointer, &value, NUM_SIZE);
		pointer += NUM_SIZE;

		offset += NUM_SIZE + value * IFD_SIZE + OFFSET_SIZE;

		count = exynos_exif_write_data(pointer, EXIF_TAG_GPS_VERSION_ID,
			EXIF_TYPE_BYTE, 4, NULL, NULL, &attributes->gps_version_id, sizeof(char));
		pointer += count;

		count = exynos_exif_write_data(pointer, EXIF_TAG_GPS_LATITUDE_REF,
			EXIF_TYPE_ASCII, 2, NULL, NULL, &attributes->gps_latitude_ref, sizeof(char));
		pointer += count;

		count = exynos_exif_write_data(pointer, EXIF_TAG_GPS_LATITUDE,
			EXIF_TYPE_RATIONAL, 3, &offset, exif_ifd_start, &attributes->gps_latitude, sizeof(attributes->gps_latitude[0]));
		pointer += count;

		count = exynos_exif_write_data(pointer, EXIF_TAG_GPS_LONGITUDE_REF,
			EXIF_TYPE_ASCII, 2, NULL, NULL, &attributes->gps_longitude_ref, sizeof(char));
		pointer += count;

		count = exynos_exif_write_data(pointer, EXIF_TAG_GPS_LONGITUDE,
			EXIF_TYPE_RATIONAL, 3, &offset, exif_ifd_start, &attributes->gps_longitude, sizeof(attributes->gps_longitude[0]));
		pointer += count;

		count = exynos_exif_write_data(pointer, EXIF_TAG_GPS_ALTITUDE_REF,
			EXIF_TYPE_BYTE, 1, NULL, NULL, &attributes->gps_altitude_ref, sizeof(char));
		pointer += count;

		count = exynos_exif_write_data(pointer, EXIF_TAG_GPS_ALTITUDE,
			EXIF_TYPE_RATIONAL, 1, &offset, exif_ifd_start, &attributes->gps_altitude, sizeof(attributes->gps_altitude));
		pointer += count;

		count = exynos_exif_write_data(pointer, EXIF_TAG_GPS_TIMESTAMP,
			EXIF_TYPE_RATIONAL, 3, &offset, exif_ifd_start, &attributes->gps_timestamp, sizeof(attributes->gps_timestamp[0]));
		pointer += count;

		value = strlen((char *) attributes->gps_processing_method);
		if (value > 0) {
			value = value > 100 ? 100 : value;

			data = calloc(1, value + sizeof(exif_ascii_prefix));
			memcpy(data, &exif_ascii_prefix, sizeof(exif_ascii_prefix));
			memcpy((void *) ((int) data + (int) sizeof(exif_ascii_prefix)), attributes->gps_processing_method, value);

			count = exynos_exif_write_data(pointer, EXIF_TAG_GPS_PROCESSING_METHOD,
				EXIF_TYPE_UNDEFINED, value + sizeof(exif_ascii_prefix), &offset, exif_ifd_start, data, sizeof(char));
			pointer += count;

			free(data);
		}

		count = exynos_exif_write_data(pointer, EXIF_TAG_GPS_DATESTAMP,
				EXIF_TYPE_ASCII, 11, &offset, exif_ifd_start, &attributes->gps_datestamp, 1);
		pointer += count;

		value = 0;
		memcpy(pointer, &value, OFFSET_SIZE);
		pointer += OFFSET_SIZE;
	}

	if (attributes->enableThumb) {
		value = offset;
		memcpy(exif_ifd_thumb, &value, OFFSET_SIZE);

		pointer = (unsigned char *) ((int) exif_ifd_start + (int) offset);

		value = NUM_1TH_IFD_TIFF;
		memcpy(pointer, &value, NUM_SIZE);
		pointer += NUM_SIZE;

		offset += NUM_SIZE + NUM_1TH_IFD_TIFF * IFD_SIZE + OFFSET_SIZE;

		count = exynos_exif_write_data(pointer, EXIF_TAG_IMAGE_WIDTH,
				EXIF_TYPE_LONG, 1, NULL, NULL, &attributes->widthThumb, sizeof(attributes->widthThumb));
		pointer += count;

		count = exynos_exif_write_data(pointer, EXIF_TAG_IMAGE_HEIGHT,
				EXIF_TYPE_LONG, 1, NULL, NULL, &attributes->heightThumb, sizeof(attributes->heightThumb));
		pointer += count;

		count = exynos_exif_write_data(pointer, EXIF_TAG_COMPRESSION_SCHEME,
				EXIF_TYPE_SHORT, 1, NULL, NULL, &attributes->compression_scheme, sizeof(attributes->compression_scheme));
		pointer += count;

		count = exynos_exif_write_data(pointer, EXIF_TAG_ORIENTATION,
				EXIF_TYPE_SHORT, 1, NULL, NULL, &attributes->orientation, sizeof(attributes->orientation));
		pointer += count;

		count = exynos_exif_write_data(pointer, EXIF_TAG_X_RESOLUTION,
				EXIF_TYPE_RATIONAL, 1, &offset, exif_ifd_start, &attributes->x_resolution, sizeof(attributes->x_resolution));
		pointer += count;

		count = exynos_exif_write_data(pointer, EXIF_TAG_Y_RESOLUTION,
				EXIF_TYPE_RATIONAL, 1, &offset, exif_ifd_start, &attributes->y_resolution, sizeof(attributes->y_resolution));
		pointer += count;

		count = exynos_exif_write_data(pointer, EXIF_TAG_RESOLUTION_UNIT,
				EXIF_TYPE_SHORT, 1, NULL, NULL, &attributes->resolution_unit, sizeof(attributes->resolution_unit));
		pointer += count;

		count = exynos_exif_write_data(pointer, EXIF_TAG_JPEG_INTERCHANGE_FORMAT,
				EXIF_TYPE_LONG, 1, NULL, NULL, &offset, sizeof(offset));
		pointer += count;

		value = (unsigned int) jpeg_thumbnail_size;

		count = exynos_exif_write_data(pointer, EXIF_TAG_JPEG_INTERCHANGE_FORMAT_LEN,
				EXIF_TYPE_LONG, 1, NULL, NULL, &value, sizeof(value));
		pointer += count;

		value = 0;
		memcpy(pointer, &value, OFFSET_SIZE);

		pointer = (unsigned char *) ((int) exif_ifd_start + (int) offset);

		memcpy(pointer, jpeg_thumbnail_data, jpeg_thumbnail_size);
		offset += jpeg_thumbnail_size;
	} else {
		value = 0;
		memcpy(exif_ifd_thumb, &value, OFFSET_SIZE);

	}

	pointer = (unsigned char *) exif_ifd_data_start;

	memcpy(pointer, exif_app1_marker, sizeof(exif_app1_marker));
	pointer += sizeof(exif_app1_marker);

	memory_size = offset + 10;
	value = memory_size - 2;
	exif_app1_size[0] = (value >> 8) & 0xff;
	exif_app1_size[1] = value & 0xff;

	memcpy(pointer, exif_app1_size, sizeof(exif_app1_size));

	exif->memory = memory;
	exif->memory_size = memory_size;

	rc = 0;
	goto complete;

error:
	if (memory != NULL && memory->release != NULL) {
		memory->release(memory);
		exif->memory = NULL;
	}

	rc = -1;

complete:
	return rc;
}
