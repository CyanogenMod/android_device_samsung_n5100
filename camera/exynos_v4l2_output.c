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
#include <errno.h>
#include <malloc.h>
#include <poll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>

#define LOG_TAG "exynos_v4l2_output"
#include <utils/Log.h>

#include "exynos_camera.h"

int exynos_v4l2_output_start(struct exynos_camera *exynos_camera,
	struct exynos_v4l2_output *output)
{
	int width, height, format;
	int buffer_width, buffer_height, buffer_format;
	camera_memory_t *memory = NULL;
	int memory_address, memory_size;
#ifdef EXYNOS_ION
	int memory_ion_fd = -1;
#endif
	int buffers_count, buffer_length;
	int v4l2_id;
	int value;
	int fd;
	int rc;
	int i;

	if (exynos_camera == NULL || output == NULL)
		return -EINVAL;

	ALOGD("%s()", __func__);

	if (output->enabled) {
		ALOGE("Output was already started");
		return -1;
	}

	width = output->width;
	height = output->height;
	format = output->format;

	buffer_width = output->buffer_width;
	buffer_height = output->buffer_height;
	buffer_format = output->buffer_format;

	v4l2_id = output->v4l2_id;

	buffers_count = output->buffers_count;
	if (buffers_count <= 0) {
		ALOGE("%s: Invalid buffers count: %d", __func__, buffers_count);
		goto error;
	}

	buffer_length = exynos_camera_buffer_length(width, height, format);

	rc = exynos_v4l2_open(exynos_camera, v4l2_id);
	if (rc < 0) {
		ALOGE("%s: Unable to open v4l2 device", __func__);
		goto error;
	}

	rc = exynos_v4l2_querycap_out(exynos_camera, v4l2_id);
	if (rc < 0) {
		ALOGE("%s: Unable to query capabilities", __func__);
		goto error;
	}

	rc = exynos_v4l2_g_fmt_out(exynos_camera, v4l2_id, NULL, NULL, NULL);
	if (rc < 0) {
		ALOGE("%s: Unable to get format", __func__);
		goto error;
	}

	value = 0;
	rc = exynos_v4l2_g_ctrl(exynos_camera, v4l2_id, V4L2_CID_RESERVED_MEM_BASE_ADDR, &value);
	if (rc < 0) {
		ALOGE("%s: Unable to get address", __func__);
		goto error;
	}

	memory_address = value;

	value = 0;
	rc = exynos_v4l2_g_ctrl(exynos_camera, v4l2_id, V4L2_CID_RESERVED_MEM_SIZE, &value);
	if (rc < 0) {
		ALOGE("%s: Unable to get size", __func__);
		goto error;
	}

	memory_size = value * 1024;
	
	value = 0;
	rc = exynos_v4l2_g_ctrl(exynos_camera, v4l2_id, V4L2_CID_FIMC_VERSION, &value);
	if (rc < 0) {
		ALOGE("%s: Unable to get fimc version", __func__);
		goto error;
	}

	rc = exynos_v4l2_s_ctrl(exynos_camera, v4l2_id, V4L2_CID_OVLY_MODE, FIMC_OVLY_NONE_MULTI_BUF);
	if (rc < 0) {
		ALOGE("%s: Unable to set overlay mode", __func__);
		goto error;
	}

	rc = exynos_v4l2_s_fmt_pix_out(exynos_camera, v4l2_id, buffer_width, buffer_height, buffer_format, 0);
	if (rc < 0) {
		ALOGE("%s: Unable to set output pixel format!", __func__);
		goto error;
	}

	rc = exynos_v4l2_s_crop_out(exynos_camera, v4l2_id, 0, 0, buffer_width, buffer_height);
	if (rc < 0) {
		ALOGE("%s: Unable to crop", __func__);
		goto error;
	}

	rc = exynos_v4l2_reqbufs_out(exynos_camera, v4l2_id, 1);
	if (rc < 0) {
		ALOGE("%s: Unable to request buffers", __func__);
		goto error;
	}

	if (memory_address != 0 && memory_address != (int) 0xffffffff && memory_size >= buffer_length) {
		for (i = buffers_count; i > 0; i--) {
			if (buffer_length * i < memory_size)
				break;
		}

		// This should never happen
		if (i == 0)
			goto error;

		buffers_count = i;
		ALOGD("Found %d buffers available for output!", buffers_count);

		if (EXYNOS_CAMERA_CALLBACK_DEFINED(request_memory)) {
			fd = exynos_v4l2_fd(exynos_camera, v4l2_id);
			if (fd < 0) {
				ALOGE("%s: Unable to get v4l2 fd for id %d", __func__, v4l2_id);
				goto error;
			}

			memory = exynos_camera->callbacks.request_memory(fd, buffer_length, buffers_count, exynos_camera->callbacks.user);
			if (memory == NULL || memory->data == NULL || memory->data == MAP_FAILED) {
				ALOGE("%s: Unable to request memory", __func__);
				goto error;
			}
		} else {
			ALOGE("%s: No memory request function!", __func__);
			goto error;
		}
	} else {
#ifdef EXYNOS_ION
		memory_ion_fd = exynos_ion_alloc(exynos_camera, buffers_count * buffer_length);
		if (memory_ion_fd < 0) {
			ALOGE("%s: Unable to alloc ION memory", __func__);
			goto error;
		}

		if (EXYNOS_CAMERA_CALLBACK_DEFINED(request_memory)) {
			memory = exynos_camera->callbacks.request_memory(memory_ion_fd, buffer_length, buffers_count, exynos_camera->callbacks.user);
			if (memory == NULL || memory->data == NULL || memory->data == MAP_FAILED) {
				ALOGE("%s: Unable to request memory", __func__);
				goto error;
			}
		} else {
			ALOGE("%s: No memory request function!", __func__);
			goto error;
		}

		memory_address = exynos_ion_phys(exynos_camera, memory_ion_fd);
#else
		ALOGE("%s: Unable to find memory", __func__);
		goto error;
#endif
	}

	output->memory = memory;
	output->memory_address = memory_address;
#ifdef EXYNOS_ION
	output->memory_ion_fd = memory_ion_fd;
#endif
	output->memory_index = 0;
	output->buffers_count = buffers_count;
	output->buffer_length = buffer_length;

	output->enabled = 1;

	rc = 0;
	goto complete;

error:
	if (memory != NULL && memory->release != NULL) {
		memory->release(memory);
		output->memory = NULL;
	}

#ifdef EXYNOS_ION
	if (memory_ion_fd >= 0)
		exynos_ion_free(exynos_camera, memory_ion_fd);
#endif

	exynos_v4l2_close(exynos_camera, v4l2_id);

	rc = -1;

complete:
	return rc;
}

void exynos_v4l2_output_stop(struct exynos_camera *exynos_camera,
	struct exynos_v4l2_output *output)
{
	int v4l2_id;
	int rc;

	if (exynos_camera == NULL || output == NULL)
		return;

//	ALOGD("%s()", __func__);

	if (!output->enabled) {
		ALOGE("Output was already stopped");
		return;
	}

	v4l2_id = output->v4l2_id;

	rc = exynos_v4l2_reqbufs_out(exynos_camera, v4l2_id, 0);
	if (rc < 0)
		ALOGE("%s: Unable to request buffers", __func__);

	if (output->memory != NULL && output->memory->release != NULL) {
		output->memory->release(output->memory);
		output->memory = NULL;
	}

#ifdef EXYNOS_ION
	if (output->memory_ion_fd >= 0) {
		exynos_ion_free(exynos_camera, output->memory_ion_fd);
		output->memory_ion_fd = -1;
	}
#endif

	exynos_v4l2_close(exynos_camera, v4l2_id);

	output->enabled = 0;
}

int exynos_v4l2_output(struct exynos_camera *exynos_camera,
	struct exynos_v4l2_output *output, int buffer_address)
{
	struct fimc_buf fimc_buffer;
	void *fb_base;
	int width, height, format;
	int buffer_width, buffer_height, buffer_format;
	int buffer_length;
	int address;
	int v4l2_id;
	int rc;

	if (exynos_camera == NULL || output == NULL)
		return -EINVAL;

//	ALOGD("%s()", __func__);

	if (!output->enabled) {
		ALOGE("Output was not started");
		return -1;
	}

	width = output->width;
	height = output->height;
	format = output->format;

	buffer_width = output->buffer_width;
	buffer_height = output->buffer_height;
	buffer_format = output->buffer_format;

	buffer_length = output->buffer_length;
	v4l2_id = output->v4l2_id;

	rc = exynos_v4l2_g_fbuf(exynos_camera, v4l2_id, &fb_base, NULL, NULL, NULL);
	if (rc < 0) {
		ALOGE("%s: Unable to get fbuf", __func__);
		goto error;
	}

	rc = exynos_v4l2_s_fbuf(exynos_camera, v4l2_id, fb_base, width, height, format);
	if (rc < 0) {
		ALOGE("%s: Unable to set fbuf", __func__);
		goto error;
	}

	memset(&fimc_buffer, 0, sizeof(fimc_buffer));

	address = output->memory_address + buffer_length * output->memory_index;

	exynos_camera_yuv_planes(width, height, format, address, (int *) &fimc_buffer.base[0], (int *) &fimc_buffer.base[1], (int *) &fimc_buffer.base[2]);

	rc = exynos_v4l2_s_ctrl(exynos_camera, v4l2_id, V4L2_CID_DST_INFO, (int) &fimc_buffer);
	if (rc < 0) {
		ALOGE("%s: Unable to set dst info", __func__);
		goto error;
	}

	rc = exynos_v4l2_s_fmt_win(exynos_camera, v4l2_id, 0, 0, width, height);
	if (rc < 0) {
		ALOGE("%s: Unable to set overlay win", __func__);
		goto error;
	}

	rc = exynos_v4l2_streamon_out(exynos_camera, v4l2_id);
	if (rc < 0) {
		ALOGE("%s: Unable to start stream", __func__);
		goto error;
	}

	memset(&fimc_buffer, 0, sizeof(fimc_buffer));

	exynos_camera_yuv_planes(buffer_width, buffer_height, buffer_format, buffer_address, (int *) &fimc_buffer.base[0], (int *) &fimc_buffer.base[1], (int *) &fimc_buffer.base[2]);

	rc = exynos_v4l2_qbuf_out(exynos_camera, v4l2_id, 0, (unsigned long) &fimc_buffer);
	if (rc < 0) {
		ALOGE("%s: Unable to queue buffer", __func__);
		goto error;
	}

	rc = exynos_v4l2_dqbuf_out(exynos_camera, v4l2_id);
	if (rc < 0) {
		ALOGE("%s: Unable to dequeue buffer", __func__);
		goto error;
	}

	rc = exynos_v4l2_streamoff_out(exynos_camera, v4l2_id);
	if (rc < 0) {
		ALOGE("%s: Unable to stop stream", __func__);
		goto error;
	}

	rc = 0;
	goto complete;

error:
	rc = -1;

complete:
	return rc;
}

int exynos_v4l2_output_release(struct exynos_camera *exynos_camera,
	struct exynos_v4l2_output *output)
{
	int buffers_count;
	int memory_index;

	if (exynos_camera == NULL || output == NULL)
		return -EINVAL;

//	ALOGD("%s()", __func__);

	buffers_count = output->buffers_count;
	memory_index = output->memory_index;

	memory_index++;
	output->memory_index = memory_index % buffers_count;

	return 0;
}
