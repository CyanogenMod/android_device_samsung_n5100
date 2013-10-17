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

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include <linux/videodev2.h>
#include <linux/videodev2_exynos_camera.h>
#include <linux/videodev2_exynos_media.h>
#include <linux/fimc.h>

#ifdef EXYNOS_JPEG_HW
#include <jpeg_hal.h>
#endif
#include <Exif.h>

#include <hardware/hardware.h>
#include <hardware/camera.h>

#ifndef _EXYNOS_CAMERA_H_
#define _EXYNOS_CAMERA_H_

#define EXYNOS_CAMERA_MAX_V4L2_NODES_COUNT	4

#define EXYNOS_CAMERA_CAPTURE_BUFFERS_COUNT	1
#define EXYNOS_CAMERA_PREVIEW_BUFFERS_COUNT	8
#define EXYNOS_CAMERA_RECORDING_BUFFERS_COUNT	6
#define EXYNOS_CAMERA_GRALLOC_BUFFERS_COUNT	3

#define EXYNOS_CAMERA_UNKNOWN_CAPTURE_MODE 167774080
#define ISX012_AUTO_FOCUS_IN_PROGRESS 0x8

#define EXYNOS_CAMERA_PICTURE_OUTPUT_FORMAT	V4L2_PIX_FMT_YUYV

#define EXYNOS_CAMERA_MSG_ENABLED(msg) (exynos_camera->messages_enabled & msg)
#define EXYNOS_CAMERA_CALLBACK_DEFINED(cb) (exynos_camera->callbacks.cb != NULL)

#define EXYNOS_CAMERA_ALIGN(value) ((value + (0x10000 - 1)) & ~(0x10000 - 1))

/*
 * Structures
 */

struct exynos_camera;

struct list_head {
	struct list_head *next;
	struct list_head *prev;
};

enum exynos_param_type {
	EXYNOS_PARAM_INT,
	EXYNOS_PARAM_FLOAT,
	EXYNOS_PARAM_STRING,
};

union exynos_param_data {
	int integer;
	float floating;
	char *string;
};

struct exynos_param {
	struct list_head list;

	char *key;
	union exynos_param_data data;
	enum exynos_param_type type;
};

struct exynos_camera_buffer {
	void *pointer;
	int address;
	int length;

	int width;
	int height;
	int format;
};

struct exynos_camera_capture_listener {
	struct list_head list;

	int width;
	int height;
	int format;

	int (*callback)(struct exynos_camera *exynos_camera, struct exynos_camera_buffer *buffers, int buffers_count);
	int busy;
};

struct exynos_camera_mbus_resolution {
	int width;
	int height;
	int mbus_width;
	int mbus_height;
};

struct exynos_camera_params {
	char *preview_size_values;
	char *preview_size;
	char *preview_format_values;
	char *preview_format;
	char *preview_frame_rate_values;
	int preview_frame_rate;
	char *preview_fps_range_values;
	char *preview_fps_range;

	char *picture_size_values;
	char *picture_size;
	char *picture_format_values;
	char *picture_format;
	char *jpeg_thumbnail_size_values;
	int jpeg_thumbnail_width;
	int jpeg_thumbnail_height;
	int jpeg_thumbnail_quality;
	int jpeg_quality;

	int video_snapshot_supported;
	int full_video_snap_supported;

	char *recording_size;
	char *recording_size_values;
	char *recording_format;

	char *focus_mode;
	char *focus_mode_values;
	char *focus_distances;
	char *focus_areas;
	int max_num_focus_areas;

	int zoom_supported;
	int smooth_zoom_supported;
	char *zoom_ratios;
	int zoom;
	int max_zoom;

	char *flash_mode;
	char *flash_mode_values;

	int exposure_compensation;
	float exposure_compensation_step;
	int min_exposure_compensation;
	int max_exposure_compensation;

	char *whitebalance;
	char *whitebalance_values;

	char *antibanding;
	char *antibanding_values;

	char *scene_mode;
	char *scene_mode_values;

	char *effect;
	char *effect_values;

	char *iso;
	char *iso_values;
};

struct exynos_camera_preset {
	char *name;
	int facing;
	int orientation;

	int rotation;
	int hflip;
	int vflip;

	int capture_format;
	int picture_format;
	int fimc_is;

	float focal_length;
	float horizontal_view_angle;
	float vertical_view_angle;

	int metering;

	struct exynos_camera_params params;
	struct exynos_camera_mbus_resolution *mbus_resolutions;
	int mbus_resolutions_count;
};

struct exynos_v4l2_node {
	int id;
	char *node;
};

struct exynos_v4l2_output {
	int enabled;

	int v4l2_id;

	int width;
	int height;
	int format;

	int buffer_width;
	int buffer_height;
	int buffer_format;

	camera_memory_t *memory;
	int memory_address;
#ifdef EXYNOS_ION
	int memory_ion_fd;
#endif
	int memory_index;
	int buffers_count;
	int buffer_length;
};

struct exynos_exif {
	int enabled;

	exif_attribute_t attributes;
	void *jpeg_thumbnail_data;
	int jpeg_thumbnail_size;

	camera_memory_t *memory;
	int memory_size;
};

#ifdef EXYNOS_JPEG_HW
struct exynos_jpeg {
	int enabled;

	int fd;
	struct jpeg_buf buffer_in;
	struct jpeg_buf buffer_out;
	camera_memory_t *memory_in;
	void *memory_in_pointer;
#ifdef EXYNOS_ION
	int memory_in_ion_fd;
#endif
	camera_memory_t *memory_out;
	void *memory_out_pointer;
	int memory_out_size;
#ifdef EXYNOS_ION
	int memory_out_ion_fd;
#endif

	int width;
	int height;
	int format;

	int quality;
};
#endif

struct exynox_camera_config {
	struct exynos_camera_preset *presets;
	int presets_count;

	struct exynos_v4l2_node *v4l2_nodes;
	int v4l2_nodes_count;
};

struct exynos_camera_callbacks {
	camera_notify_callback notify;
	camera_data_callback data;
	camera_data_timestamp_callback data_timestamp;
	camera_request_memory request_memory;
	void *user;
};

struct exynos_camera {
	int v4l2_fds[EXYNOS_CAMERA_MAX_V4L2_NODES_COUNT];
	int ion_fd;

	struct exynox_camera_config *config;
	struct exynos_param *params;

	struct exynos_camera_callbacks callbacks;
	int callback_lock;
	int messages_enabled;

	gralloc_module_t *gralloc;

	// Capture

	pthread_t capture_thread;
	pthread_mutex_t capture_mutex;
	pthread_mutex_t capture_lock_mutex;
	int capture_thread_running;
	int capture_thread_enabled;

	int capture_enabled;
	struct exynos_camera_capture_listener *capture_listeners;
	camera_memory_t *capture_memory;
	int capture_memory_address;
	int capture_memory_index;
	void *capture_yuv_buffer;
	void *capture_jpeg_buffer;
	int capture_auto_focus_result;
	int capture_hybrid;
	int capture_width;
	int capture_height;
	int capture_format;
	int capture_buffers_count;
	int capture_buffer_length;

	// Preview

	pthread_t preview_thread;
	pthread_mutex_t preview_mutex;
	pthread_mutex_t preview_lock_mutex;
	int preview_thread_running;
	int preview_thread_enabled;

	int preview_output_enabled;
	struct exynos_camera_capture_listener *preview_listener;
	struct preview_stream_ops *preview_window;
	struct exynos_camera_buffer preview_buffer;
	struct exynos_v4l2_output preview_output;

	// Picture

	pthread_t picture_thread;
	pthread_mutex_t picture_mutex;
	pthread_mutex_t picture_lock_mutex;
	int picture_thread_running;
	int picture_thread_enabled;

	int picture_enabled;
	int picture_completed;
	struct exynos_camera_capture_listener *picture_listener;
	camera_memory_t *picture_memory;
	struct exynos_camera_buffer picture_jpeg_buffer;
	struct exynos_camera_buffer picture_jpeg_thumbnail_buffer;
	struct exynos_camera_buffer picture_yuv_buffer;
	struct exynos_camera_buffer picture_yuv_thumbnail_buffer;

	// Recording

	pthread_t recording_thread;
	pthread_mutex_t recording_mutex;
	pthread_mutex_t recording_lock_mutex;
	int recording_thread_running;
	int recording_thread_enabled;

	int recording_output_enabled;
	struct exynos_camera_capture_listener *recording_listener;
	camera_memory_t *recording_memory;
	int recording_memory_index;
	struct exynos_camera_buffer recording_buffer;
	struct exynos_v4l2_output recording_output;
	int recording_buffers_count;
	int recording_buffer_length;
	int recording_metadata;

	// Auto-focus

	pthread_t auto_focus_thread;
	pthread_mutex_t auto_focus_mutex;
	int auto_focus_thread_enabled;
	int auto_focus_thread_running;

	// Camera params

	int camera_rotation;
	int camera_hflip;
	int camera_vflip;
	int camera_capture_format;
	int camera_picture_format;
	int camera_fimc_is;
	int camera_focal_length;
	int camera_metering;

	struct exynos_camera_mbus_resolution *camera_mbus_resolutions;
	int camera_mbus_resolutions_count;

	int camera_sensor_mode;
	int fimc_is_mode;

	// Params

	int preview_width;
	int preview_height;
	int preview_format;
	int preview_fps;
	int picture_width;
	int picture_height;
	int picture_format;
	int jpeg_thumbnail_width;
	int jpeg_thumbnail_height;
	int jpeg_thumbnail_quality;
	int jpeg_quality;
	int recording_width;
	int recording_height;
	int recording_format;
	int focus_mode;
	int focus_x;
	int focus_y;
	int zoom;
	int flash_mode;
	int exposure_compensation;
	int whitebalance;
	int antibanding;
	int scene_mode;
	int effect;
	int iso;
	int metering;
};

struct exynos_camera_addrs {
	unsigned int type;
	unsigned int y;
	unsigned int cbcr;
	unsigned int index;
	unsigned int reserved;
};

// This is because the linux header uses anonymous union
struct exynos_v4l2_ext_control {
	__u32 id;
	__u32 size;
	__u32 reserved2[1];
	union {
		__s32 value;
		__s64 value64;
		char *string;
	} data;
} __attribute__ ((packed));

/*
 * Camera
 */

// Camera
int exynos_camera_start(struct exynos_camera *exynos_camera, int id);
void exynos_camera_stop(struct exynos_camera *exynos_camera);

// Params
int exynos_camera_params_init(struct exynos_camera *exynos_camera, int id);
int exynos_camera_params_apply(struct exynos_camera *exynos_camera, int force);

// Capture
int exynos_camera_capture(struct exynos_camera *exynos_camera);
int exynos_camera_capture_thread_start(struct exynos_camera *exynos_camera);
void exynos_camera_capture_thread_stop(struct exynos_camera *exynos_camera);
int exynos_camera_capture_start(struct exynos_camera *exynos_camera, int is_capture);
void exynos_camera_capture_stop(struct exynos_camera *exynos_camera);
int exynos_camera_capture_setup(struct exynos_camera *exynos_camera);
struct exynos_camera_capture_listener *exynos_camera_capture_listener_register(
	struct exynos_camera *exynos_camera, int width, int height, int format,
	int (*callback)(struct exynos_camera *exynos_camera, struct exynos_camera_buffer *buffers, int buffers_count));
void exynos_camera_capture_listener_unregister(
	struct exynos_camera *exynos_camera,
	struct exynos_camera_capture_listener *listener);

// Preview
int exynos_camera_preview_output_start(struct exynos_camera *exynos_camera);
void exynos_camera_preview_output_stop(struct exynos_camera *exynos_camera);
int exynos_camera_preview_callback(struct exynos_camera *exynos_camera,
	struct exynos_camera_buffer *buffers, int buffers_count);
int exynos_camera_preview(struct exynos_camera *exynos_camera);
int exynos_camera_preview_thread_start(struct exynos_camera *exynos_camera);
void exynos_camera_preview_thread_stop(struct exynos_camera *exynos_camera);

// Picture
int exynos_camera_picture_callback(struct exynos_camera *exynos_camera,
	struct exynos_camera_buffer *buffers, int buffers_count);
int exynos_camera_picture(struct exynos_camera *exynos_camera);
int exynos_camera_picture_thread_start(struct exynos_camera *exynos_camera);
void exynos_camera_picture_thread_stop(struct exynos_camera *exynos_camera);

// Recording
int exynos_camera_recording_output_start(struct exynos_camera *exynos_camera);
void exynos_camera_recording_output_stop(struct exynos_camera *exynos_camera);
int exynos_camera_recording_callback(struct exynos_camera *exynos_camera,
	struct exynos_camera_buffer *buffers, int buffers_count);
void exynos_camera_recording_frame_release(struct exynos_camera *exynos_camera);
int exynos_camera_recording(struct exynos_camera *exynos_camera);
int exynos_camera_recording_thread_start(struct exynos_camera *exynos_camera);
void exynos_camera_recording_thread_stop(struct exynos_camera *exynos_camera);

// Auto-focus
int exynos_camera_auto_focus(struct exynos_camera *exynos_camera, int auto_focus_status);
int exynos_camera_auto_focus_thread_start(struct exynos_camera *exynos_camera);
void exynos_camera_auto_focus_thread_stop(struct exynos_camera *exynos_camera);

/*
 * EXIF
 */

int exynos_exif_start(struct exynos_camera *exynos_camera, struct exynos_exif *exif);
void exynos_exif_stop(struct exynos_camera *exynos_camera,
	struct exynos_exif *exif);
int exynos_exif(struct exynos_camera *exynos_camera, struct exynos_exif *exif);

/*
 * ION
 */

#ifdef EXYNOS_ION
int exynos_ion_init(struct exynos_camera *exynos_camera);
int exynos_ion_open(struct exynos_camera *exynos_camera);
void exynos_ion_close(struct exynos_camera *exynos_camera);
int exynos_ion_alloc(struct exynos_camera *exynos_camera, int size);
int exynos_ion_free(struct exynos_camera *exynos_camera, int fd);
int exynos_ion_phys(struct exynos_camera *exynos_camera, int fd);
int exynos_ion_msync(struct exynos_camera *exynos_camera, int fd,
	int offset, int size);
#endif

/*
 * Jpeg
 */

int exynos_jpeg_start(struct exynos_camera *exynos_camera,
	struct exynos_jpeg *jpeg);
void exynos_jpeg_stop(struct exynos_camera *exynos_camera,
	struct exynos_jpeg *jpeg);
int exynos_jpeg(struct exynos_camera *exynos_camera, struct exynos_jpeg *jpeg);

/*
 * Param
 */

int exynos_param_int_get(struct exynos_camera *exynos_camera,
	char *key);
float exynos_param_float_get(struct exynos_camera *exynos_camera,
	char *key);
char *exynos_param_string_get(struct exynos_camera *exynos_camera,
	char *key);
int exynos_param_int_set(struct exynos_camera *exynos_camera,
	char *key, int integer);
int exynos_param_float_set(struct exynos_camera *exynos_camera,
	char *key, float floating);
int exynos_param_string_set(struct exynos_camera *exynos_camera,
	char *key, char *string);
char *exynos_params_string_get(struct exynos_camera *exynos_camera);
int exynos_params_string_set(struct exynos_camera *exynos_camera, char *string);

/*
 * Utils
 */

int list_head_insert(struct list_head *list, struct list_head *prev,
	struct list_head *next);
void list_head_remove(struct list_head *list);

int exynos_camera_buffer_length(int width, int height, int format);
void exynos_camera_yuv_planes(int width, int height, int format, int address, int *address_y, int *address_cb, int *address_cr);

/*
 * V4L2
 */

int exynos_v4l2_init(struct exynos_camera *exynos_camera);
int exynos_v4l2_index(struct exynos_camera *exynos_camera, int exynos_v4l2_id);
int exynos_v4l2_fd(struct exynos_camera *exynos_camera, int exynos_v4l2_id);

int exynos_v4l2_open(struct exynos_camera *exynos_camera, int exynos_v4l2_id);
void exynos_v4l2_close(struct exynos_camera *exynos_camera, int exynos_v4l2_id);
int exynos_v4l2_ioctl(struct exynos_camera *exynos_camera, int exynos_v4l2_id,
	int request, void *data);
int exynos_v4l2_poll(struct exynos_camera *exynos_camera, int exynos_v4l2_id);
int exynos_v4l2_qbuf(struct exynos_camera *exynos_camera, int exynos_v4l2_id,
	int type, int memory, int index, unsigned long userptr);
int exynos_v4l2_qbuf_cap(struct exynos_camera *exynos_camera, int exynos_v4l2_id,
	int index);
int exynos_v4l2_qbuf_out(struct exynos_camera *exynos_camera, int exynos_v4l2_id,
	int index, unsigned long userptr);
int exynos_v4l2_dqbuf(struct exynos_camera *exynos_camera, int exynos_v4l2_id,
	int type, int memory);
int exynos_v4l2_dqbuf_cap(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id);
int exynos_v4l2_dqbuf_out(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id);
int exynos_v4l2_reqbufs(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int type, int memory, int count);
int exynos_v4l2_reqbufs_cap(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int count);
int exynos_v4l2_reqbufs_out(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int count);
int exynos_v4l2_querybuf(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int type, int memory, int index);
int exynos_v4l2_querybuf_cap(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int index);
int exynos_v4l2_querybuf_out(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int index);
int exynos_v4l2_querycap(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int flags);
int exynos_v4l2_querycap_cap(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id);
int exynos_v4l2_querycap_out(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id);
int exynos_v4l2_streamon(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int type);
int exynos_v4l2_streamon_cap(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id);
int exynos_v4l2_streamon_out(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id);
int exynos_v4l2_streamoff(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int type);
int exynos_v4l2_streamoff_cap(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id);
int exynos_v4l2_streamoff_out(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id);
int exynos_v4l2_g_fmt(struct exynos_camera *exynos_camera, int exynos_v4l2_id,
	int type, int *width, int *height, int *fmt);
int exynos_v4l2_g_fmt_cap(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int *width, int *height, int *fmt);
int exynos_v4l2_g_fmt_out(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int *width, int *height, int *fmt);
int exynos_v4l2_s_fmt_pix(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int type, int width, int height, int fmt, int field,
	int priv);
int exynos_v4l2_s_fmt_pix_cap(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int width, int height, int fmt, int priv);
int exynos_v4l2_s_fmt_pix_out(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int width, int height, int fmt, int priv);
int exynos_v4l2_s_fmt_win(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int left, int top, int width, int height);
int exynos_v4l2_enum_fmt(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int type, int fmt);
int exynos_v4l2_enum_fmt_cap(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int fmt);
int exynos_v4l2_enum_fmt_out(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int fmt);
int exynos_v4l2_enum_input(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int id);
int exynos_v4l2_s_input(struct exynos_camera *exynos_camera, int exynos_v4l2_id,
	int id);
int exynos_v4l2_g_ext_ctrls(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, struct v4l2_ext_control *control, int count);
int exynos_v4l2_g_ctrl(struct exynos_camera *exynos_camera, int exynos_v4l2_id,
	int id, int *value);
int exynos_v4l2_s_ctrl(struct exynos_camera *exynos_camera, int exynos_v4l2_id,
	int id, int value);
int exynos_v4l2_s_parm(struct exynos_camera *exynos_camera, int exynos_v4l2_id,
	int type, struct v4l2_streamparm *streamparm);
int exynos_v4l2_s_parm_cap(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, struct v4l2_streamparm *streamparm);
int exynos_v4l2_s_parm_out(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, struct v4l2_streamparm *streamparm);
int exynos_v4l2_s_crop(struct exynos_camera *exynos_camera, int exynos_v4l2_id,
	int type, int left, int top, int width, int height);
int exynos_v4l2_s_crop_cap(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int left, int top, int width, int height);
int exynos_v4l2_s_crop_out(struct exynos_camera *exynos_camera,
	int exynos_v4l2_id, int left, int top, int width, int height);
int exynos_v4l2_g_fbuf(struct exynos_camera *exynos_camera, int exynos_v4l2_id,
	void **base, int *width, int *height, int *fmt);
int exynos_v4l2_s_fbuf(struct exynos_camera *exynos_camera, int exynos_v4l2_id,
	void *base, int width, int height, int fmt);

/*
 * V4L2 Output
 */

int exynos_v4l2_output_start(struct exynos_camera *exynos_camera,
	struct exynos_v4l2_output *output);
void exynos_v4l2_output_stop(struct exynos_camera *exynos_camera,
	struct exynos_v4l2_output *output);
int exynos_v4l2_output(struct exynos_camera *exynos_camera,
	struct exynos_v4l2_output *output, int buffer_address);
int exynos_v4l2_output_release(struct exynos_camera *exynos_camera,
	struct exynos_v4l2_output *output);

#endif
