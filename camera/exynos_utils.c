/*
 * Copyright (C) 2013 Paul Kocialkowski
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
#include <errno.h>
#include <malloc.h>
#include <ctype.h>

#define LOG_TAG "exynos_utils"
#include <utils/Log.h>

#include "exynos_camera.h"

int list_head_insert(struct list_head *list, struct list_head *prev,
	struct list_head *next)
{
	if (list == NULL)
		return -EINVAL;

	list->prev = prev;
	list->next = next;

	if(prev != NULL)
		prev->next = list;
	if(next != NULL)
		next->prev = list;

	return 0;
}

void list_head_remove(struct list_head *list)
{
	if(list == NULL)
		return;

	if(list->next != NULL)
		list->next->prev = list->prev;
	if(list->prev != NULL)
		list->prev->next = list->next;
}


int exynos_camera_buffer_length(int width, int height, int format)
{
	float bpp;
	int buffer_length;

	switch (format) {
		case V4L2_PIX_FMT_RGB32:
			bpp = 4.0f;
			buffer_length = (int) ((float) width * (float) height * bpp);
			break;
		case V4L2_PIX_FMT_RGB565:
		case V4L2_PIX_FMT_YUYV:
		case V4L2_PIX_FMT_UYVY:
		case V4L2_PIX_FMT_VYUY:
		case V4L2_PIX_FMT_YVYU:
		case V4L2_PIX_FMT_YUV422P:
		case V4L2_PIX_FMT_NV16:
		case V4L2_PIX_FMT_NV61:
			bpp = 2.0f;
			buffer_length = (int) ((float) width * (float) height * bpp);
			break;
		case V4L2_PIX_FMT_NV12:
		case V4L2_PIX_FMT_NV12T:
		case V4L2_PIX_FMT_YUV420:
		case V4L2_PIX_FMT_YVU420:
			bpp = 1.5f;
			buffer_length = EXYNOS_CAMERA_ALIGN(width * height);
			buffer_length += EXYNOS_CAMERA_ALIGN(width * height / 2);
			break;
		case V4L2_PIX_FMT_NV21:
			bpp = 1.5f;
			buffer_length = (int) ((float) width * (float) height * bpp);
			break;
		case V4L2_PIX_FMT_JPEG:
		case V4L2_PIX_FMT_INTERLEAVED:
		default:
			buffer_length = -1;
			bpp = 0;
			break;
	}

	return buffer_length;
}

void exynos_camera_yuv_planes(int width, int height, int format, int address, int *address_y, int *address_cb, int *address_cr)
{
	switch (format) {
		case V4L2_PIX_FMT_RGB32:
		case V4L2_PIX_FMT_RGB565:
		case V4L2_PIX_FMT_YUYV:
		case V4L2_PIX_FMT_UYVY:
		case V4L2_PIX_FMT_VYUY:
		case V4L2_PIX_FMT_YVYU:
			if (address_y != NULL)
				*address_y = address;
			break;
		case V4L2_PIX_FMT_YUV420:
			if (address_y != NULL)
				*address_y = address;

			address += EXYNOS_CAMERA_ALIGN(width * height);

			if (address_cb != NULL)
				*address_cb = address;

			address += EXYNOS_CAMERA_ALIGN(width * height / 4);

			if (address_cr != NULL)
				*address_cr = address;
			break;
		case V4L2_PIX_FMT_NV12:
		case V4L2_PIX_FMT_NV12T:
			if (address_y != NULL)
				*address_y = address;

			address += EXYNOS_CAMERA_ALIGN(width * height);

			if (address_cb != NULL)
				*address_cb = address;

			if (address_cr != NULL)
				*address_cr = address;
			break;
		case V4L2_PIX_FMT_NV21:
		case V4L2_PIX_FMT_NV16:
		case V4L2_PIX_FMT_NV61:
		default:
			if (address_y != NULL)
				*address_y = address;

			address += width * height;

			if (address_cb != NULL)
				*address_cb = address;

			if (address_cr != NULL)
				*address_cr = address;
			break;
	}
}
