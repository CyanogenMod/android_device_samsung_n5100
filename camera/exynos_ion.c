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

#include <linux/ion.h>

#define LOG_TAG "exynos_ion"
#include <utils/Log.h>

#include "exynos_camera.h"

int exynos_ion_init(struct exynos_camera *exynos_camera)
{
	exynos_camera->ion_fd = -1;

	return 0;
}

int exynos_ion_open(struct exynos_camera *exynos_camera)
{
	int fd;

	fd = open("/dev/ion", O_RDWR);
	if (fd < 0) {
		ALOGE("%s: Unable to open ion device", __func__);
		return -1;
	}

	exynos_camera->ion_fd = fd;

	return 0;
}

void exynos_ion_close(struct exynos_camera *exynos_camera)
{
	if (exynos_camera->ion_fd >= 0)
		close(exynos_camera->ion_fd);

	exynos_camera->ion_fd = -1;
}

int exynos_ion_alloc(struct exynos_camera *exynos_camera, int size)
{
	struct ion_allocation_data alloc_data;
	struct ion_fd_data share_data;
	struct ion_handle_data free_data;
	int page_size;
	int fd;
	int rc;

	page_size = getpagesize();

	fd = exynos_camera->ion_fd;
	if (fd < 0)
		return -1;

	memset(&alloc_data, 0, sizeof(alloc_data));
	alloc_data.len = size;
	alloc_data.align = page_size;
	alloc_data.flags = ION_HEAP_EXYNOS_CONTIG_MASK;

	rc = ioctl(fd, ION_IOC_ALLOC, &alloc_data);
	if (rc < 0)
		return -1;

	memset(&share_data, 0, sizeof(share_data));
	share_data.handle = alloc_data.handle;

	rc = ioctl(fd, ION_IOC_SHARE, &share_data);
	if (rc < 0)
		return -1;

	memset(&free_data, 0, sizeof(free_data));
	free_data.handle = alloc_data.handle;

	rc = ioctl(fd, ION_IOC_FREE, &free_data);
	if (rc < 0)
		return -1;

	return share_data.fd;
}

int exynos_ion_free(struct exynos_camera *exynos_camera, int fd)
{
	close(fd);
	return 0;
}

int exynos_ion_phys(struct exynos_camera *exynos_camera, int fd)
{
	struct ion_custom_data custom_data;
	struct ion_phys_data phys_data;
	int rc;

	memset(&phys_data, 0, sizeof(phys_data));
	phys_data.fd_buffer = fd;

	memset(&custom_data, 0, sizeof(custom_data));
	custom_data.cmd = ION_EXYNOS_CUSTOM_PHYS;
	custom_data.arg = (unsigned long) &phys_data;

	fd = exynos_camera->ion_fd;
	if (fd < 0)
		return -1;

	rc = ioctl(fd, ION_IOC_CUSTOM, &custom_data);
	if (rc < 0)
		return -1;

	return (int) phys_data.phys;
}

int exynos_ion_msync(struct exynos_camera *exynos_camera, int fd,
	int offset, int size)
{
	struct ion_custom_data custom_data;
	struct ion_msync_data msync_data;
	int rc;

	memset(&msync_data, 0, sizeof(msync_data));
	msync_data.dir = IMSYNC_SYNC_FOR_DEV | IMSYNC_DEV_TO_RW;
	msync_data.fd_buffer = fd;
	msync_data.offset = offset;
	msync_data.size = size;

	memset(&custom_data, 0, sizeof(custom_data));
	custom_data.cmd = ION_EXYNOS_CUSTOM_MSYNC;
	custom_data.arg = (unsigned long) &msync_data;

	fd = exynos_camera->ion_fd;
	if (fd < 0)
		return -1;

	rc = ioctl(fd, ION_IOC_CUSTOM, &custom_data);
	if (rc < 0)
		return -1;

	return 0;
}
