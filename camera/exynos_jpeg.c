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

#define LOG_TAG "exynos_jpeg"
#include <utils/Log.h>

#include "exynos_camera.h"

#ifdef EXYNOS_JPEG_HW
int exynos_jpeg_start(struct exynos_camera *exynos_camera,
	struct exynos_jpeg *jpeg)
{
	struct jpeg_config config;
	struct jpeg_buf *buffer_in;
	struct jpeg_buf *buffer_out;
	camera_memory_t *memory = NULL;
#ifdef EXYNOS_ION
	int memory_ion_fd = -1;
#endif
	int address;
	int fd = -1;
	int rc;

	if (exynos_camera == NULL || jpeg == NULL)
		return -EINVAL;

	ALOGD("%s()", __func__);

	if (jpeg->enabled) {
		ALOGE("Jpeg was already started");
		return -1;
	}

	buffer_in = &jpeg->buffer_in;
	buffer_out = &jpeg->buffer_out;

#ifdef EXYNOS_ION
	jpeg->memory_in_ion_fd = -1;
	jpeg->memory_out_ion_fd = -1;
#endif

	fd = jpeghal_enc_init();
	if (fd < 0) {
		ALOGE("%s: Unable to init jpeg encoder", __func__);
		goto error;
	}

	jpeg->fd = fd;

	memset(&config, 0, sizeof(config));
	config.mode = JPEG_ENCODE;
	config.width = jpeg->width;
	config.height = jpeg->height;
	config.num_planes = 1;
	config.pix.enc_fmt.in_fmt = jpeg->format;
	config.pix.enc_fmt.out_fmt = V4L2_PIX_FMT_JPEG_420;

	if (jpeg->quality >= 90)
		config.enc_qual = QUALITY_LEVEL_1;
	else if (jpeg->quality >= 80)
		config.enc_qual = QUALITY_LEVEL_2;
	else if (jpeg->quality >= 70)
		config.enc_qual = QUALITY_LEVEL_3;
	else
		config.enc_qual = QUALITY_LEVEL_4;

	rc = jpeghal_enc_setconfig(fd, &config);
	if (rc < 0) {
		ALOGE("%s: Unable to set jpeg config", __func__);
		goto error;
	}

	rc = jpeghal_s_ctrl(fd, V4L2_CID_CACHEABLE, 1);
	if (rc < 0) {
		ALOGE("%s: Unable to set cacheable control", __func__);
		goto error;
	}

	memset(buffer_in, 0, sizeof(struct jpeg_buf));
	buffer_in->memory = V4L2_MEMORY_MMAP;
	buffer_in->num_planes = 1;

	memset(buffer_out, 0, sizeof(struct jpeg_buf));
	buffer_out->memory = V4L2_MEMORY_MMAP;
	buffer_out->num_planes = 1;

	rc = jpeghal_set_inbuf(fd, buffer_in);
	if (rc < 0) {
#ifdef EXYNOS_ION
		// Input

		buffer_in->memory = V4L2_MEMORY_USERPTR;
		buffer_in->length[0] = exynos_camera_buffer_length(jpeg->width, jpeg->height, jpeg->format);

		memory_ion_fd = exynos_ion_alloc(exynos_camera, buffer_in->length[0]);
		if (memory_ion_fd < 0) {
			ALOGE("%s: Unable to alloc input ION memory", __func__);
			goto error;
		}

		address = exynos_ion_phys(exynos_camera, memory_ion_fd);

		if (EXYNOS_CAMERA_CALLBACK_DEFINED(request_memory)) {
			memory = exynos_camera->callbacks.request_memory(memory_ion_fd, buffer_in->length[0], 1, exynos_camera->callbacks.user);
			if (memory == NULL || memory->data == NULL || memory->data == MAP_FAILED) {
				ALOGE("%s: Unable to request memory", __func__);
				goto error;
			}
		} else {
			ALOGE("%s: No memory request function!", __func__);
			goto error;
		}

		jpeg->memory_in = memory;
		jpeg->memory_in_pointer = memory->data;
		jpeg->memory_in_ion_fd = memory_ion_fd;
		buffer_in->start[0] = (void *) address;

		rc = jpeghal_set_inbuf(fd, buffer_in);
		if (rc < 0) {
			ALOGE("%s: Unable to set input buffer", __func__);
			goto error;
		}

		// Output

		buffer_out->memory = V4L2_MEMORY_USERPTR;
		buffer_out->length[0] = jpeg->width * jpeg->height * 4;

		memory_ion_fd = exynos_ion_alloc(exynos_camera, buffer_out->length[0]);
		if (memory_ion_fd < 0) {
			ALOGE("%s: Unable to alloc output ION memory", __func__);
			goto error;
		}

		address = exynos_ion_phys(exynos_camera, memory_ion_fd);

		if (EXYNOS_CAMERA_CALLBACK_DEFINED(request_memory)) {
			memory = exynos_camera->callbacks.request_memory(memory_ion_fd, buffer_out->length[0], 1, exynos_camera->callbacks.user);
			if (memory == NULL || memory->data == NULL || memory->data == MAP_FAILED) {
				ALOGE("%s: Unable to request memory", __func__);
				goto error;
			}
		} else {
			ALOGE("%s: No memory request function!", __func__);
			goto error;
		}

		jpeg->memory_out = memory;
		jpeg->memory_out_pointer = memory->data;
		jpeg->memory_out_ion_fd = memory_ion_fd;
		buffer_out->start[0] = (void *) address;
#else
		ALOGE("%s: Unable to set input buffer", __func__);
		goto error;
#endif
	} else {
		jpeg->memory_in_pointer = buffer_in->start[0];
		jpeg->memory_out_pointer = buffer_out->start[0];
	}

	rc = jpeghal_set_outbuf(fd, buffer_out);
	if (rc < 0) {
		ALOGE("%s: Unable to set output buffer", __func__);
		goto error;
	}

	jpeg->enabled = 1;

	rc = 0;
	goto complete;

error:
	if (fd >= 0) {
		// Avoid releasing unrequested mmap buffers

		if (buffer_in->memory == 0)
			buffer_in->memory = V4L2_MEMORY_USERPTR;

		if (buffer_out->memory == 0)
			buffer_out->memory = V4L2_MEMORY_USERPTR;

		jpeghal_deinit(fd, buffer_in, buffer_out);
		jpeg->fd = -1;
	}

	if (jpeg->memory_in != NULL && jpeg->memory_in->release != NULL) {
		jpeg->memory_in->release(jpeg->memory_in);
		jpeg->memory_in = NULL;
#ifdef EXYNOS_ION
		if (jpeg->memory_in_ion_fd >= 0) {
			exynos_ion_free(exynos_camera, jpeg->memory_in_ion_fd);
			jpeg->memory_in_ion_fd = -1;
		}
#endif
	}

	if (jpeg->memory_out != NULL && jpeg->memory_out->release != NULL) {
		jpeg->memory_out->release(jpeg->memory_out);
		jpeg->memory_out = NULL;

#ifdef EXYNOS_ION
		if (jpeg->memory_out_ion_fd >= 0) {
			exynos_ion_free(exynos_camera, jpeg->memory_out_ion_fd);
			jpeg->memory_out_ion_fd = -1;
		}
#endif
	}

	rc = -1;

complete:
	return rc;
}

void exynos_jpeg_stop(struct exynos_camera *exynos_camera,
	struct exynos_jpeg *jpeg)
{
	struct jpeg_buf *buffer_in;
	struct jpeg_buf *buffer_out;
	int fd = -1;
	int rc;

	if (exynos_camera == NULL || jpeg == NULL)
		return;

	ALOGD("%s()", __func__);

	if (!jpeg->enabled) {
		ALOGE("Jpeg was already stopped");
		return;
	}

	buffer_in = &jpeg->buffer_in;
	buffer_out = &jpeg->buffer_out;

	fd = jpeg->fd;

	if (fd >= 0) {
		jpeghal_deinit(fd, buffer_in, buffer_out);
		jpeg->fd = -1;
	}

	if (jpeg->memory_in != NULL && jpeg->memory_in->release != NULL) {
		jpeg->memory_in->release(jpeg->memory_in);
		jpeg->memory_in = NULL;
#ifdef EXYNOS_ION
		if (jpeg->memory_in_ion_fd >= 0) {
			exynos_ion_free(exynos_camera, jpeg->memory_in_ion_fd);
			jpeg->memory_in_ion_fd = -1;
		}
#endif
	}

	if (jpeg->memory_out != NULL && jpeg->memory_out->release != NULL) {
		jpeg->memory_out->release(jpeg->memory_out);
		jpeg->memory_out = NULL;

#ifdef EXYNOS_ION
		if (jpeg->memory_out_ion_fd >= 0) {
			exynos_ion_free(exynos_camera, jpeg->memory_out_ion_fd);
			jpeg->memory_out_ion_fd = -1;
		}
#endif
	}

	jpeg->enabled = 0;
}

int exynos_jpeg(struct exynos_camera *exynos_camera, struct exynos_jpeg *jpeg)
{
	struct jpeg_buf *buffer_in;
	struct jpeg_buf *buffer_out;
	int memory_size;
	int fd = -1;
	int rc;

	if (exynos_camera == NULL || jpeg == NULL)
		return -EINVAL;

	ALOGD("%s()", __func__);

	if (!jpeg->enabled) {
		ALOGE("Jpeg was not started");
		return -1;
	}

	buffer_in = &jpeg->buffer_in;
	buffer_out = &jpeg->buffer_out;

	fd = jpeg->fd;
	if (fd < 0) {
		ALOGE("%s: Invalid jpeg fd", __func__);
		goto error;
	}

#ifdef EXYNOS_ION
	if (jpeg->memory_in != NULL && jpeg->memory_in_ion_fd >= 0) {
		rc = exynos_ion_msync(exynos_camera, jpeg->memory_in_ion_fd, 0, buffer_in->length[0]);
		if (rc < 0) {
			ALOGE("%s: Unable to sync ION memory", __func__);
			goto error;
		}
	}
#endif

	rc = jpeghal_enc_exe(fd, buffer_in, buffer_out);
	if (rc < 0) {
		ALOGE("%s: Unable to encode jpeg", __func__);
		goto error;
	}

	memory_size = jpeghal_g_ctrl(fd, V4L2_CID_CAM_JPEG_ENCODEDSIZE);
	if (memory_size <= 0) {
		ALOGE("%s: Unable to get jpeg size", __func__);
		goto error;
	}

	jpeg->memory_out_size = memory_size;

#ifdef EXYNOS_ION
	if (jpeg->memory_out != NULL && jpeg->memory_out_ion_fd >= 0) {
		rc = exynos_ion_msync(exynos_camera, jpeg->memory_out_ion_fd, 0, memory_size);
		if (rc < 0) {
			ALOGE("%s: Unable to sync ION memory", __func__);
			goto error;
		}
	}
#endif

	rc = 0;
	goto complete;

error:
	if (fd >= 0) {
		// Avoid releasing unrequested mmap buffers

		if (buffer_in->memory == 0)
			buffer_in->memory = V4L2_MEMORY_USERPTR;

		if (buffer_out->memory == 0)
			buffer_out->memory = V4L2_MEMORY_USERPTR;

		jpeghal_deinit(fd, buffer_in, buffer_out);
		jpeg->fd = -1;
	}

	if (jpeg->memory_in != NULL && jpeg->memory_in->release != NULL) {
		jpeg->memory_in->release(jpeg->memory_in);
		jpeg->memory_in = NULL;

#ifdef EXYNOS_ION
		if (jpeg->memory_in_ion_fd >= 0) {
			exynos_ion_free(exynos_camera, jpeg->memory_in_ion_fd);
			jpeg->memory_in_ion_fd = -1;
		}
#endif
	}

	if (jpeg->memory_out != NULL && jpeg->memory_out->release != NULL) {
		jpeg->memory_out->release(jpeg->memory_out);
		jpeg->memory_out = NULL;

#ifdef EXYNOS_ION
		if (jpeg->memory_out_ion_fd >= 0) {
			exynos_ion_free(exynos_camera, jpeg->memory_out_ion_fd);
			jpeg->memory_out_ion_fd = -1;
		}
#endif
	}

	rc = -1;

complete:
	return rc;
}
#endif
