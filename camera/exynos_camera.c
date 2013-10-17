/*
 * Copyright (C) 2013 Paul Kocialkowski
 * Modified for N5110: Scott Brissenden
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
#include <time.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>

#define LOG_TAG "exynos_camera"
#include <utils/Log.h>
#include <utils/Timers.h>

#include "exynos_camera.h"

#define BIG2LITTLE_ENDIAN(big)	((big & 0xff) << 24 | (big & 0xff00) << 8 | (big & 0xff0000) >> 8 | (big & 0xff000000) >> 24)

/*
 * Devices configurations
 */


struct exynos_camera_preset exynos_camera_presets_smdk4x12[] = {
	{
		.name = "ISX012",
		.facing = CAMERA_FACING_BACK,
		.orientation = 90,
		.rotation = 0,
		.hflip = 0,
		.vflip = 0,
		.capture_format = V4L2_PIX_FMT_NV21,
		.picture_format = V4L2_PIX_FMT_YUYV,
		.fimc_is = 0,
		.focal_length = 3.43f,
		.horizontal_view_angle = 59.6f,
		.vertical_view_angle = 46.3f,
		.metering = METERING_CENTER,
		.params = {
			.preview_size_values = "1024x768",
			.preview_size = "1024x768",
			.preview_format_values = "yuv420sp,yuv420p",
			.preview_format = "yuv420sp",
			.preview_frame_rate_values = "30,20,15",
			.preview_frame_rate = 30,
			.preview_fps_range_values = "(10000,30000)",
			.preview_fps_range = "10000,30000",

			.picture_size_values = "2560x1920,2048x1536,1600x1200,960x720,640x480",
			.picture_size = "2560x1920",
			.picture_format_values = "jpeg",
			.picture_format = "yuyv",
			.jpeg_thumbnail_size_values = "160x120,0x0",
			.jpeg_thumbnail_width = 160,
			.jpeg_thumbnail_height = 120,
			.jpeg_thumbnail_quality = 100,
			.jpeg_quality = 90,

			.video_snapshot_supported = 0,
			.full_video_snap_supported = 0,

			.recording_size = "1280x720",
			.recording_size_values = "1280x720,1920x1080,720x480,640x480,352x288,320x240,176x144",
			.recording_format = "yuv420sp",

			.focus_mode = "auto",
			.focus_mode_values = "auto,macro,fixed",
			.focus_distances = "0.15,1.20,Infinity",
			.focus_areas = NULL,
			.max_num_focus_areas = 1,

			.zoom_supported = 0,
			//.smooth_zoom_supported = 0,
			//.zoom_ratios = "100,102,104,109,111,113,119,121,124,131,134,138,146,150,155,159,165,170,182,189,200,213,222,232,243,255,283,300,319,364,400",
			//.zoom = 0,
			//.max_zoom = 30,

			.flash_mode = NULL,
			.flash_mode_values = NULL,

			.exposure_compensation = 0,
			.exposure_compensation_step = 0.5,
			.min_exposure_compensation = -4,
			.max_exposure_compensation = 4,

			.whitebalance = "auto",
			.whitebalance_values = "auto,fluorescent,daylight,cloudy-daylight",

			.antibanding = "50hz",
			.antibanding_values = "50hz,off",

			.scene_mode = "auto",
			.scene_mode_values = "auto,portrait,landscape,night,beach,snow,sunset,fireworks,sports,party,candlelight,dusk-dawn,fall-color,text,back-light",

			.effect = "none",
			.effect_values = "none,mono,negative,sepia,solarize,sketch,pastel",

			.iso = "auto",
			.iso_values = "auto,ISO100,ISO200,ISO400,ISO800",
		},
		.mbus_resolutions = NULL,
		.mbus_resolutions_count = 0,
	},
	{
		.name = "SR130PC20",
		.facing = CAMERA_FACING_FRONT,
		.orientation = 270,
		.rotation = 0,
		.hflip = 0,
		.vflip = 0,
		.capture_format = V4L2_PIX_FMT_NV21,
		.picture_format = V4L2_PIX_FMT_YUYV,
		.fimc_is = 1,
		.focal_length = 2.17f,
		.horizontal_view_angle = 54.7f,
		.vertical_view_angle = 52.58f,
		.metering = METERING_CENTER,
		.params = {
			.preview_size_values = "640x480,352x288,320x240",
			.preview_size = "640x480",
			.preview_format_values = "yuv420sp,yuv420p",
			.preview_format = "yuv420sp",
			.preview_frame_rate_values = "30,20,15,8",
			.preview_frame_rate = 30,
			.preview_fps_range_values = "(15000,30000)",
			.preview_fps_range = "15000,30000",

			.picture_size_values = "1280x960,640x480",
			.picture_size = "1280x960",
			.picture_format_values = "jpeg",
			.picture_format = "yuyv",
			.jpeg_thumbnail_size_values = "160x120,160x160,160x90,144x96",
			.jpeg_thumbnail_width = 160,
			.jpeg_thumbnail_height = 120,
			.jpeg_thumbnail_quality = 100,
			.jpeg_quality = 100,

			.video_snapshot_supported = 0,
			.full_video_snap_supported = 0,

			.recording_size = "640x480",
			.recording_size_values = "640x480",
			.recording_format = "yuv420sp",

			.focus_mode = "fixed",
			.focus_mode_values = "infinity,fixed",
			.focus_distances = "0.20,0.25,Infinity",
			.focus_areas = NULL,
			.max_num_focus_areas = 0,

			.zoom_supported = 0,

			.flash_mode = NULL,
			.flash_mode_values = NULL,

			.exposure_compensation = 0,
			.exposure_compensation_step = 0.5,
			.min_exposure_compensation = -4,
			.max_exposure_compensation = 4,

			.whitebalance = "auto",
			.whitebalance_values = "auto,fluorescent,daylight,cloudy-daylight",

			.antibanding = "50hz",
			.antibanding_values = "50hz,off",

			.scene_mode = "auto",
			.scene_mode_values = "auto,portrait,landscape,night,beach,snow,sunset,fireworks,sports,party,candlelight,dusk-dawn,fall-color,text,back-light",

			.effect = "none",
			.effect_values = "none,mono,negative,sepia,solarize,sketch,pastel",

			.iso = "auto",
			.iso_values = "auto,ISO100,ISO200,ISO400,ISO800",
		},
		.mbus_resolutions = NULL,
		.mbus_resolutions_count = 0,
	},
};

struct exynos_v4l2_node exynos_v4l2_nodes_smdk4x12[] = {
	{	// FIMC0 is used for capture
		.id = 0,
		.node = "/dev/video0",
	},
	{	// FIMC1 is used for preview output
		.id = 1,
		.node = "/dev/video1",
	},
	{	// FIMC2 is used for picture output
		.id = 2,
		.node = "/dev/video2",
	},
	{	// FIMC3 is used for recording output
		.id = 3,
		.node = "/dev/video3",
	},
};

struct exynox_camera_config exynos_camera_config_smdk4x12 = {
	.presets = (struct exynos_camera_preset *) &exynos_camera_presets_smdk4x12,
	.presets_count = 2,
	.v4l2_nodes = (struct exynos_v4l2_node *) &exynos_v4l2_nodes_smdk4x12,
	.v4l2_nodes_count = 4,
};

/*
 * Exynos Camera
 */

struct exynox_camera_config *exynos_camera_config =
	&exynos_camera_config_smdk4x12;

int exynos_camera_start(struct exynos_camera *exynos_camera, int id)
{
	int rc;

	if (exynos_camera == NULL || id >= exynos_camera->config->presets_count)
		return -EINVAL;

	// ION

#ifdef EXYNOS_ION
	rc = exynos_ion_init(exynos_camera);
	if (rc < 0) {
		ALOGE("%s: Unable to init ION", __func__);
		goto error;
	}

	rc = exynos_ion_open(exynos_camera);
	if (rc < 0) {
		ALOGE("%s: Unable to open ION", __func__);
		goto error;
	}
#endif
	// V4L2

	rc = exynos_v4l2_init(exynos_camera);
	if (rc < 0) {
		ALOGE("%s: Unable to init v4l2", __func__);
		goto error;
	}
	// FIMC0

	rc = exynos_v4l2_open(exynos_camera, 0);
	if (rc < 0) {
		ALOGE("%s: Unable to open v4l2 device", __func__);
		goto error;
	}

	rc = exynos_v4l2_querycap_cap(exynos_camera, 0);
	if (rc < 0) {
		ALOGE("%s: Unable to query capabilities", __func__);
		goto error;
	}

	rc = exynos_v4l2_enum_input(exynos_camera, 0, id);
	if (rc < 0) {
		ALOGE("%s: Unable to enumerate input", __func__);
		goto error;
	}

	rc = exynos_v4l2_s_input(exynos_camera, 0, id);
	if (rc < 0) {
		ALOGE("%s: Unable to set inputs", __func__);
		goto error;
	}

	// Recording

	exynos_camera->recording_metadata = 1;

	// Params

	rc = exynos_camera_params_init(exynos_camera, id);
	if (rc < 0) {
		ALOGE("%s: Unable to init params", __func__);
		goto error;
	}

	// Gralloc

	rc = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, (const struct hw_module_t **) &exynos_camera->gralloc);
	if (rc)
		ALOGE("%s: Unable to get gralloc module", __func__);

	rc = 0;
	goto complete;

error:
	exynos_v4l2_close(exynos_camera, 0);

#ifdef EXYNOS_ION
	exynos_ion_close(exynos_camera);
#endif

	rc = -1;

complete:
	return rc;
}

void exynos_camera_stop(struct exynos_camera *exynos_camera)
{
	int i;
	int id;

	if (exynos_camera == NULL || exynos_camera->config == NULL)
		return;

	exynos_v4l2_close(exynos_camera, 0);

#ifdef EXYNOS_ION
	exynos_ion_close(exynos_camera);
#endif
}

// Params

int exynos_camera_params_init(struct exynos_camera *exynos_camera, int id)
{
	int rc;

	if (exynos_camera == NULL || id >= exynos_camera->config->presets_count)
		return -EINVAL;

	// Camera params

	exynos_camera->camera_rotation = exynos_camera->config->presets[id].rotation;
	exynos_camera->camera_hflip = exynos_camera->config->presets[id].hflip;
	exynos_camera->camera_vflip = exynos_camera->config->presets[id].vflip;
	exynos_camera->camera_capture_format = exynos_camera->config->presets[id].capture_format;
	exynos_camera->camera_picture_format = exynos_camera->config->presets[id].picture_format;
	exynos_camera->camera_fimc_is = exynos_camera->config->presets[id].fimc_is;
	exynos_camera->camera_focal_length = (int) (exynos_camera->config->presets[id].focal_length * 100);
	exynos_camera->camera_metering = exynos_camera->config->presets[id].metering;

	exynos_camera->camera_mbus_resolutions = exynos_camera->config->presets[id].mbus_resolutions;
	exynos_camera->camera_mbus_resolutions_count = exynos_camera->config->presets[id].mbus_resolutions_count;

	// Recording preview

	exynos_param_string_set(exynos_camera, "preferred-preview-size-for-video",
		exynos_camera->config->presets[id].params.preview_size);

	// Preview

	exynos_param_string_set(exynos_camera, "preview-size-values",
		exynos_camera->config->presets[id].params.preview_size_values);
	exynos_param_string_set(exynos_camera, "preview-size",
		exynos_camera->config->presets[id].params.preview_size);
	exynos_param_string_set(exynos_camera, "preview-format-values",
		exynos_camera->config->presets[id].params.preview_format_values);
	exynos_param_string_set(exynos_camera, "preview-format",
		exynos_camera->config->presets[id].params.preview_format);
	exynos_param_string_set(exynos_camera, "preview-frame-rate-values",
		exynos_camera->config->presets[id].params.preview_frame_rate_values);
	exynos_param_int_set(exynos_camera, "preview-frame-rate",
		exynos_camera->config->presets[id].params.preview_frame_rate);
	exynos_param_string_set(exynos_camera, "preview-fps-range-values",
		exynos_camera->config->presets[id].params.preview_fps_range_values);
	exynos_param_string_set(exynos_camera, "preview-fps-range",
		exynos_camera->config->presets[id].params.preview_fps_range);

	// Picture

	exynos_param_string_set(exynos_camera, "picture-size-values",
		exynos_camera->config->presets[id].params.picture_size_values);
	exynos_param_string_set(exynos_camera, "picture-size",
		exynos_camera->config->presets[id].params.picture_size);
	exynos_param_string_set(exynos_camera, "picture-format-values",
		exynos_camera->config->presets[id].params.picture_format_values);
	exynos_param_string_set(exynos_camera, "picture-format",
		exynos_camera->config->presets[id].params.picture_format);
	exynos_param_string_set(exynos_camera, "jpeg-thumbnail-size-values",
		exynos_camera->config->presets[id].params.jpeg_thumbnail_size_values);
	exynos_param_int_set(exynos_camera, "jpeg-thumbnail-width",
		exynos_camera->config->presets[id].params.jpeg_thumbnail_width);
	exynos_param_int_set(exynos_camera, "jpeg-thumbnail-height",
		exynos_camera->config->presets[id].params.jpeg_thumbnail_height);
	exynos_param_int_set(exynos_camera, "jpeg-thumbnail-quality",
		exynos_camera->config->presets[id].params.jpeg_thumbnail_quality);
	exynos_param_int_set(exynos_camera, "jpeg-quality",
		exynos_camera->config->presets[id].params.jpeg_quality);

	if (exynos_camera->config->presets[id].params.video_snapshot_supported == 1)
		exynos_param_string_set(exynos_camera, "video-snapshot-supported", "true");
	if (exynos_camera->config->presets[id].params.full_video_snap_supported == 1)
		exynos_param_string_set(exynos_camera, "full-video-snap-supported", "true");

	// Recording

	exynos_param_string_set(exynos_camera, "video-size",
		exynos_camera->config->presets[id].params.recording_size);
	exynos_param_string_set(exynos_camera, "video-size-values",
		exynos_camera->config->presets[id].params.recording_size_values);
	exynos_param_string_set(exynos_camera, "video-frame-format",
		exynos_camera->config->presets[id].params.recording_format);

	// Focus

	exynos_param_string_set(exynos_camera, "focus-mode",
		exynos_camera->config->presets[id].params.focus_mode);
	exynos_param_string_set(exynos_camera, "focus-mode-values",
		exynos_camera->config->presets[id].params.focus_mode_values);
	exynos_param_string_set(exynos_camera, "focus-distances",
		exynos_camera->config->presets[id].params.focus_distances);
	if (exynos_camera->config->presets[id].params.max_num_focus_areas > 0) {
		exynos_param_string_set(exynos_camera, "focus-areas",
			exynos_camera->config->presets[id].params.focus_areas);
		exynos_param_int_set(exynos_camera, "max-num-focus-areas",
			exynos_camera->config->presets[id].params.max_num_focus_areas);
	}

	// Zoom

	if (exynos_camera->config->presets[id].params.zoom_supported == 1) {
		exynos_param_string_set(exynos_camera, "zoom-supported", "true");

		if (exynos_camera->config->presets[id].params.smooth_zoom_supported == 1)
			exynos_param_string_set(exynos_camera, "smooth-zoom-supported", "true");

		if (exynos_camera->config->presets[id].params.zoom_ratios != NULL)
			exynos_param_string_set(exynos_camera, "zoom-ratios", exynos_camera->config->presets[id].params.zoom_ratios);

		exynos_param_int_set(exynos_camera, "zoom", exynos_camera->config->presets[id].params.zoom);
		exynos_param_int_set(exynos_camera, "max-zoom", exynos_camera->config->presets[id].params.max_zoom);

	} else {
		exynos_param_string_set(exynos_camera, "zoom-supported", "false");
	}

	// Flash

	exynos_param_string_set(exynos_camera, "flash-mode",
		exynos_camera->config->presets[id].params.flash_mode);
	exynos_param_string_set(exynos_camera, "flash-mode-values",
		exynos_camera->config->presets[id].params.flash_mode_values);

	// Exposure

	exynos_param_int_set(exynos_camera, "exposure-compensation",
		exynos_camera->config->presets[id].params.exposure_compensation);
	exynos_param_float_set(exynos_camera, "exposure-compensation-step",
		exynos_camera->config->presets[id].params.exposure_compensation_step);
	exynos_param_int_set(exynos_camera, "min-exposure-compensation",
		exynos_camera->config->presets[id].params.min_exposure_compensation);
	exynos_param_int_set(exynos_camera, "max-exposure-compensation",
		exynos_camera->config->presets[id].params.max_exposure_compensation);

	// Antibanding

	exynos_param_string_set(exynos_camera, "antibanding",
		exynos_camera->config->presets[id].params.antibanding);
	exynos_param_string_set(exynos_camera, "antibanding-values",
		exynos_camera->config->presets[id].params.antibanding_values);

	// WB

	exynos_param_string_set(exynos_camera, "whitebalance",
		exynos_camera->config->presets[id].params.whitebalance);
	exynos_param_string_set(exynos_camera, "whitebalance-values",
		exynos_camera->config->presets[id].params.whitebalance_values);

	// Scene mode

	exynos_param_string_set(exynos_camera, "scene-mode",
		exynos_camera->config->presets[id].params.scene_mode);
	exynos_param_string_set(exynos_camera, "scene-mode-values",
		exynos_camera->config->presets[id].params.scene_mode_values);

	// Effect

	exynos_param_string_set(exynos_camera, "effect",
		exynos_camera->config->presets[id].params.effect);
	exynos_param_string_set(exynos_camera, "effect-values",
		exynos_camera->config->presets[id].params.effect_values);

	// ISO

	exynos_param_string_set(exynos_camera, "iso",
		exynos_camera->config->presets[id].params.iso);
	exynos_param_string_set(exynos_camera, "iso-values",
		exynos_camera->config->presets[id].params.iso_values);

	// Camera

	exynos_param_float_set(exynos_camera, "focal-length",
		exynos_camera->config->presets[id].focal_length);
	exynos_param_float_set(exynos_camera, "horizontal-view-angle",
		exynos_camera->config->presets[id].horizontal_view_angle);
	exynos_param_float_set(exynos_camera, "vertical-view-angle",
		exynos_camera->config->presets[id].vertical_view_angle);

	rc = exynos_camera_params_apply(exynos_camera, 1);
	if (rc < 0) {
		ALOGE("%s: Unable to apply params", __func__);
		return -1;
	}

	return 0;
}

int exynos_camera_params_apply(struct exynos_camera *exynos_camera, int force)
{
	char *recording_hint_string;
	char *recording_preview_size_string;

	char *preview_size_string;
	int preview_width = 0;
	int preview_height = 0;
	char *preview_format_string;
	int preview_format;
	int preview_fps;

	char *picture_size_string;
	int picture_width = 0;
	int picture_height = 0;
	char *picture_format_string;
	int picture_format;

	int jpeg_thumbnail_width;
	int jpeg_thumbnail_height;
	int jpeg_thumbnail_quality;
	int jpeg_quality;

	char *video_size_string;
	int recording_width = 0;
	int recording_height = 0;
	char *video_frame_format_string;
	int recording_format;
	int camera_sensor_mode;
	int fimc_is_mode = 0;

	char *focus_mode_string;
	int focus_mode = 0;
	char *focus_areas_string;
	int focus_left, focus_top, focus_right, focus_bottom, focus_weigth;
	int focus_x;
	int focus_y;

	char *zoom_supported_string;
	int zoom, max_zoom;

	char *flash_mode_string;
	int flash_mode;

	int exposure_compensation;
	int min_exposure_compensation;
	int max_exposure_compensation;

	char *antibanding_string;
	int antibanding;

	char *whitebalance_string;
	int whitebalance;

	char *scene_mode_string;
	int scene_mode;

	char *effect_string;
	int effect;

	char *iso_string;
	int iso;

	int w, h;
	char *k;
	int rc;

	if (exynos_camera == NULL)
		return -EINVAL;

	// Preview

	preview_size_string = exynos_param_string_get(exynos_camera, "preview-size");
	if (preview_size_string != NULL) {
		sscanf(preview_size_string, "%dx%d", &preview_width, &preview_height);

		if (preview_width != 0 && preview_width != exynos_camera->preview_width)
			exynos_camera->preview_width = preview_width;
		if (preview_height != 0 && preview_height != exynos_camera->preview_height)
			exynos_camera->preview_height = preview_height;
	}

	preview_format_string = exynos_param_string_get(exynos_camera, "preview-format");
	if (preview_format_string != NULL) {
		if (strcmp(preview_format_string, "yuv420sp") == 0) {
			preview_format = V4L2_PIX_FMT_NV21;
		} else if (strcmp(preview_format_string, "yuv420p") == 0) {
			preview_format = V4L2_PIX_FMT_YUV420;
		} else if (strcmp(preview_format_string, "rgb565") == 0) {
			preview_format = V4L2_PIX_FMT_RGB565;
		} else if (strcmp(preview_format_string, "rgb8888") == 0) {
			preview_format = V4L2_PIX_FMT_RGB32;
		} else {
			ALOGE("%s: Unsupported preview format: %s", __func__, preview_format_string);
			preview_format = V4L2_PIX_FMT_NV21;
		}

		if (preview_format != exynos_camera->preview_format)
			exynos_camera->preview_format = preview_format;
	}

	preview_fps = exynos_param_int_get(exynos_camera, "preview-frame-rate");
	if (preview_fps > 0)
		exynos_camera->preview_fps = preview_fps;
	else
		exynos_camera->preview_fps = 0;

	// Picture

	picture_size_string = exynos_param_string_get(exynos_camera, "picture-size");
	if (picture_size_string != NULL) {
		sscanf(picture_size_string, "%dx%d", &picture_width, &picture_height);

		if (picture_width != 0 && picture_height != 0 && (picture_width != exynos_camera->picture_width || picture_height != exynos_camera->picture_height)) {
			exynos_camera->picture_width = picture_width;
			exynos_camera->picture_height = picture_height;
		}
	}

	picture_format_string = exynos_param_string_get(exynos_camera, "picture-format");
	if (picture_format_string != NULL) {
		if (strcmp(picture_format_string, "jpeg") == 0) {
			picture_format = V4L2_PIX_FMT_JPEG;
		}else if (strcmp(picture_format_string, "yuyv") == 0) {
			picture_format = V4L2_PIX_FMT_YUYV;
		} else {
			ALOGE("%s: Unsupported picture format: %s", __func__, picture_format_string);
			picture_format = V4L2_PIX_FMT_JPEG;
		}

		if (picture_format != exynos_camera->picture_format)
			exynos_camera->picture_format = picture_format;
	}

	jpeg_thumbnail_width = exynos_param_int_get(exynos_camera, "jpeg-thumbnail-width");
	if (jpeg_thumbnail_width > 0)
		exynos_camera->jpeg_thumbnail_width = jpeg_thumbnail_width;

	jpeg_thumbnail_height = exynos_param_int_get(exynos_camera, "jpeg-thumbnail-height");
	if (jpeg_thumbnail_height > 0)
		exynos_camera->jpeg_thumbnail_height = jpeg_thumbnail_height;

	jpeg_thumbnail_quality = exynos_param_int_get(exynos_camera, "jpeg-thumbnail-quality");
	if (jpeg_thumbnail_quality > 0)
		exynos_camera->jpeg_thumbnail_quality = jpeg_thumbnail_quality;

	jpeg_quality = exynos_param_int_get(exynos_camera, "jpeg-quality");
	if (jpeg_quality <= 100 && jpeg_quality >= 0 && (jpeg_quality != exynos_camera->jpeg_quality || force)) {
		exynos_camera->jpeg_quality = jpeg_quality;
		rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAM_JPEG_QUALITY, jpeg_quality);
		if (rc < 0)
			ALOGE("%s: Unable to set jpeg quality", __func__);
	}

	// Recording

	video_size_string = exynos_param_string_get(exynos_camera, "video-size");
	if (video_size_string == NULL)
		video_size_string = exynos_param_string_get(exynos_camera, "preview-size");

	if (video_size_string != NULL) {
		sscanf(video_size_string, "%dx%d", &recording_width, &recording_height);

		if (recording_width != 0 && recording_width != exynos_camera->recording_width)
			exynos_camera->recording_width = recording_width;
		if (recording_height != 0 && recording_height != exynos_camera->recording_height)
			exynos_camera->recording_height = recording_height;
	}

	video_frame_format_string = exynos_param_string_get(exynos_camera, "video-frame-format");
	if (video_frame_format_string != NULL) {
		if (strcmp(video_frame_format_string, "yuv420sp") == 0) {
			recording_format = V4L2_PIX_FMT_NV12;
		} else if (strcmp(video_frame_format_string, "yuv420p") == 0) {
			recording_format = V4L2_PIX_FMT_YUV420;
		} else if (strcmp(video_frame_format_string, "rgb565") == 0) {
			recording_format = V4L2_PIX_FMT_RGB565;
		} else if (strcmp(video_frame_format_string, "rgb8888") == 0) {
			recording_format = V4L2_PIX_FMT_RGB32;
		} else {
			ALOGE("%s: Unsupported recording format: %s", __func__, video_frame_format_string);
			recording_format = V4L2_PIX_FMT_NV12;
		}

		if (recording_format != exynos_camera->recording_format)
			exynos_camera->recording_format = recording_format;
	}

	recording_hint_string = exynos_param_string_get(exynos_camera, "recording-hint");
	if (recording_hint_string != NULL && strcmp(recording_hint_string, "true") == 0) {
		camera_sensor_mode = SENSOR_MOVIE;

		k = exynos_param_string_get(exynos_camera, "preview-size-values");
		while (recording_width != 0 && recording_height != 0) {
			if (k == NULL)
				break;

			sscanf(k, "%dx%d", &w, &h);

			// Look for same aspect ratio
			if ((recording_width * h) / recording_height == w) {
				preview_width = w;
				preview_height = h;
				break;
			}

			k = strchr(k, ',');
			if (k == NULL)
				break;

			k++;
		}

		if (preview_width != 0 && preview_width != exynos_camera->preview_width)
			exynos_camera->preview_width = preview_width;
		if (preview_height != 0 && preview_height != exynos_camera->preview_height)
			exynos_camera->preview_height = preview_height;
	} else {
		camera_sensor_mode = SENSOR_CAMERA;
	}

	// Switching modes

	if (camera_sensor_mode != exynos_camera->camera_sensor_mode) {
		exynos_camera->camera_sensor_mode = camera_sensor_mode;
		rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_SENSOR_MODE, camera_sensor_mode);
		if (rc < 0)
			ALOGE("%s: Unable to set sensor mode", __func__);
	}

	// Focus

	focus_areas_string = exynos_param_string_get(exynos_camera, "focus-areas");
	if (focus_areas_string != NULL) {
		focus_left = focus_top = focus_right = focus_bottom = focus_weigth = 0;

		rc = sscanf(focus_areas_string, "(%d,%d,%d,%d,%d)",
			&focus_left, &focus_top, &focus_right, &focus_bottom, &focus_weigth);
		if (rc != 5) {
			ALOGE("%s: Unable to scan focus areas", __func__);
		} else if (focus_left != 0 && focus_top != 0 && focus_right != 0 && focus_bottom != 0) {
			focus_x = (((focus_left + focus_right) / 2) + 1000) * preview_width / 2000;
			focus_y =  (((focus_top + focus_bottom) / 2) + 1000) * preview_height / 2000;

			if (focus_x != exynos_camera->focus_x || force) {
				exynos_camera->focus_x = focus_x;

				rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_OBJECT_POSITION_X, focus_x);
				if (rc < 0)
					ALOGE("%s: Unable to set object x position", __func__);
			}

			if (focus_y != exynos_camera->focus_y || force) {
				exynos_camera->focus_y = focus_y;

				rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_OBJECT_POSITION_Y, focus_y);
				if (rc < 0)
					ALOGE("%s: Unable to set object y position", __func__);
			}

			focus_mode = FOCUS_MODE_TOUCH;
		}
	}

	focus_mode_string = exynos_param_string_get(exynos_camera, "focus-mode");
	if (focus_mode_string != NULL) {
		if (focus_mode == 0) {
			if (strcmp(focus_mode_string, "auto") == 0)
				focus_mode = FOCUS_MODE_AUTO;
			else if (strcmp(focus_mode_string, "infinity") == 0)
				focus_mode = FOCUS_MODE_INFINITY;
			else if (strcmp(focus_mode_string, "macro") == 0)
				focus_mode = FOCUS_MODE_MACRO;
			else if (strcmp(focus_mode_string, "fixed") == 0)
				focus_mode = FOCUS_MODE_FIXED;
			else if (strcmp(focus_mode_string, "facedetect") == 0)
				focus_mode = FOCUS_MODE_FACEDETECT;
			else if (strcmp(focus_mode_string, "continuous-video") == 0)
				focus_mode = FOCUS_MODE_CONTINOUS_VIDEO;
			else if (strcmp(focus_mode_string, "continuous-picture") == 0)
				focus_mode = FOCUS_MODE_CONTINOUS_PICTURE;
			else
				focus_mode = FOCUS_MODE_AUTO;
		}

		if (focus_mode != exynos_camera->focus_mode || force) {
			rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_FOCUS_MODE, focus_mode);
			if (rc < 0)
				ALOGE("%s: Unable to set focus mode", __func__);
		}

		exynos_camera->focus_mode = focus_mode;
	}

	// Zoom

	zoom_supported_string = exynos_param_string_get(exynos_camera, "zoom-supported");
	if (zoom_supported_string != NULL && strcmp(zoom_supported_string, "true") == 0) {
		zoom = exynos_param_int_get(exynos_camera, "zoom");
		max_zoom = exynos_param_int_get(exynos_camera, "max-zoom");
		if (zoom <= max_zoom && zoom >= 0 && (zoom != exynos_camera->zoom || force)) {
			exynos_camera->zoom = zoom;
			rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_ZOOM, zoom);
			if (rc < 0)
				ALOGE("%s: Unable to set camera zoom", __func__);
		}

	}

	// Flash

	flash_mode_string = exynos_param_string_get(exynos_camera, "flash-mode");
	if (flash_mode_string != NULL) {
		if (strcmp(flash_mode_string, "off") == 0)
			flash_mode = FLASH_MODE_OFF;
		else if (strcmp(flash_mode_string, "auto") == 0)
			flash_mode = FLASH_MODE_AUTO;
		else if (strcmp(flash_mode_string, "on") == 0)
			flash_mode = FLASH_MODE_ON;
		else if (strcmp(flash_mode_string, "torch") == 0)
			flash_mode = FLASH_MODE_TORCH;
		else
			flash_mode = FLASH_MODE_AUTO;

		if (flash_mode != exynos_camera->flash_mode || force) {
			exynos_camera->flash_mode = flash_mode;
			rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_FLASH_MODE, flash_mode);
			if (rc < 0)
				ALOGE("%s:Unable to set flash mode", __func__);
		}
	}

	// Exposure

	exposure_compensation = exynos_param_int_get(exynos_camera, "exposure-compensation");
	min_exposure_compensation = exynos_param_int_get(exynos_camera, "min-exposure-compensation");
	max_exposure_compensation = exynos_param_int_get(exynos_camera, "max-exposure-compensation");

	if (exposure_compensation <= max_exposure_compensation && exposure_compensation >= min_exposure_compensation &&
		(exposure_compensation != exynos_camera->exposure_compensation || force)) {
		exynos_camera->exposure_compensation = exposure_compensation;
		rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_BRIGHTNESS, exposure_compensation);
		if (rc < 0)
			ALOGE("%s: Unable to set exposure", __func__);
	}

	// Antibanding

	antibanding_string = exynos_param_string_get(exynos_camera, "antibanding");
	if (antibanding_string != NULL) {
		if (strcmp(antibanding_string, "auto") == 0)
			antibanding = ANTI_BANDING_AUTO;
		else if (strcmp(antibanding_string, "50hz") == 0)
			antibanding = ANTI_BANDING_50HZ;
		else if (strcmp(antibanding_string, "off") == 0)
			antibanding = ANTI_BANDING_OFF;
		else
			antibanding = ANTI_BANDING_AUTO;

		if (antibanding != exynos_camera->antibanding || force) {
			exynos_camera->antibanding = antibanding;
			rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_ANTI_BANDING, antibanding);
			if (rc < 0)
				ALOGE("%s: Unable to set antibanding", __func__);
		}
	}
	
	if(!exynos_camera->camera_fimc_is){
		rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_AE_LOCK_UNLOCK, AE_UNLOCK);
		if (rc < 0)
			 ALOGE("%s: Unable to set antibanding", __func__);
		rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_AWB_LOCK_UNLOCK, AWB_UNLOCK);
		if (rc < 0)
			  ALOGE("%s: Unable to set antibanding", __func__);
	}	

	// WB

	whitebalance_string = exynos_param_string_get(exynos_camera, "whitebalance");
	if (whitebalance_string != NULL) {
		if (strcmp(whitebalance_string, "auto") == 0)
			whitebalance = WHITE_BALANCE_AUTO;
		else if (strcmp(whitebalance_string, "incandescent") == 0)
			whitebalance = WHITE_BALANCE_TUNGSTEN;
		else if (strcmp(whitebalance_string, "fluorescent") == 0)
			whitebalance = WHITE_BALANCE_FLUORESCENT;
		else if (strcmp(whitebalance_string, "daylight") == 0)
			whitebalance = WHITE_BALANCE_SUNNY;
		else if (strcmp(whitebalance_string, "cloudy-daylight") == 0)
			whitebalance = WHITE_BALANCE_CLOUDY;
		else
			whitebalance = WHITE_BALANCE_AUTO;

		if (whitebalance != exynos_camera->whitebalance || force) {
			exynos_camera->whitebalance = whitebalance;
			rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_WHITE_BALANCE, whitebalance);
			if (rc < 0)
				ALOGE("%s: Unable to set whitebalance", __func__);
		}
	}

	// Scene mode

	scene_mode_string = exynos_param_string_get(exynos_camera, "scene-mode");
	if (scene_mode_string != NULL) {
		if (strcmp(scene_mode_string, "auto") == 0)
			scene_mode = SCENE_MODE_NONE;
		else if (strcmp(scene_mode_string, "portrait") == 0)
			scene_mode = SCENE_MODE_PORTRAIT;
		else if (strcmp(scene_mode_string, "landscape") == 0)
			scene_mode = SCENE_MODE_LANDSCAPE;
		else if (strcmp(scene_mode_string, "night") == 0)
			scene_mode = SCENE_MODE_NIGHTSHOT;
		else if (strcmp(scene_mode_string, "beach") == 0)
			scene_mode = SCENE_MODE_BEACH_SNOW;
		else if (strcmp(scene_mode_string, "snow") == 0)
			scene_mode = SCENE_MODE_BEACH_SNOW;
		else if (strcmp(scene_mode_string, "sunset") == 0)
			scene_mode = SCENE_MODE_SUNSET;
		else if (strcmp(scene_mode_string, "fireworks") == 0)
			scene_mode = SCENE_MODE_FIREWORKS;
		else if (strcmp(scene_mode_string, "sports") == 0)
			scene_mode = SCENE_MODE_SPORTS;
		else if (strcmp(scene_mode_string, "party") == 0)
			scene_mode = SCENE_MODE_PARTY_INDOOR;
		else if (strcmp(scene_mode_string, "candlelight") == 0)
			scene_mode = SCENE_MODE_CANDLE_LIGHT;
		else if (strcmp(scene_mode_string, "dusk-dawn") == 0)
			scene_mode = SCENE_MODE_DUSK_DAWN;
		else if (strcmp(scene_mode_string, "fall-color") == 0)
			scene_mode = SCENE_MODE_FALL_COLOR;
		else if (strcmp(scene_mode_string, "back-light") == 0)
			scene_mode = SCENE_MODE_BACK_LIGHT;
		else if (strcmp(scene_mode_string, "text") == 0)
			scene_mode = SCENE_MODE_TEXT;
		else
			scene_mode = SCENE_MODE_NONE;

		if (scene_mode != exynos_camera->scene_mode || force) {
			exynos_camera->scene_mode = scene_mode;
			rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_SCENE_MODE, scene_mode);
			if (rc < 0)
				ALOGE("%s: Unable to set scene mode", __func__);
		}
	}

	// Effect

	effect_string = exynos_param_string_get(exynos_camera, "effect");
	if (effect_string != NULL) {
		if (strcmp(effect_string, "auto") == 0)
			effect = IMAGE_EFFECT_NONE;
		if (strcmp(effect_string, "none") == 0)
			effect = IMAGE_EFFECT_NONE;
		else if (strcmp(effect_string, "mono") == 0)
			effect = IMAGE_EFFECT_BNW;
		else if (strcmp(effect_string, "negative") == 0)
			effect = IMAGE_EFFECT_NEGATIVE;
		else if (strcmp(effect_string, "sepia") == 0)
			effect = IMAGE_EFFECT_SEPIA;
		else if (strcmp(effect_string, "aqua") == 0)
			effect = IMAGE_EFFECT_AQUA;
		else if (strcmp(effect_string, "solarize") == 0)
			effect = IMAGE_EFFECT_SOLARIZE;
		else if (strcmp(effect_string, "posterize") == 0)
			effect = IMAGE_EFFECT_POSTERIZE;
		else if (strcmp(effect_string, "washed") == 0)
			effect = IMAGE_EFFECT_WASHED;
		else if (strcmp(effect_string, "sketch") == 0)
			effect = IMAGE_EFFECT_SKETCH;
		else if (strcmp(effect_string, "vintage-warm") == 0)
			effect = IMAGE_EFFECT_VINTAGE_WARM;
		else if (strcmp(effect_string, "vintage-cold") == 0)
			effect = IMAGE_EFFECT_VINTAGE_COLD;
		else if (strcmp(effect_string, "point-blue") == 0)
			effect = IMAGE_EFFECT_POINT_BLUE;
		else if (strcmp(effect_string, "point-red-yellow") == 0)
			effect = IMAGE_EFFECT_POINT_RED_YELLOW;
		else if (strcmp(effect_string, "point-green") == 0)
			effect = IMAGE_EFFECT_POINT_GREEN;
		else
			effect = IMAGE_EFFECT_NONE;

		if (effect != exynos_camera->effect || force) {
			exynos_camera->effect = effect;
			rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_EFFECT, effect);
			if (rc < 0)
				ALOGE("%s: Unable to set effect", __func__);
		}
	}

	// ISO

	iso_string = exynos_param_string_get(exynos_camera, "iso");
	if (iso_string != NULL) {
		if (strcmp(iso_string, "auto") == 0)
			iso = ISO_AUTO;
		else if (strcmp(iso_string, "ISO50") == 0)
			iso = ISO_50;
		else if (strcmp(iso_string, "ISO100") == 0)
			iso = ISO_100;
		else if (strcmp(iso_string, "ISO200") == 0)
			iso = ISO_200;
		else if (strcmp(iso_string, "ISO400") == 0)
			iso = ISO_400;
		else if (strcmp(iso_string, "ISO800") == 0)
			iso = ISO_800;
		else
			iso = ISO_AUTO;

		if (iso != exynos_camera->iso || force) {
			exynos_camera->iso = iso;
			rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_ISO, iso);
			if (rc < 0)
				ALOGE("%s: Unable to set iso", __func__);
		}
	}

	return 0;
}

// Capture
int exynos_camera_capture(struct exynos_camera *exynos_camera)
{
	struct exynos_camera_capture_listener *listener;
	struct exynos_camera_buffer *buffers = NULL;
	struct exynos_camera_buffer *buffer;
	struct list_head *list;
	int width, height, format;
	int yuv_length, jpeg_length;
	int jpeg_offset, jpeg_size;
	int jpeg_thumbnail_offset, jpeg_thumbnail_size;
	int buffers_count;
	int buffer_length;
	int auto_focus_result;
	int decoded;
	int busy;
	void *pointer;
	int address;
	int offset;
	int index;
	int rc;
	int value;

	if (exynos_camera == NULL)
		return -EINVAL;

	width = exynos_camera->capture_width;
	height = exynos_camera->capture_height;
	format = exynos_camera->capture_format;

	buffers_count = exynos_camera->capture_buffers_count;
	buffer_length = exynos_camera->capture_buffer_length;

	// V4L2
	
	//ALOGE("%s : ",__func__);	

	index = exynos_v4l2_dqbuf_cap(exynos_camera, 0);
	if (index < 0 || index >= buffers_count) {
		rc = exynos_v4l2_poll(exynos_camera, 0);
		if (rc < 0) {
			ALOGE("%s Unable to poll", __func__);
			goto error;
		} else if (rc == 0) {
			// Timeout
			rc = 0;
			goto complete;
		}

		index = exynos_v4l2_dqbuf_cap(exynos_camera, 0);
		if (index < 0 || index >= buffers_count) {
			ALOGE("%s: Unable to dequeue buffer", __func__);
			goto error;
		}
	}

	exynos_camera->capture_memory_index = index;

	address = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_PADDR_Y, index);
	if (address == 0 || address == (int) 0xffffffff) {
		ALOGE("%s: Unable to get address", __func__);
		goto error;
	}

	offset = address - exynos_camera->capture_memory_address;
	if (offset != index * buffer_length)
		ALOGE("%s: Inconsistent memory offset (0x%x/0x%x)", __func__, offset, index * buffer_length);

	pointer = (void *) ((unsigned char *) exynos_camera->capture_memory->data + offset);
	
	value = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_PADDR_CBCR, 0);

	// Buffers

	switch (format) {
		case V4L2_PIX_FMT_JPEG:
			jpeg_size = jpeg_offset = 0;
			jpeg_thumbnail_size = jpeg_thumbnail_offset = 0;

			rc = exynos_v4l2_g_ctrl(exynos_camera, 0, V4L2_CID_CAM_JPEG_MAIN_SIZE, &jpeg_size);
			if (rc < 0 || jpeg_size <= 0) {
				ALOGE("%s: Unable to get jpeg size", __func__);
				goto error;
			}

			rc = exynos_v4l2_g_ctrl(exynos_camera, 0, V4L2_CID_CAM_JPEG_MAIN_OFFSET, &jpeg_offset);
			if (rc < 0) {
				ALOGE("%s: Unable to get jpeg offset", __func__);
				goto error;
			}

			rc = exynos_v4l2_g_ctrl(exynos_camera, 0, V4L2_CID_CAM_JPEG_THUMB_SIZE, &jpeg_thumbnail_size);
			if (rc < 0 || jpeg_thumbnail_size <= 0) {
				ALOGE("%s: Unable to get jpeg thumbnail size", __func__);
				goto error;
			}

			rc = exynos_v4l2_g_ctrl(exynos_camera, 0, V4L2_CID_CAM_JPEG_THUMB_OFFSET, &jpeg_thumbnail_offset);
			if (rc < 0) {
				ALOGE("%s: Unable to get jpeg thumbnail offset", __func__);
				goto error;
			}

			buffers_count = 2;
			buffers = (struct exynos_camera_buffer *) calloc(buffers_count, sizeof(struct exynos_camera_buffer));

			buffer = buffers;

			buffer->pointer = (void *) ((unsigned char *) pointer + jpeg_offset);
			buffer->address = address + jpeg_offset;
			buffer->length = jpeg_size;
			buffer->width = exynos_camera->picture_width;
			buffer->height = exynos_camera->picture_height;
			buffer->format = V4L2_PIX_FMT_JPEG;

			buffer = (struct exynos_camera_buffer *) ((unsigned char *) buffer + sizeof(struct exynos_camera_buffer));

			buffer->pointer = (void *) ((unsigned char *) pointer + jpeg_thumbnail_offset);
			buffer->address = address + jpeg_thumbnail_offset;
			buffer->length = jpeg_thumbnail_size;
			buffer->width = exynos_camera->jpeg_thumbnail_width;
			buffer->height = exynos_camera->jpeg_thumbnail_height;
			buffer->format = V4L2_PIX_FMT_JPEG;
			break;
		default:
			buffers_count = 1;
			buffers = (struct exynos_camera_buffer *) calloc(buffers_count, sizeof(struct exynos_camera_buffer));

			buffer = buffers;

			buffer->pointer = pointer;
			buffer->address = address;
			buffer->length = buffer_length;
			buffer->width = width;
			buffer->height = height;
			buffer->format = format;
			break;
	}

	// Listeners

	list = (struct list_head *) exynos_camera->capture_listeners;
	while (list != NULL)  {
		listener = (struct exynos_camera_capture_listener *) list;

		if (listener->callback == NULL)
			goto list_continue_callback;

		/*
		 * Callback must never call a capture-locked function or it will
		 * block. Hence, do not unregister the listener in callback.
		 */

		listener->callback(exynos_camera, buffers, buffers_count);

list_continue_callback:
		list = list->next;
	}

	do {
		busy = 0;

		list = (struct list_head *) exynos_camera->capture_listeners;
		while (list != NULL)  {
			listener = (struct exynos_camera_capture_listener *) list;

			if (listener->callback == NULL)
				goto list_continue_busy;

			busy |= listener->busy;

list_continue_busy:
			list = list->next;
		}

		if (busy)
			usleep(1000);
	} while (busy);

	rc = exynos_v4l2_qbuf_cap(exynos_camera, 0, index);
	if (rc < 0) {
		ALOGE("%s: Unable to queue buffer", __func__);
		goto error;
	}

	rc = 0;
	goto complete;

error:
	rc = -1;

complete:
	if (buffers != NULL)
		free(buffers);

	return rc;
}

void *exynos_camera_capture_thread(void *data)
{
	struct exynos_camera *exynos_camera;
	int rc;

	if (data == NULL)
		return NULL;

	exynos_camera = (struct exynos_camera *) data;

	ALOGE("%s: Starting thread", __func__);
	exynos_camera->capture_thread_running = 1;

	while (exynos_camera->capture_thread_enabled) {
		pthread_mutex_lock(&exynos_camera->capture_lock_mutex);

		while (exynos_camera->capture_enabled) {
			pthread_mutex_lock(&exynos_camera->capture_mutex);

			if (!exynos_camera->capture_enabled) {
				pthread_mutex_unlock(&exynos_camera->capture_mutex);
				break;
			}

			rc = exynos_camera_capture(exynos_camera);
			if (rc < 0) {
				ALOGE("%s: Unable to capture", __func__);
				pthread_mutex_unlock(&exynos_camera->capture_mutex);
				break;
			}

			pthread_mutex_unlock(&exynos_camera->capture_mutex);

			// Wait a bit to let others lock the mutex if they need to
			usleep(10);
		}
	}

	exynos_camera->capture_thread_running = 0;
	ALOGE("%s: Exiting thread", __func__);

	return NULL;
}

int exynos_camera_capture_thread_start(struct exynos_camera *exynos_camera)
{
	pthread_attr_t thread_attr;
	int rc;

	if (exynos_camera == NULL)
		return -EINVAL;

	ALOGD("%s()", __func__);

	if (exynos_camera->capture_thread_enabled) {
		ALOGE("Capture thread was already started!");
		return -1;
	}

	pthread_mutex_init(&exynos_camera->capture_mutex, NULL);
	pthread_mutex_init(&exynos_camera->capture_lock_mutex, NULL);

	// Initial lock
	pthread_mutex_lock(&exynos_camera->capture_lock_mutex);

	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

	exynos_camera->capture_thread_enabled = 1;

	rc = pthread_create(&exynos_camera->capture_thread, &thread_attr, exynos_camera_capture_thread, (void *) exynos_camera);
	if (rc < 0) {
		ALOGE("%s: Unable to create thread", __func__);
		goto error;
	}

	rc = 0;
	goto complete;

error:
	pthread_mutex_destroy(&exynos_camera->capture_mutex);
	pthread_mutex_destroy(&exynos_camera->capture_lock_mutex);

	rc = -1;

complete:
	return rc;
}

void exynos_camera_capture_thread_stop(struct exynos_camera *exynos_camera)
{
	int i;

	if (exynos_camera == NULL)
		return;

	ALOGD("%s()", __func__);

	if (!exynos_camera->capture_thread_enabled) {
		ALOGE("Capture thread was already stopped!");
		return;
	}

	exynos_camera->capture_enabled = 0;
	exynos_camera->capture_thread_enabled = 0;

	pthread_mutex_unlock(&exynos_camera->capture_lock_mutex);

	// Wait for the thread to end
	i = 0;
	while (exynos_camera->capture_thread_running) {
		if (i++ > 10000) {
			ALOGE("Capture thread is taking too long to end, something is going wrong");
			break;
		}
		usleep(100);
	}

	if (exynos_camera->capture_enabled) {
		pthread_mutex_lock(&exynos_camera->capture_mutex);
		exynos_camera_capture_stop(exynos_camera);
		pthread_mutex_unlock(&exynos_camera->capture_mutex);
	}

	pthread_mutex_destroy(&exynos_camera->capture_mutex);
	pthread_mutex_destroy(&exynos_camera->capture_lock_mutex);
}

int exynos_camera_capture_start(struct exynos_camera *exynos_camera, int is_capture)
{
	struct v4l2_streamparm fps_param;
	int width, height, format;
	int mbus_width, mbus_height;
	int buffers_count, buffer_length;
	camera_memory_t *memory = NULL;
	int value;
	int fd;
	int rc;
	int i;

	if (exynos_camera == NULL)
		return -EINVAL;

	ALOGD("%s()", __func__);

	if (exynos_camera->capture_enabled) {
		ALOGE("Capture was already started!");
		return -1;
	}

	width = exynos_camera->capture_width;
	height = exynos_camera->capture_height;
	format = exynos_camera->capture_format;

	// V4L2

	rc = exynos_v4l2_enum_fmt_cap(exynos_camera, 0, format);
	if (rc < 0) {
		ALOGE("%s: Unable to enumerate formats", __func__);
		goto error;
	}

	mbus_width = width;
	mbus_height = height;

	if (exynos_camera->camera_mbus_resolutions != NULL) {
		for (i = 0; i < exynos_camera->camera_mbus_resolutions_count; i++) {
			if (exynos_camera->camera_mbus_resolutions[i].width == width && exynos_camera->camera_mbus_resolutions[i].height == height) {
				mbus_width = exynos_camera->camera_mbus_resolutions[i].mbus_width;
				mbus_height = exynos_camera->camera_mbus_resolutions[i].mbus_height;
				break;
			}
		}
	}
	
	if(!is_capture || exynos_camera->preview_output_enabled) {
		rc = exynos_v4l2_s_fmt_pix_cap(exynos_camera, 0, width, height, format, V4L2_PIX_FMT_MODE_PREVIEW);
		if (rc < 0) {
			ALOGE("%s: Unable to set capture pixel format", __func__);
			goto error;
		}

		// Set MBUS width/height/format
		rc = exynos_v4l2_s_fmt_pix(exynos_camera, 0, V4L2_BUF_TYPE_PRIVATE, mbus_width, mbus_height, format, V4L2_FIELD_NONE, V4L2_PIX_FMT_MODE_PREVIEW);
		if (rc < 0) {
			ALOGE("%s: Unable to set MBUS capture pixel format", __func__);
			goto error;
		}
	}else {
		rc = exynos_v4l2_s_fmt_pix_cap(exynos_camera, 0, width, height, format, V4L2_PIX_FMT_MODE_CAPTURE);
		if (rc < 0) {
			ALOGE("%s: Unable to set capture pixel format", __func__);
			goto error;
		}

		// Set MBUS width/height/format
	        rc = exynos_v4l2_s_fmt_pix(exynos_camera, 0, V4L2_BUF_TYPE_PRIVATE, mbus_width, mbus_height, format, IS_MODE_CAPTURE_STILL, V4L2_PIX_FMT_MODE_CAPTURE);
		if (rc < 0) {
			 ALOGE("%s: Unable to set MBUS capture pixel format", __func__);
			 goto error;
		}
	}

	if(!is_capture || exynos_camera->preview_output_enabled){
		rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_EMBEDDEDDATA_ENABLE, 0);
		if (rc < 0) {
			ALOGE("%s: Unable to set embdedded data enable", __func__);
			goto error;
		}
	}

	// Let's assume FIMC0 has memory available through mmap
	if(!is_capture || exynos_camera->preview_output_enabled) {
		for (i = EXYNOS_CAMERA_PREVIEW_BUFFERS_COUNT; i > 0; i--) {
			rc = exynos_v4l2_reqbufs_cap(exynos_camera, 0, i);
			if (rc >= 0)
				break;
		}
	}else {
		for (i = EXYNOS_CAMERA_CAPTURE_BUFFERS_COUNT; i > 0; i--) {
			rc = exynos_v4l2_reqbufs_cap(exynos_camera, 0, i);
			if (rc >= 0)
				break;
		}
	}
	
	if (rc < 0) {
		ALOGE("%s: Unable to request buffers", __func__);
		goto error;
	}

	buffers_count = rc;
	ALOGD("Found %d buffers available for capture!", buffers_count);

	memset(&fps_param, 0, sizeof(fps_param));
	fps_param.parm.capture.timeperframe.numerator = 1;
	fps_param.parm.capture.timeperframe.denominator = exynos_camera->preview_fps;

	rc = exynos_v4l2_s_parm_cap(exynos_camera, 0, &fps_param);
	if (rc < 0) {
		ALOGE("%s: Unable to set fps", __func__);
		goto error;
	}

	for (i = 0; i < buffers_count; i++) {
		rc = exynos_v4l2_querybuf_cap(exynos_camera, 0, i);
		if (rc < 0) {
			ALOGE("%s: Unable to query buffers", __func__);
			goto error;
		}
	}

	buffer_length = rc;
	
	value = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_PADDR_Y, 0);
	if (value == 0 || value == (int) 0xffffffff) {
		ALOGE("%s: Unable to get address", __func__);
		goto error;
	}
	
	exynos_camera->capture_memory_address = value;
	
	value = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_PADDR_CBCR, 0);
	
	if (EXYNOS_CAMERA_CALLBACK_DEFINED(request_memory)) {
		fd = exynos_v4l2_fd(exynos_camera, 0);
		if (fd < 0) {
			ALOGE("%s: Unable to get v4l2 fd for id %d", __func__, 0);
			goto error;
		}

		exynos_camera->capture_memory = NULL;

		memory = exynos_camera->callbacks.request_memory(fd, buffer_length, buffers_count, exynos_camera->callbacks.user);
		if (memory == NULL || memory->data == NULL || memory->data == MAP_FAILED) {
			ALOGE("%s: Unable to request memory", __func__);
			goto error;
		}

		exynos_camera->capture_memory = memory;
	} else {
		ALOGE("%s: No memory request function!", __func__);
		goto error;
	}

	for (i = 0; i < buffers_count; i++) {
		rc = exynos_v4l2_qbuf_cap(exynos_camera, 0, i);
		if (rc < 0) {
			ALOGE("%s: Unable to queue buffer", __func__);
			goto error;
		}
	}

	exynos_camera->capture_buffers_count = buffers_count;
	exynos_camera->capture_buffer_length = buffer_length;

	rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_ROTATION,
		exynos_camera->camera_rotation);
	if (rc < 0) {
		ALOGE("%s: Unable to set rotation", __func__);
		goto error;
	}

	rc = exynos_v4l2_streamon_cap(exynos_camera, 0);
	if (rc < 0) {
		ALOGE("%s: Unable to start stream", __func__);
		goto error;
	}

	exynos_camera->capture_enabled = 1;

	if(!is_capture){
		rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_CAPTURE_MODE, EXYNOS_CAMERA_UNKNOWN_CAPTURE_MODE);
		if (rc < 0) {
			ALOGE("%s: Unable to set V4L2_CID_CAMERA_CAPTURE_MODE", __func__);
			goto error;
		}
	}
	
	pthread_mutex_unlock(&exynos_camera->capture_lock_mutex);
	
	rc = 0;
	goto complete;

error:
	if (exynos_camera->capture_memory != NULL && exynos_camera->capture_memory->release != NULL) {
		exynos_camera->capture_memory->release(exynos_camera->capture_memory);
		exynos_camera->capture_memory = NULL;
	}

	rc = -1;

complete:
	return rc;
}

void exynos_camera_capture_stop(struct exynos_camera *exynos_camera)
{
	int rc;
	int i;

	if (exynos_camera == NULL)
		return;

	ALOGD("%s()", __func__);

	if (!exynos_camera->capture_enabled) {
		ALOGE("Capture was already stopped!");
		return;
	}

	rc = exynos_v4l2_streamoff_cap(exynos_camera, 0);
	if (rc < 0) {
		ALOGE("%s: Unable to stop stream", __func__);
	}	

	if (exynos_camera->capture_memory != NULL && exynos_camera->capture_memory->release != NULL) {
		exynos_camera->capture_memory->release(exynos_camera->capture_memory);
		exynos_camera->capture_memory = NULL;
	}

	if (exynos_camera->capture_yuv_buffer != NULL) {
		free(exynos_camera->capture_yuv_buffer);
		exynos_camera->capture_yuv_buffer = NULL;
	}

	if (exynos_camera->capture_jpeg_buffer != NULL) {
		free(exynos_camera->capture_jpeg_buffer);
		exynos_camera->capture_jpeg_buffer = NULL;
	}

	exynos_camera->capture_enabled = 0;
}

int exynos_camera_capture_setup(struct exynos_camera *exynos_camera)
{
	struct exynos_camera_capture_listener *listener;
	struct list_head *list;
	int width, height, format;
	int rc;

	if (exynos_camera == NULL)
		return -EINVAL;

	ALOGD("%s()", __func__);

	// No listener left
	if (exynos_camera->capture_listeners == NULL && exynos_camera->capture_enabled) {
	        ALOGE("%s: no listener left", __func__);
		exynos_camera_capture_stop(exynos_camera);
		return 0;
	}

	width = height = format = 0;

	list = (struct list_head *) exynos_camera->capture_listeners;
	while (list != NULL) {
		listener = (struct exynos_camera_capture_listener *) list;

		if (listener->width >= width && listener->height >= height) {
			width = listener->width;
			height = listener->height;
			format = listener->format;
		}

list_continue:
		list = list->next;
	}

	// Only picture is listening, but we need some preview size anyway
	if (format == V4L2_PIX_FMT_YUYV && (width == 0 || height == 0)) {
		width = exynos_camera->preview_width;
		height = exynos_camera->preview_height;
	}
	
	ALOGD("%s: Selected width: %d, height: %d, format: 0x%x", __func__, width, height, format);

	if (!exynos_camera->capture_enabled) {
	    exynos_camera->capture_width = width;
		exynos_camera->capture_height = height;
		exynos_camera->capture_format = format;

		rc = exynos_camera_capture_start(exynos_camera,0);
		if (rc < 0) {
			ALOGE("%s: Unable to start capture", __func__);
			return -1;
		}
	} else if ((exynos_camera->capture_width != width || exynos_camera->capture_height != height || exynos_camera->capture_format != format) 
	  && !exynos_camera->recording_thread_enabled) {
		exynos_camera_capture_stop(exynos_camera);

		exynos_camera->capture_width = width;
		exynos_camera->capture_height = height;
		exynos_camera->capture_format = format;

		rc = exynos_camera_capture_start(exynos_camera,1);
		if (rc < 0) {
			ALOGE("%s: Unable to start capture", __func__);
			return -1;
		}
	}

	return 0;
}

struct exynos_camera_capture_listener *exynos_camera_capture_listener_register(
	struct exynos_camera *exynos_camera, int width, int height, int format,
	int (*callback)(struct exynos_camera *exynos_camera, struct exynos_camera_buffer *buffers, int buffers_count))
{
	struct exynos_camera_capture_listener *listener = NULL;
	struct list_head *list_end;
	struct list_head *list;
	int rc;
	
	ALOGE("%s: ", __func__);

	if (exynos_camera == NULL || callback == NULL)
		return NULL;

	pthread_mutex_lock(&exynos_camera->capture_mutex);

	listener = calloc(1, sizeof(struct exynos_camera_capture_listener));
	if (listener == NULL)
		goto error;

	listener->width = width;
	listener->height = height;
	listener->format = format;
	listener->callback = callback;
	listener->busy = 0;

	list_end = (struct list_head *) exynos_camera->capture_listeners;
	while (list_end != NULL && list_end->next != NULL)
		list_end = list_end->next;

	list = (struct list_head *) listener;
	list_head_insert(list, list_end, NULL);

	if (exynos_camera->capture_listeners == NULL)
		exynos_camera->capture_listeners = listener;

	rc = exynos_camera_capture_setup(exynos_camera);
	if (rc < 0) {
		ALOGE("%s: Unable to setup capture", __func__);
		goto error;
	}

	rc = 0;
	goto complete;

error:
	listener = NULL;

complete:
	pthread_mutex_unlock(&exynos_camera->capture_mutex);

	return listener;
}

void exynos_camera_capture_listener_unregister(
	struct exynos_camera *exynos_camera,
	struct exynos_camera_capture_listener *listener)
{
	struct list_head *list;
	int rc;

	if (exynos_camera == NULL || listener == NULL)
		return;

	pthread_mutex_lock(&exynos_camera->capture_mutex);

	list = (struct list_head *) exynos_camera->capture_listeners;
	while (list != NULL) {
		if ((void *) list == (void *) listener) {
			list_head_remove(list);

			if ((void *) list == (void *) exynos_camera->capture_listeners)
				exynos_camera->capture_listeners = (struct exynos_camera_capture_listener *) list->next;

			memset(listener, 0, sizeof(struct exynos_camera_capture_listener));
			free(listener);

			break;
		}
list_continue:
		list = list->next;
	}

	rc = exynos_camera_capture_setup(exynos_camera);
	if (rc < 0) {
		ALOGE("%s: Unable to setup capture", __func__);
		goto complete;
	}

complete:
	pthread_mutex_unlock(&exynos_camera->capture_mutex);
}

// Preview

int exynos_camera_preview_output_start(struct exynos_camera *exynos_camera)
{
	struct exynos_v4l2_output *output;
	int rc;

	if (exynos_camera == NULL)
		return -EINVAL;

	ALOGD("%s()", __func__);

	if (exynos_camera->preview_output_enabled) {
		ALOGE("Preview was already started!");
		return -1;
	}

	output = &exynos_camera->preview_output;

	memset(output, 0, sizeof(struct exynos_v4l2_output));
	output->v4l2_id = 1;
	output->width = exynos_camera->preview_width;
	output->height = exynos_camera->preview_height;
	output->format = exynos_camera->preview_format;
	output->buffer_width = exynos_camera->preview_buffer.width;
	output->buffer_height = exynos_camera->preview_buffer.height;
	output->buffer_format = exynos_camera->preview_buffer.format;
	output->buffers_count = EXYNOS_CAMERA_PREVIEW_BUFFERS_COUNT;
		
	rc = exynos_v4l2_output_start(exynos_camera, output);
	if (rc < 0) {
		ALOGE("%s: Unable to start preview output", __func__);
		goto error;
	}
	exynos_camera->preview_output_enabled = 1;

	rc = 0;
	goto complete;

error:
	rc = -1;

complete:
	return rc;
}

void exynos_camera_preview_output_stop(struct exynos_camera *exynos_camera)
{
	struct exynos_v4l2_output *output;

	if (exynos_camera == NULL)
		return;

	ALOGD("%s()", __func__);

	if (!exynos_camera->preview_output_enabled) {
		ALOGE("Preview was already stopped!");
		return;
	}

	output = &exynos_camera->preview_output;

	exynos_v4l2_output_stop(exynos_camera, output);

	exynos_camera->preview_output_enabled = 0;
}

int exynos_camera_preview_callback(struct exynos_camera *exynos_camera,
	struct exynos_camera_buffer *buffers, int buffers_count)
{
	struct exynos_camera_buffer *buffer = NULL;
	int width, height, format;
	int buffer_width, buffer_height, buffer_format;
	int rc;
	int i;

	if (exynos_camera == NULL || buffers == NULL || buffers_count <= 0)
		return -EINVAL;

	//ALOGD("%s()", __func__);

	if (exynos_camera->preview_listener == NULL)
		return -1;

	if (exynos_camera->preview_listener->busy) {
		ALOGE("%s: Dropping buffer", __func__);
		return 0;
	}

	exynos_camera->preview_listener->busy = 1;

	width = exynos_camera->preview_width;
	height = exynos_camera->preview_height;
	format = exynos_camera->preview_format;

	for (i = 0; i < buffers_count; i++) {
		if (buffers->format == V4L2_PIX_FMT_JPEG)
			goto buffers_continue;

		// Optimal buffer
		if (buffers->width == width && buffers->height == height) {
			buffer = buffers;
			break;
		}
		
		// Might-work buffer, but not optimal
		buffer = buffers;

buffers_continue:
		buffers = (struct exynos_camera_buffer *) ((unsigned char *) buffers + sizeof(struct exynos_camera_buffer));
	}

	if (buffer == NULL) {
		ALOGE("%s: Unable to find an appropriate buffer for preview", __func__);
		exynos_camera->preview_listener->busy = 0;
		return 0;
	}

	buffer_width = buffer->width;
	buffer_height = buffer->height;
	buffer_format = buffer->format;

	pthread_mutex_lock(&exynos_camera->preview_mutex);

	if (buffer_width != width || buffer_height != height || buffer_format != format) {
		if (!exynos_camera->preview_output_enabled) {
			memcpy(&exynos_camera->preview_buffer, buffer, sizeof(struct exynos_camera_buffer));

			rc = exynos_camera_preview_output_start(exynos_camera);
			if (rc < 0) {
				ALOGE("%s: Unable to start preview", __func__);
				goto error;
			}
		} else if (exynos_camera->preview_buffer.width != buffer_width || exynos_camera->preview_buffer.height != buffer_height || exynos_camera->preview_buffer.format != buffer_format) {
			exynos_camera_preview_output_stop(exynos_camera);

			memcpy(&exynos_camera->preview_buffer, buffer, sizeof(struct exynos_camera_buffer));

			rc = exynos_camera_preview_output_start(exynos_camera);
			if (rc < 0) {
				ALOGE("%s: Unable to start preview", __func__);
				goto error;
			}
		} else {
			memcpy(&exynos_camera->preview_buffer, buffer, sizeof(struct exynos_camera_buffer));
		}
	} else {
		// The buffer format exactly matches our expectations
		if (exynos_camera->preview_output_enabled)
			exynos_camera_preview_output_stop(exynos_camera);

		memcpy(&exynos_camera->preview_buffer, buffer, sizeof(struct exynos_camera_buffer));
	}

	pthread_mutex_unlock(&exynos_camera->preview_lock_mutex);

	pthread_mutex_unlock(&exynos_camera->preview_mutex);

	rc = 0;
	goto complete;

error:
	pthread_mutex_unlock(&exynos_camera->preview_mutex);

	exynos_camera->preview_listener->busy = 0;

	rc = -1;

complete:
	return rc;
}

int exynos_camera_preview(struct exynos_camera *exynos_camera)
{
	struct exynos_v4l2_output *output;
	int width, height, format;
	buffer_handle_t *window_buffer;
	void *window_data;
	int window_stride;
	camera_memory_t *memory;
	void *memory_pointer;
	int memory_index;
	int memory_size;
	int rc;

	if (exynos_camera == NULL)
		goto error;

//	ALOGD("%s()", __func__);

	width = exynos_camera->preview_width;
	height = exynos_camera->preview_height;
	format = exynos_camera->preview_format;

	output = &exynos_camera->preview_output;

	if (exynos_camera->preview_output_enabled) {
		rc = exynos_v4l2_output(exynos_camera, output, exynos_camera->preview_buffer.address);
		if (rc < 0) {
			ALOGE("%s: Unable to output preview", __func__);
			goto error;
		}

		memory = output->memory;
		memory_index = output->memory_index;
		memory_pointer = (void *) ((unsigned char *) memory->data + output->buffer_length * memory_index);
		memory_size = output->buffer_length;
	} else {
		// In that case, we can directly use the capture memory
		memory = exynos_camera->capture_memory;
		memory_index = exynos_camera->capture_memory_index;
		memory_pointer = exynos_camera->preview_buffer.pointer;
		memory_size = exynos_camera->preview_buffer.length;
	}

	if (exynos_camera->preview_window != NULL && exynos_camera->gralloc != NULL) {
		exynos_camera->preview_window->dequeue_buffer(exynos_camera->preview_window, &window_buffer, &window_stride);
		exynos_camera->gralloc->lock(exynos_camera->gralloc, *window_buffer, GRALLOC_USAGE_SW_WRITE_OFTEN, 0, 0, width, height, &window_data);

		if (window_data == NULL) {
			ALOGE("%s: Unable to lock gralloc", __func__);
			goto error;
		}

		memcpy(window_data, memory_pointer, memory_size);

		exynos_camera->gralloc->unlock(exynos_camera->gralloc, *window_buffer);
		exynos_camera->preview_window->enqueue_buffer(exynos_camera->preview_window, window_buffer);
	}

	if (EXYNOS_CAMERA_MSG_ENABLED(CAMERA_MSG_PREVIEW_FRAME) && EXYNOS_CAMERA_CALLBACK_DEFINED(data) && !exynos_camera->callback_lock) {
		exynos_camera->callbacks.data(CAMERA_MSG_PREVIEW_FRAME, memory, memory_index, NULL, exynos_camera->callbacks.user);
	}

	if (exynos_camera->preview_output_enabled) {
		rc = exynos_v4l2_output_release(exynos_camera, output);
		if (rc < 0) {
			ALOGE("%s: Unable to release preview output", __func__);
			goto error;
		}
	}

	rc = 0;
	goto complete;

error:
	rc = -1;

complete:
	exynos_camera->preview_listener->busy = 0;

	return rc;
}

void *exynos_camera_preview_thread(void *data)
{
	struct exynos_camera *exynos_camera;
	int rc;

	if (data == NULL)
		return NULL;

	exynos_camera = (struct exynos_camera *) data;

	ALOGE("%s: Starting thread", __func__);
	exynos_camera->preview_thread_running = 1;

	while (exynos_camera->preview_thread_enabled) {
		pthread_mutex_lock(&exynos_camera->preview_lock_mutex);

		pthread_mutex_lock(&exynos_camera->preview_mutex);

		if (exynos_camera->preview_listener == NULL) {
			pthread_mutex_unlock(&exynos_camera->preview_mutex);
			break;
		}

		if (exynos_camera->preview_listener->busy) {
			rc = exynos_camera_preview(exynos_camera);
			if (rc < 0) {
				ALOGE("%s: Unable to preview", __func__);
				pthread_mutex_unlock(&exynos_camera->preview_mutex);
				break;
			}
		}

		pthread_mutex_unlock(&exynos_camera->preview_mutex);
	}

	exynos_camera->preview_thread_running = 0;
	ALOGE("%s: Exiting thread", __func__);

	return NULL;
}

int exynos_camera_preview_thread_start(struct exynos_camera *exynos_camera)
{
	struct exynos_camera_capture_listener *listener;
	pthread_attr_t thread_attr;
	int rc;

	if (exynos_camera == NULL)
		return -EINVAL;

	ALOGD("%s()", __func__);

	if (exynos_camera->preview_thread_enabled) {
		ALOGE("Preview thread was already started!");
		return -1;
	}

	pthread_mutex_init(&exynos_camera->preview_mutex, NULL);
	pthread_mutex_init(&exynos_camera->preview_lock_mutex, NULL);

	// Initial lock
	pthread_mutex_lock(&exynos_camera->preview_lock_mutex);

	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

	exynos_camera->preview_thread_enabled = 1;

	rc = pthread_create(&exynos_camera->preview_thread, &thread_attr, exynos_camera_preview_thread, (void *) exynos_camera);
	if (rc < 0) {
		ALOGE("%s: Unable to create thread", __func__);
		goto error;
	}

	listener = exynos_camera_capture_listener_register(exynos_camera, exynos_camera->preview_width, exynos_camera->preview_height, exynos_camera->preview_format, exynos_camera_preview_callback);
	if (listener == NULL) {
		ALOGE("%s: Unable to register capture listener", __func__);
		goto error;
	}

	exynos_camera->preview_listener = listener;

	rc = 0;
	goto complete;

error:
	pthread_mutex_destroy(&exynos_camera->preview_mutex);
	pthread_mutex_destroy(&exynos_camera->preview_lock_mutex);

	rc = -1;

complete:
	return rc;
}

void exynos_camera_preview_thread_stop(struct exynos_camera *exynos_camera)
{
	int i;

	if (exynos_camera == NULL)
		return;

	ALOGD("%s()", __func__);

	if (!exynos_camera->preview_thread_enabled) {
		ALOGE("Preview thread was already stopped!");
		return;
	}

	if (exynos_camera->preview_listener != NULL) {
		exynos_camera_capture_listener_unregister(exynos_camera, exynos_camera->preview_listener);
		exynos_camera->preview_listener = NULL;
	}

	exynos_camera->preview_thread_enabled = 0;

	pthread_mutex_unlock(&exynos_camera->preview_lock_mutex);

	// Wait for the thread to end
	i = 0;
	while (exynos_camera->preview_thread_running) {
		if (i++ > 10000) {
			ALOGE("Preview thread is taking too long to end, something is going wrong");
			break;
		}
		usleep(100);
	}

	if (exynos_camera->preview_output_enabled) {
		pthread_mutex_lock(&exynos_camera->preview_mutex);
		exynos_camera_preview_output_stop(exynos_camera);
		pthread_mutex_unlock(&exynos_camera->preview_mutex);
	}

	pthread_mutex_destroy(&exynos_camera->preview_mutex);
	pthread_mutex_destroy(&exynos_camera->preview_lock_mutex);

	// Invalidate the preview window
	exynos_camera->preview_window = NULL;
}

// Picture

int exynos_camera_picture_callback(struct exynos_camera *exynos_camera,
	struct exynos_camera_buffer *buffers, int buffers_count)
{
	struct exynos_camera_buffer *jpeg_buffer = NULL;
	struct exynos_camera_buffer *jpeg_thumbnail_buffer = NULL;
	struct exynos_camera_buffer *yuv_buffer = NULL;
	struct exynos_camera_buffer *yuv_thumbnail_buffer = NULL;
	int width, height;
	int thumbnail_width, thumbnail_height;
	int rc;
	int i;

	if (exynos_camera == NULL || buffers == NULL || buffers_count <= 0)
		return -EINVAL;

	ALOGD("%s()", __func__);

	width = exynos_camera->picture_width;
	height = exynos_camera->picture_height;
	thumbnail_width = exynos_camera->jpeg_thumbnail_width;
	thumbnail_height = exynos_camera->jpeg_thumbnail_height;

	if (exynos_camera->picture_completed){
		ALOGE("%s: exynos_camera->picture_completed", __func__);
		return -1;
	}

	if (exynos_camera->picture_listener == NULL){
	        ALOGE("%s: exynos_camera->picture_listener == NULL", __func__);
		return -1;
	}
	if (exynos_camera->picture_listener->busy) {
		ALOGE("%s: Dropping buffer", __func__);
		return 0;
	}

	pthread_mutex_lock(&exynos_camera->picture_mutex);

	if (!exynos_camera->picture_enabled) {
		rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_CAPTURE, 0);
		if (rc < 0) {
			ALOGE("%s: Unable to set capture", __func__);
			goto error;
		}

		exynos_camera->picture_enabled = 1;

		pthread_mutex_unlock(&exynos_camera->picture_mutex);
		return 0;
	}

	pthread_mutex_unlock(&exynos_camera->picture_mutex);

	exynos_camera->picture_listener->busy = 1;

	// Let's assume the picture format is JPEG

	for (i = 0; i < buffers_count; i++) {
		if (buffers->format == V4L2_PIX_FMT_JPEG) {
			if (buffers->width == width && buffers->height == height)
				jpeg_buffer = buffers;
			else if (buffers->width == thumbnail_width && buffers->height == thumbnail_height)
				jpeg_thumbnail_buffer = buffers;
		} else {
			if (buffers->width >= width && buffers->height >= height){
				yuv_buffer = buffers;
			}
			if (buffers->width >= thumbnail_width && buffers->height >= thumbnail_height){
				yuv_thumbnail_buffer = buffers;
			}
		}

buffers_continue:
		buffers = (struct exynos_camera_buffer *) ((unsigned char *) buffers + sizeof(struct exynos_camera_buffer));
	}

	if (jpeg_buffer == NULL && yuv_buffer == NULL) {
		ALOGE("%s: Unable to find an appropriate buffer for picture", __func__);
		exynos_camera->picture_listener->busy = 0;
		return 0;
	}

	pthread_mutex_lock(&exynos_camera->picture_mutex);

	if (jpeg_buffer == NULL){
		memset(&exynos_camera->picture_jpeg_buffer, 0, sizeof(exynos_camera->picture_jpeg_buffer));
	}else{
		memcpy(&exynos_camera->picture_jpeg_buffer, jpeg_buffer, sizeof(struct exynos_camera_buffer));
	}
	if (jpeg_thumbnail_buffer == NULL){
		memset(&exynos_camera->picture_jpeg_thumbnail_buffer, 0, sizeof(exynos_camera->picture_jpeg_thumbnail_buffer));
	}else{
		memcpy(&exynos_camera->picture_jpeg_thumbnail_buffer, jpeg_thumbnail_buffer, sizeof(struct exynos_camera_buffer));
	}
	if (yuv_buffer == NULL){
		memset(&exynos_camera->picture_yuv_buffer, 0, sizeof(exynos_camera->picture_yuv_buffer));
	}else{
		memcpy(&exynos_camera->picture_yuv_buffer, yuv_buffer, sizeof(struct exynos_camera_buffer));
	}
	if (yuv_thumbnail_buffer == NULL){
		memset(&exynos_camera->picture_yuv_thumbnail_buffer, 0, sizeof(exynos_camera->picture_yuv_thumbnail_buffer));
	}else{
		memcpy(&exynos_camera->picture_yuv_thumbnail_buffer, yuv_thumbnail_buffer, sizeof(struct exynos_camera_buffer));
	}
	pthread_mutex_unlock(&exynos_camera->picture_lock_mutex);

	pthread_mutex_unlock(&exynos_camera->picture_mutex);

	rc = 0;
	goto complete;

error:
	pthread_mutex_unlock(&exynos_camera->picture_mutex);

	exynos_camera->picture_listener->busy = 0;

	rc = -1;

complete:
	return rc;
}

int exynos_camera_picture(struct exynos_camera *exynos_camera)
{
	struct exynos_camera_buffer *jpeg_buffer;
	struct exynos_camera_buffer *jpeg_thumbnail_buffer;
	struct exynos_camera_buffer *yuv_buffer;
	struct exynos_camera_buffer *yuv_thumbnail_buffer;
	struct exynos_v4l2_output output;
	struct exynos_jpeg jpeg;
	struct exynos_exif exif;
	int output_enabled = 0;
	int width, height, format;
	int buffer_width, buffer_height, buffer_format, buffer_address;
	camera_memory_t *memory = NULL;
	int memory_size;
	unsigned char *p;
	camera_memory_t *jpeg_memory = NULL;
	void *jpeg_data = NULL;
	int jpeg_size = 0;
	camera_memory_t *jpeg_thumbnail_memory = NULL;
	void *jpeg_thumbnail_data = NULL;
	int jpeg_thumbnail_size = 0;
	void *yuv_data = NULL;
	int yuv_address;
	int yuv_size = 0;
	void *yuv_thumbnail_data = NULL;
	int yuv_thumbnail_address;
	int yuv_thumbnail_size = 0;
	int rc;

	if (exynos_camera == NULL)
		goto error;

	ALOGD("%s()", __func__);

	jpeg_buffer = &exynos_camera->picture_jpeg_buffer;
	jpeg_thumbnail_buffer = &exynos_camera->picture_jpeg_thumbnail_buffer;
	yuv_buffer = &exynos_camera->picture_yuv_buffer;
	yuv_thumbnail_buffer = &exynos_camera->picture_yuv_thumbnail_buffer;

	if (jpeg_buffer->pointer != NULL && jpeg_buffer->length > 0) {
		jpeg_data = jpeg_buffer->pointer;
		jpeg_size = jpeg_buffer->length;
	}

	if (jpeg_thumbnail_buffer->pointer != NULL && jpeg_thumbnail_buffer->length > 0) {
		jpeg_thumbnail_data = jpeg_thumbnail_buffer->pointer;
		jpeg_thumbnail_size = jpeg_thumbnail_buffer->length;
	}

	if (yuv_buffer->pointer != NULL && yuv_buffer->length > 0) {
		yuv_data = yuv_buffer->pointer;
		yuv_address = yuv_buffer->address;
		yuv_size = yuv_buffer->length;
	}

	if (yuv_thumbnail_buffer->pointer != NULL && yuv_thumbnail_buffer->length > 0) {
		yuv_thumbnail_data = yuv_thumbnail_buffer->pointer;
		yuv_thumbnail_address = yuv_thumbnail_buffer->address;
		yuv_thumbnail_size = yuv_thumbnail_buffer->length;
	}

	// JPEG

	if (jpeg_data == NULL) {
		if (yuv_data == NULL || yuv_size <= 0) {
			ALOGE("%s: Unable to create jpeg without an YUV buffer", __func__);
			goto error;
		}

		width = exynos_camera->picture_width;
		height = exynos_camera->picture_height;
		format = yuv_buffer->format;

		buffer_width = yuv_buffer->width;
		buffer_height = yuv_buffer->height;
		buffer_format = yuv_buffer->format;
		buffer_address = yuv_buffer->address;

		if (width != buffer_width && height != buffer_height) {
			format = EXYNOS_CAMERA_PICTURE_OUTPUT_FORMAT;

			memset(&output, 0, sizeof(output));
			output.v4l2_id = 2;
			output.width = width;
			output.height = height;
			output.format = format;
			output.buffer_width = buffer_width;
			output.buffer_height = buffer_height;
			output.buffer_format = buffer_format;
			output.buffers_count = 1;

			rc = exynos_v4l2_output_start(exynos_camera, &output);
			if (rc < 0) {
				ALOGE("%s: Unable to start picture output", __func__);
				goto error;
			}

			rc = exynos_v4l2_output(exynos_camera, &output, buffer_address);
			if (rc < 0) {
				ALOGE("%s: Unable to output picture", __func__);
				goto error;
			}

			output_enabled = 1;

			yuv_data = output.memory->data;
			yuv_address = output.memory_address;
			yuv_size = output.buffer_length;
		}

		memset(&jpeg, 0, sizeof(jpeg));
		jpeg.width = width;
		jpeg.height = height;
		jpeg.format = format;
		jpeg.quality = exynos_camera->jpeg_quality;

		rc = exynos_jpeg_start(exynos_camera, &jpeg);
		if (rc < 0) {
			ALOGE("%s: Unable to start jpeg", __func__);
			goto error;
		}

		if (jpeg.memory_in_pointer == NULL) {
			ALOGE("%s: Invalid memory input pointer", __func__);
			goto error;
		}

		memcpy(jpeg.memory_in_pointer, yuv_data, yuv_size);

		rc = exynos_jpeg(exynos_camera, &jpeg);
		if (rc < 0) {
			ALOGE("%s: Unable to jpeg", __func__);
			goto error;
		}

		jpeg_size = jpeg.memory_out_size;
		if (jpeg_size <= 0) {
			ALOGE("%s: Invalid jpeg size", __func__);
			goto error;
		}

		if (EXYNOS_CAMERA_CALLBACK_DEFINED(request_memory)) {
			jpeg_memory = exynos_camera->callbacks.request_memory(-1, jpeg_size, 1, exynos_camera->callbacks.user);
			if (jpeg_memory == NULL || jpeg_memory->data == NULL || jpeg_memory->data == MAP_FAILED) {
				ALOGE("%s: Unable to request memory", __func__);
				goto error;
			}
		} else {
			ALOGE("%s: No memory request function!", __func__);
			goto error;
		}

		jpeg_data = jpeg_memory->data;

		memcpy(jpeg_data, jpeg.memory_out_pointer, jpeg_size);

		exynos_jpeg_stop(exynos_camera, &jpeg);

		if (output_enabled) {
			exynos_v4l2_output_stop(exynos_camera, &output);
			output_enabled = 0;
		}
	}

	// Thumbnail

	if (jpeg_thumbnail_data == NULL) {
		if (yuv_thumbnail_data == NULL || yuv_thumbnail_size <= 0) {
			ALOGE("%s: Unable to create jpeg thumbnail without an YUV buffer", __func__);
			goto error;
		}

		width = exynos_camera->jpeg_thumbnail_width;
		height = exynos_camera->jpeg_thumbnail_height;
		format = yuv_thumbnail_buffer->format;

		buffer_width = yuv_thumbnail_buffer->width;
		buffer_height = yuv_thumbnail_buffer->height;
		buffer_format = yuv_thumbnail_buffer->format;
		buffer_address = yuv_thumbnail_buffer->address;

		if (width != buffer_width && height != buffer_height) {
			format = EXYNOS_CAMERA_PICTURE_OUTPUT_FORMAT;

			memset(&output, 0, sizeof(output));
			output.v4l2_id = 2;
			output.width = width;
			output.height = height;
			output.format = format;
			output.buffer_width = buffer_width;
			output.buffer_height = buffer_height;
			output.buffer_format = buffer_format;
			output.buffers_count = 1;

			rc = exynos_v4l2_output_start(exynos_camera, &output);
			if (rc < 0) {
				ALOGE("%s: Unable to start thumbnail picture output", __func__);
				goto error;
			}

			output_enabled = 1;

			rc = exynos_v4l2_output(exynos_camera, &output, buffer_address);
			if (rc < 0) {
				ALOGE("%s: Unable to output thumbnail picture", __func__);
				goto error;
			}

			yuv_thumbnail_data = output.memory->data;
			yuv_thumbnail_address = output.memory_address;
			yuv_thumbnail_size = output.buffer_length;
		}

		memset(&jpeg, 0, sizeof(jpeg));
		jpeg.width = width;
		jpeg.height = height;
		jpeg.format = format;
		jpeg.quality = exynos_camera->jpeg_thumbnail_quality;

		rc = exynos_jpeg_start(exynos_camera, &jpeg);
		if (rc < 0) {
			ALOGE("%s: Unable to start jpeg", __func__);
			goto error;
		}

		if (jpeg.memory_in_pointer == NULL) {
			ALOGE("%s: Invalid memory input pointer", __func__);
			goto error;
		}

		memcpy(jpeg.memory_in_pointer, yuv_thumbnail_data, yuv_thumbnail_size);

		rc = exynos_jpeg(exynos_camera, &jpeg);
		if (rc < 0) {
			ALOGE("%s: Unable to jpeg", __func__);
			goto error;
		}

		jpeg_thumbnail_size = jpeg.memory_out_size;
		if (jpeg_thumbnail_size <= 0) {
			ALOGE("%s: Invalid jpeg size", __func__);
			goto error;
		}

		if (EXYNOS_CAMERA_CALLBACK_DEFINED(request_memory)) {
			jpeg_thumbnail_memory = exynos_camera->callbacks.request_memory(-1, jpeg_thumbnail_size, 1, exynos_camera->callbacks.user);
			if (jpeg_thumbnail_memory == NULL || jpeg_thumbnail_memory->data == NULL || jpeg_thumbnail_memory->data == MAP_FAILED) {
				ALOGE("%s: Unable to request memory", __func__);
				goto error;
			}
		} else {
			ALOGE("%s: No memory request function!", __func__);
			goto error;
		}

		jpeg_thumbnail_data = jpeg_thumbnail_memory->data;

		memcpy(jpeg_thumbnail_data, jpeg.memory_out_pointer, jpeg_thumbnail_size);

		exynos_jpeg_stop(exynos_camera, &jpeg);

		if (output_enabled) {
			exynos_v4l2_output_stop(exynos_camera, &output);
			output_enabled = 0;
		}
	}

	// EXIF

	memset(&exif, 0, sizeof(exif));
	exif.jpeg_thumbnail_data = jpeg_thumbnail_data;
	exif.jpeg_thumbnail_size = jpeg_thumbnail_size;

	rc = exynos_exif_start(exynos_camera, &exif);
	if (rc < 0) {
		ALOGE("%s: Unable to start exif", __func__);
		goto error;
	}

	rc = exynos_exif(exynos_camera, &exif);
	if (rc < 0) {
		ALOGE("%s: Unable to exif", __func__);
		goto error;
	}

	memory_size = exif.memory_size + jpeg_size;

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

	p = (unsigned char *) memory->data;

	// Copy the first two bytes of the JPEG picture
	memcpy(p, jpeg_data, 2);
	p += 2;

	// Copy the EXIF data
	memcpy(p, exif.memory->data, exif.memory_size);
	p += exif.memory_size;

	// Copy the JPEG picture
	memcpy(p, (void *) ((unsigned char *) jpeg_data + 2), jpeg_size - 2);

	exynos_exif_stop(exynos_camera, &exif);

	exynos_camera->picture_memory = memory;

	rc = 0;
	goto complete;

error:
	if (output_enabled)
		exynos_v4l2_output_stop(exynos_camera, &output);

	if (memory != NULL && memory->release != NULL) {
		memory->release(memory);
		exynos_camera->picture_memory = NULL;
	}

	if (EXYNOS_CAMERA_MSG_ENABLED(CAMERA_MSG_ERROR) && EXYNOS_CAMERA_CALLBACK_DEFINED(notify) && !exynos_camera->callback_lock)
		exynos_camera->callbacks.notify(CAMERA_MSG_ERROR, -1, 0, exynos_camera->callbacks.user);

	rc = -1;

complete:
	if (jpeg_memory != NULL && jpeg_memory->release != NULL)
		jpeg_memory->release(jpeg_memory);

	if (jpeg_thumbnail_memory != NULL && jpeg_thumbnail_memory->release != NULL)
		jpeg_thumbnail_memory->release(jpeg_thumbnail_memory);

	exynos_camera->picture_completed = 1;
	exynos_camera->picture_listener->busy = 0;

	return rc;
}

void *exynos_camera_picture_thread(void *data)
{
	struct exynos_camera *exynos_camera;
	int rc;

	if (data == NULL)
		return NULL;

	exynos_camera = (struct exynos_camera *) data;

	ALOGE("%s: Starting thread", __func__);
	exynos_camera->picture_thread_running = 1;

        exynos_camera->picture_completed = 0;

	while (exynos_camera->picture_thread_enabled) {
		pthread_mutex_lock(&exynos_camera->picture_lock_mutex);
		pthread_mutex_lock(&exynos_camera->picture_mutex);

		if (exynos_camera->picture_listener == NULL) {
			pthread_mutex_unlock(&exynos_camera->picture_mutex);
			break;
		}
		
		if (exynos_camera->picture_listener->busy) {
			rc = exynos_camera_picture(exynos_camera);
			if (rc < 0) {
				ALOGE("%s: Unable to take picture", __func__);
				pthread_mutex_unlock(&exynos_camera->picture_mutex);
				break;
			}
		}

		pthread_mutex_unlock(&exynos_camera->picture_mutex);

		if (exynos_camera->picture_completed) {
		        ALOGE("%s: exynos_camera->picture_completed", __func__);
			exynos_camera->picture_thread_running = 0;
			exynos_camera_picture_thread_stop(exynos_camera);
			break;
		}
	}

	exynos_camera->picture_thread_running = 0;
	ALOGE("%s: Exiting thread", __func__);

	return NULL;
}

int exynos_camera_picture_thread_start(struct exynos_camera *exynos_camera)
{
	struct exynos_camera_capture_listener *listener;
	pthread_attr_t thread_attr;
	int format;
	int rc;

	if (exynos_camera == NULL)
		return -EINVAL;

	ALOGD("%s()", __func__);

	if (exynos_camera->picture_thread_enabled) {
		ALOGE("Picture thread was already started!");
		return 0;
	}

	if (exynos_camera->camera_picture_format)
		format = exynos_camera->camera_picture_format;
	else
		format = exynos_camera->picture_format;
	
	pthread_mutex_init(&exynos_camera->picture_mutex, NULL);

	pthread_mutex_init(&exynos_camera->picture_lock_mutex, NULL);

	// Initial lock
	pthread_mutex_lock(&exynos_camera->picture_lock_mutex);

	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

	exynos_camera->picture_thread_enabled = 1;

	rc = pthread_create(&exynos_camera->picture_thread, &thread_attr, exynos_camera_picture_thread, (void *) exynos_camera);
	if (rc < 0) {
		ALOGE("%s: Unable to create thread", __func__);
		goto error;
	}

	format = exynos_camera->picture_format;
		
	listener = exynos_camera_capture_listener_register(exynos_camera, exynos_camera->picture_width, exynos_camera->picture_height, format, exynos_camera_picture_callback);
	if (listener == NULL) {
		ALOGE("%s: Unable to register picture capture listener", __func__);
		goto error;
	}

	exynos_camera->picture_listener = listener;
	exynos_camera->picture_enabled = 1;

	rc = 0;
	goto complete;

error:
	pthread_mutex_destroy(&exynos_camera->picture_mutex);
	pthread_mutex_destroy(&exynos_camera->picture_lock_mutex);

	rc = -1;

complete:
	return rc;
}

void exynos_camera_picture_thread_stop(struct exynos_camera *exynos_camera)
{
	camera_memory_t *memory;
	int i;

	if (exynos_camera == NULL)
		return;

	ALOGD("%s()", __func__);

	if (!exynos_camera->picture_thread_enabled) {
		ALOGE("Picture thread was already stopped!");
		return;
	}

	memory = exynos_camera->picture_memory;

	if (exynos_camera->picture_listener != NULL) {
		exynos_camera_capture_listener_unregister(exynos_camera, exynos_camera->picture_listener);
		exynos_camera->picture_listener = NULL;
	}

	exynos_camera->picture_thread_enabled = 0;

	pthread_mutex_unlock(&exynos_camera->picture_lock_mutex);

	// Wait for the thread to end
	i = 0;
	while (exynos_camera->picture_thread_running) {
		if (i++ > 10000) {
			ALOGE("Picture thread is taking too long to end, something is going wrong");
			break;
		}
		usleep(100);
	}

	if (exynos_camera->picture_enabled) {
		pthread_mutex_lock(&exynos_camera->picture_mutex);
		exynos_camera->picture_enabled = 0;
		pthread_mutex_unlock(&exynos_camera->picture_mutex);
	}

	pthread_mutex_destroy(&exynos_camera->picture_mutex);
	pthread_mutex_destroy(&exynos_camera->picture_lock_mutex);

	if (exynos_camera->picture_completed && memory != NULL) {
		// It is important to return at this point (and not before) for burst

		if (EXYNOS_CAMERA_MSG_ENABLED(CAMERA_MSG_SHUTTER) && EXYNOS_CAMERA_CALLBACK_DEFINED(notify) && !exynos_camera->callback_lock)
			exynos_camera->callbacks.notify(CAMERA_MSG_SHUTTER, 0, 0, exynos_camera->callbacks.user);

		if (EXYNOS_CAMERA_MSG_ENABLED(CAMERA_MSG_COMPRESSED_IMAGE) && EXYNOS_CAMERA_CALLBACK_DEFINED(data) && !exynos_camera->callback_lock)
			exynos_camera->callbacks.data(CAMERA_MSG_COMPRESSED_IMAGE, memory, 0, NULL, exynos_camera->callbacks.user);

		if (memory->release != NULL) {
			memory->release(memory);
			exynos_camera->picture_memory = NULL;
		}
	}
}

// Recording

int exynos_camera_recording_output_start(struct exynos_camera *exynos_camera)
{
	struct exynos_v4l2_output *output;
	int rc;

	if (exynos_camera == NULL)
		return -EINVAL;

	ALOGD("%s()", __func__);

	if (exynos_camera->recording_output_enabled) {
		ALOGE("Recording was already started!");
		return -1;
	}

	output = &exynos_camera->recording_output;

	memset(output, 0, sizeof(struct exynos_v4l2_output));
	output->v4l2_id = 3;
	output->width = exynos_camera->recording_width;
	output->height = exynos_camera->recording_height;
	output->format = exynos_camera->recording_format;
	output->buffer_width = exynos_camera->recording_buffer.width;
	output->buffer_height = exynos_camera->recording_buffer.height;
	output->buffer_format = exynos_camera->recording_buffer.format;
	output->buffers_count = EXYNOS_CAMERA_RECORDING_BUFFERS_COUNT;

	rc = exynos_v4l2_output_start(exynos_camera, output);
	if (rc < 0) {
		ALOGE("%s: Unable to start recording output", __func__);
		goto error;
	}

	exynos_camera->recording_output_enabled = 1;

	rc = 0;
	goto complete;

error:
	rc = -1;

complete:
	return rc;
}

void exynos_camera_recording_output_stop(struct exynos_camera *exynos_camera)
{
	struct exynos_v4l2_output *output;

	if (exynos_camera == NULL)
		return;

	ALOGD("%s()", __func__);

	if (!exynos_camera->recording_output_enabled) {
		ALOGE("Recording was already stopped!");
		return;
	}

	output = &exynos_camera->recording_output;

	exynos_v4l2_output_stop(exynos_camera, output);

	exynos_camera->recording_output_enabled = 0;
}

int exynos_camera_recording_callback(struct exynos_camera *exynos_camera,
	struct exynos_camera_buffer *buffers, int buffers_count)
{
	struct exynos_camera_buffer *buffer = NULL;
	int width, height, format;
	int buffer_width, buffer_height, buffer_format;
	int rc;
	int i;

	if (exynos_camera == NULL || buffers == NULL || buffers_count <= 0)
		return -EINVAL;

	//ALOGD("%s()", __func__);

	if (exynos_camera->recording_listener == NULL)
		return -1;

	if (exynos_camera->recording_listener->busy) {
		ALOGE("%s: Dropping buffer", __func__);
		return 0;
	}

	exynos_camera->recording_listener->busy = 1;

	width = exynos_camera->recording_width;
	height = exynos_camera->recording_height;
	format = exynos_camera->recording_format;

	for (i = 0; i < buffers_count; i++) {
		if (buffers->format == V4L2_PIX_FMT_JPEG){
			goto buffers_continue;}

		// Optimal buffer
		if (buffers->width == width && buffers->height == height) {
			buffer = buffers;
			break;
		}

		// Might-work buffer, but not optimal
		buffer = buffers;

buffers_continue:
		buffers = (struct exynos_camera_buffer *) ((unsigned char *) buffers + sizeof(struct exynos_camera_buffer));
	}

	if (buffer == NULL) {
		ALOGE("%s: Unable to find an appropriate buffer for recording", __func__);
		exynos_camera->recording_listener->busy = 0;
		return 0;
	}

	buffer_width = buffer->width;
	buffer_height = buffer->height;
	buffer_format = buffer->format;

	pthread_mutex_lock(&exynos_camera->recording_mutex);

	if (!exynos_camera->recording_output_enabled) {
		memcpy(&exynos_camera->recording_buffer, buffer, sizeof(struct exynos_camera_buffer));

		rc = exynos_camera_recording_output_start(exynos_camera);
		if (rc < 0) {
			ALOGE("%s: Unable to start recording", __func__);
			goto error;
		}
	} else if (exynos_camera->recording_buffer.width != buffer_width || exynos_camera->recording_buffer.height != buffer_height || exynos_camera->recording_buffer.format != buffer_format) {
		exynos_camera_recording_output_stop(exynos_camera);

		memcpy(&exynos_camera->recording_buffer, buffer, sizeof(struct exynos_camera_buffer));

		rc = exynos_camera_recording_output_start(exynos_camera);
		if (rc < 0) {
			ALOGE("%s: Unable to start recording", __func__);
			goto error;
		}
	} else {
		memcpy(&exynos_camera->recording_buffer, buffer, sizeof(struct exynos_camera_buffer));
	}

	pthread_mutex_unlock(&exynos_camera->recording_lock_mutex);

	pthread_mutex_unlock(&exynos_camera->recording_mutex);

	rc = 0;
	goto complete;

error:
	pthread_mutex_unlock(&exynos_camera->recording_mutex);

	exynos_camera->recording_listener->busy = 0;

	rc = -1;

complete:
	return rc;
}

void exynos_camera_recording_frame_release(struct exynos_camera *exynos_camera)
{
	struct exynos_v4l2_output *output;
	int rc;

	if (exynos_camera == NULL)
		return;

//	ALOGD("%s()", __func__);

	output = &exynos_camera->recording_output;

	if (!exynos_camera->recording_output_enabled) {
		ALOGE("%s: Recording output should always be enabled", __func__);
		return;
	}

	rc = exynos_v4l2_output_release(exynos_camera, output);
	if (rc < 0) {
		ALOGE("%s: Unable to release recording output", __func__);
		return;
	}
}

int exynos_camera_recording(struct exynos_camera *exynos_camera)
{
	struct exynos_v4l2_output *output;
	struct exynos_camera_addrs *addrs;
	int width, height, format;
	camera_memory_t *memory;
	int memory_address;
	int memory_index;
	int buffer_length;
	int buffers_count;
	nsecs_t timestamp;
	int rc;

	if (exynos_camera == NULL)
		goto error;

	ALOGD("%s()", __func__);

	width = exynos_camera->recording_width;
	height = exynos_camera->recording_height;
	format = exynos_camera->recording_format;

	output = &exynos_camera->recording_output;

	buffer_length = exynos_camera->recording_buffer_length;
	buffers_count = exynos_camera->recording_buffers_count;

	timestamp = systemTime(1);

	if (!exynos_camera->recording_output_enabled) {
		ALOGE("%s: Recording output should always be enabled", __func__);
		goto error;
	}

	rc = exynos_v4l2_output(exynos_camera, output, exynos_camera->recording_buffer.address);
	if (rc < 0) {
		ALOGE("%s: Unable to output recording", __func__);
		goto error;
	}

	if (exynos_camera->recording_metadata) {
		memory = exynos_camera->recording_memory;
		memory_index = exynos_camera->recording_memory_index;
		memory_address = output->memory_address + output->buffer_length * output->memory_index;

		addrs = (struct exynos_camera_addrs *) ((unsigned char *) memory->data + buffer_length * memory_index);
		memset(addrs, 0, sizeof(struct exynos_camera_addrs));
		addrs->type = 0; // kMetadataBufferTypeCameraSource
		addrs->index = memory_index;

		exynos_camera_yuv_planes(width, height, format, memory_address, (int *) &addrs->y, (int *) &addrs->cbcr, NULL);
	} else {
		memory = output->memory;
		memory_index = output->memory_index;
	}

	if (EXYNOS_CAMERA_MSG_ENABLED(CAMERA_MSG_VIDEO_FRAME) && EXYNOS_CAMERA_CALLBACK_DEFINED(data_timestamp) && !exynos_camera->callback_lock)
		exynos_camera->callbacks.data_timestamp(timestamp, CAMERA_MSG_VIDEO_FRAME, memory, memory_index, exynos_camera->callbacks.user);
	else
		exynos_camera_recording_frame_release(exynos_camera);

	if (exynos_camera->recording_metadata) {
		memory_index++;
		exynos_camera->recording_memory_index = memory_index % buffers_count;
	}

	rc = 0;
	goto complete;

error:
	rc = -1;

complete:
	exynos_camera->recording_listener->busy = 0;

	return rc;
}

void *exynos_camera_recording_thread(void *data)
{
	struct exynos_camera *exynos_camera;
	int rc;

	if (data == NULL)
		return NULL;

	exynos_camera = (struct exynos_camera *) data;

	ALOGE("%s: Starting thread", __func__);
	exynos_camera->recording_thread_running = 1;

	while (exynos_camera->recording_thread_enabled) {
		pthread_mutex_lock(&exynos_camera->recording_lock_mutex);

		pthread_mutex_lock(&exynos_camera->recording_mutex);

		if (exynos_camera->recording_listener == NULL) {
			pthread_mutex_unlock(&exynos_camera->recording_mutex);
			break;
		}

		if (exynos_camera->recording_listener->busy) {
			rc = exynos_camera_recording(exynos_camera);
			if (rc < 0) {
				ALOGE("%s: Unable to record", __func__);
				pthread_mutex_unlock(&exynos_camera->recording_mutex);
				break;
			}
		}

		pthread_mutex_unlock(&exynos_camera->recording_mutex);
	}

	exynos_camera->recording_thread_running = 0;
	ALOGE("%s: Exiting thread", __func__);

	return NULL;
}

int exynos_camera_recording_thread_start(struct exynos_camera *exynos_camera)
{
	struct exynos_camera_capture_listener *listener;
	pthread_attr_t thread_attr;
	camera_memory_t *memory = NULL;
	int buffer_length;
	int buffers_count;
	int rc;

	if (exynos_camera == NULL)
		return -EINVAL;

	ALOGD("%s()", __func__);

	if (exynos_camera->recording_thread_enabled) {
		ALOGE("Recording thread was already started!");
		return -1;
	}

	pthread_mutex_init(&exynos_camera->recording_mutex, NULL);
	pthread_mutex_init(&exynos_camera->recording_lock_mutex, NULL);

	// Initial lock
	pthread_mutex_lock(&exynos_camera->recording_lock_mutex);

	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

	exynos_camera->recording_thread_enabled = 1;

	rc = pthread_create(&exynos_camera->recording_thread, &thread_attr, exynos_camera_recording_thread, (void *) exynos_camera);
	if (rc < 0) {
		ALOGE("%s: Unable to create thread", __func__);
		goto error;
	}
	if (exynos_camera->recording_metadata) {
		buffer_length = sizeof(struct exynos_camera_addrs);
		buffers_count = EXYNOS_CAMERA_RECORDING_BUFFERS_COUNT;

		if (EXYNOS_CAMERA_CALLBACK_DEFINED(request_memory)) {
			memory = exynos_camera->callbacks.request_memory(-1, buffer_length, buffers_count, exynos_camera->callbacks.user);
			if (memory == NULL || memory->data == NULL || memory->data == MAP_FAILED) {
				ALOGE("%s: Unable to request memory", __func__);
				goto error;
			}
		} else {
			ALOGE("%s: No memory request function!", __func__);
			goto error;
		}

		exynos_camera->recording_memory = memory;
		exynos_camera->recording_buffer_length = buffer_length;
		exynos_camera->recording_buffers_count = buffers_count;
	}

	listener = exynos_camera_capture_listener_register(exynos_camera, exynos_camera->recording_width, exynos_camera->recording_height, exynos_camera->recording_format, exynos_camera_recording_callback);
	if (listener == NULL) {
		ALOGE("%s: Unable to register recording capture listener", __func__);
		goto error;
	}

	exynos_camera->recording_listener = listener;

	rc = 0;
	goto complete;

error:
	if (memory != NULL && memory->release != NULL) {
		memory->release(memory);
		exynos_camera->recording_memory = NULL;
	}

	pthread_mutex_destroy(&exynos_camera->recording_mutex);
	pthread_mutex_destroy(&exynos_camera->recording_lock_mutex);

	rc = -1;

complete:
	return rc;
}

void exynos_camera_recording_thread_stop(struct exynos_camera *exynos_camera)
{
	int i;

	if (exynos_camera == NULL)
		return;

	ALOGD("%s()", __func__);

	if (!exynos_camera->recording_thread_enabled) {
		ALOGE("Recording thread was already stopped!");
		return;
	}

	if (exynos_camera->recording_listener != NULL) {
		exynos_camera_capture_listener_unregister(exynos_camera, exynos_camera->recording_listener);
		exynos_camera->recording_listener = NULL;
	}

	exynos_camera->recording_thread_enabled = 0;

	pthread_mutex_unlock(&exynos_camera->recording_lock_mutex);

	// Wait for the thread to end
	i = 0;
	while (exynos_camera->recording_thread_running) {
		if (i++ > 10000) {
			ALOGE("Recording thread is taking too long to end, something is going wrong");
			break;
		}
		usleep(100);
	}

	if (exynos_camera->recording_output_enabled) {
		pthread_mutex_lock(&exynos_camera->recording_mutex);
		exynos_camera_recording_output_stop(exynos_camera);
		pthread_mutex_unlock(&exynos_camera->recording_mutex);
	}

	pthread_mutex_destroy(&exynos_camera->recording_mutex);
	pthread_mutex_destroy(&exynos_camera->recording_lock_mutex);
}

// Auto-focus

int exynos_camera_auto_focus(struct exynos_camera *exynos_camera, int auto_focus_status)
{
	if (exynos_camera == NULL)
		return -EINVAL;

//	ALOGD("%s()", __func__);

	switch (auto_focus_status) {
	  case ISX012_AUTO_FOCUS_IN_PROGRESS:
	  case CAMERA_AF_STATUS_IN_PROGRESS:
/*
			if (EXYNOS_CAMERA_MSG_ENABLED(CAMERA_MSG_FOCUS_MOVE) && EXYNOS_CAMERA_CALLBACK_DEFINED(notify) && !exynos_camera->callback_lock)
				exynos_camera->callbacks.notify(CAMERA_MSG_FOCUS_MOVE, 1, 0, exynos_camera->callbacks.user);
*/
			break;
			
	  case CAMERA_AF_STATUS_SUCCESS:
			if (EXYNOS_CAMERA_MSG_ENABLED(CAMERA_MSG_FOCUS) && EXYNOS_CAMERA_CALLBACK_DEFINED(notify) && !exynos_camera->callback_lock)
				exynos_camera->callbacks.notify(CAMERA_MSG_FOCUS, 1, 0, exynos_camera->callbacks.user);
			break;
			
	  case CAMERA_AF_STATUS_FAIL:
		default:
			if (EXYNOS_CAMERA_MSG_ENABLED(CAMERA_MSG_FOCUS) && EXYNOS_CAMERA_CALLBACK_DEFINED(notify) && !exynos_camera->callback_lock)
				exynos_camera->callbacks.notify(CAMERA_MSG_FOCUS, 0, 0, exynos_camera->callbacks.user);
			break;
	}

	return 0;
}

void *exynos_camera_auto_focus_thread(void *data)
{
	struct exynos_camera *exynos_camera;
	int auto_focus_status = CAMERA_AF_STATUS_FAIL;
	int auto_focus_completed = 0;
	int rc;

	if (data == NULL)
		return NULL;

	exynos_camera = (struct exynos_camera *) data;

	ALOGE("%s: Starting thread", __func__);
	exynos_camera->auto_focus_thread_running = 1;

	while (exynos_camera->auto_focus_thread_enabled) {
		pthread_mutex_lock(&exynos_camera->auto_focus_mutex);

		rc = exynos_v4l2_g_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_AUTO_FOCUS_RESULT, &auto_focus_status);
		if (rc < 0) {
			ALOGE("%s: Unable to get auto-focus result", __func__);
			    auto_focus_status = CAMERA_AF_STATUS_FAIL;
		}

		rc = exynos_camera_auto_focus(exynos_camera, auto_focus_status);
		if (rc < 0) {
			ALOGE("%s: Unable to auto-focus", __func__);
			     auto_focus_status = CAMERA_AF_STATUS_FAIL;
		}

		if (auto_focus_status == ISX012_AUTO_FOCUS_IN_PROGRESS || auto_focus_status == CAMERA_AF_STATUS_IN_PROGRESS)
		        usleep(10000);
		else
			auto_focus_completed = 1;


		pthread_mutex_unlock(&exynos_camera->auto_focus_mutex);

		if (auto_focus_completed) {
			exynos_camera->auto_focus_thread_running = 0;
			exynos_camera_auto_focus_thread_stop(exynos_camera);
		}
	}

	exynos_camera->auto_focus_thread_running = 0;
	ALOGE("%s: Exiting thread", __func__);

	return NULL;
}

int exynos_camera_auto_focus_thread_start(struct exynos_camera *exynos_camera)
{
	pthread_attr_t thread_attr;
	int auto_focus;
	int rc;

	if (exynos_camera == NULL)
		return -EINVAL;

	ALOGD("%s()", __func__);

	if (exynos_camera->auto_focus_thread_enabled) {
		ALOGE("Auto-focus thread was already started!");
		return -1;
	}

	pthread_mutex_init(&exynos_camera->auto_focus_mutex, NULL);

	rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_SET_AUTO_FOCUS, AUTO_FOCUS_ON);
	if (rc < 0) {
		ALOGE("%s: Unable to set auto-focus on", __func__);
		goto error;
	}

	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

	exynos_camera->auto_focus_thread_enabled = 1;

	rc = pthread_create(&exynos_camera->auto_focus_thread, &thread_attr, exynos_camera_auto_focus_thread, (void *) exynos_camera);
	if (rc < 0) {
		ALOGE("%s: Unable to create thread", __func__);
		goto error;
	}

	rc = 0;
	goto complete;

error:
	pthread_mutex_destroy(&exynos_camera->auto_focus_mutex);

	rc = -1;

complete:
	return rc;
}

void exynos_camera_auto_focus_thread_stop(struct exynos_camera *exynos_camera)
{
	int rc;
	int i;

	if (exynos_camera == NULL)
		return;

	ALOGD("%s()", __func__);

	if (!exynos_camera->auto_focus_thread_enabled) {
		ALOGE("Auto-focus thread was already stopped!");
		return;
	}

	exynos_camera->auto_focus_thread_enabled = 0;

	// Wait for the thread to end
	i = 0;
	while (exynos_camera->auto_focus_thread_running) {
		if (i++ > 10000) {
			ALOGE("Auto-focus thread is taking too long to end, something is going wrong");
			break;
		}
		usleep(100);
	}

	rc = exynos_v4l2_s_ctrl(exynos_camera, 0, V4L2_CID_CAMERA_SET_AUTO_FOCUS, AUTO_FOCUS_OFF);
	if (rc < 0)
		ALOGE("%s: Unable to set auto-focus off", __func__);

	pthread_mutex_destroy(&exynos_camera->auto_focus_mutex);
}

/*
 * Exynos Camera OPS
 */

int exynos_camera_set_preview_window(struct camera_device *dev,
	struct preview_stream_ops *w)
{
	struct exynos_camera *exynos_camera;

	int width, height, format, gralloc_format;

	buffer_handle_t *buffer;
	int stride;
	void *addr = NULL;

	int rc;

	ALOGD("%s(%p, %p)", __func__, dev, w);

	if (dev == NULL || dev->priv == NULL)
		return -EINVAL;

	exynos_camera = (struct exynos_camera *) dev->priv;

	if (exynos_camera->preview_thread_enabled)
		pthread_mutex_lock(&exynos_camera->preview_mutex);

	if (w == NULL) {
		exynos_camera->preview_window = NULL;
		return 0;
	}

	if (w->set_buffer_count == NULL || w->set_usage == NULL || w->set_buffers_geometry == NULL)
		goto error;

	rc = w->set_buffer_count(w, EXYNOS_CAMERA_GRALLOC_BUFFERS_COUNT);
	if (rc) {
		ALOGE("%s: Unable to set buffer count: %d", __func__, EXYNOS_CAMERA_GRALLOC_BUFFERS_COUNT);
		goto error;
	}

	rc = w->set_usage(w, GRALLOC_USAGE_SW_WRITE_OFTEN);
	if (rc) {
		ALOGE("%s: Unable to set usage", __func__);
		goto error;
	}

	width = exynos_camera->preview_width;
	height = exynos_camera->preview_height;
	format = exynos_camera->preview_format;

	switch (format) {
		case V4L2_PIX_FMT_NV21:
			gralloc_format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
			break;
		case V4L2_PIX_FMT_YUV420:
			gralloc_format = HAL_PIXEL_FORMAT_YV12;
			break;
		case V4L2_PIX_FMT_RGB565:
			gralloc_format = HAL_PIXEL_FORMAT_RGB_565;
			break;
		case V4L2_PIX_FMT_RGB32:
			gralloc_format = HAL_PIXEL_FORMAT_RGBX_8888;
			break;
		default:
			gralloc_format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
			break;
	}

	rc = w->set_buffers_geometry(w, width, height, gralloc_format);
	if (rc) {
		ALOGE("%s: Unable to set buffers geometry", __func__);
		goto error;
	}

	exynos_camera->preview_window = w;

	rc = 0;
	goto complete;

error:
	exynos_camera->preview_window = NULL;
	rc = -1;

complete:
	if (exynos_camera->preview_thread_enabled)
		pthread_mutex_unlock(&exynos_camera->preview_mutex);

	return rc;
}

void exynos_camera_set_callbacks(struct camera_device *dev,
	camera_notify_callback notify_cb,
	camera_data_callback data_cb,
	camera_data_timestamp_callback data_cb_timestamp,
	camera_request_memory get_memory,
	void *user)
{
	struct exynos_camera *exynos_camera;

	ALOGD("%s(%p, %p)", __func__, dev, user);

	if (dev == NULL || dev->priv == NULL)
		return;

	exynos_camera = (struct exynos_camera *) dev->priv;

	exynos_camera->callbacks.notify = notify_cb;
	exynos_camera->callbacks.data = data_cb;
	exynos_camera->callbacks.data_timestamp = data_cb_timestamp;
	exynos_camera->callbacks.request_memory = get_memory;
	exynos_camera->callbacks.user = user;
}

void exynos_camera_enable_msg_type(struct camera_device *dev, int32_t msg_type)
{
	struct exynos_camera *exynos_camera;

	ALOGD("%s(%p, %d)", __func__, dev, msg_type);

	if (dev == NULL || dev->priv == NULL)
		return;

	exynos_camera = (struct exynos_camera *) dev->priv;

	exynos_camera->messages_enabled |= msg_type;
}

void exynos_camera_disable_msg_type(struct camera_device *dev, int32_t msg_type)
{
	struct exynos_camera *exynos_camera;

	ALOGD("%s(%p, %d)", __func__, dev, msg_type);

	if (dev == NULL || dev->priv == NULL)
		return;

	exynos_camera = (struct exynos_camera *) dev->priv;

	exynos_camera->messages_enabled &= ~msg_type;
}

int exynos_camera_msg_type_enabled(struct camera_device *dev, int32_t msg_type)
{
	struct exynos_camera *exynos_camera;

	ALOGD("%s(%p, %d)", __func__, dev, msg_type);

	if (dev == NULL || dev->priv == NULL)
		return -EINVAL;

	exynos_camera = (struct exynos_camera *) dev->priv;

	return exynos_camera->messages_enabled & msg_type;
}

int exynos_camera_start_preview(struct camera_device *dev)
{
	struct exynos_camera *exynos_camera;
	int rc;

	ALOGD("%s(%p)", __func__, dev);

	if (dev == NULL || dev->priv == NULL)
		return -EINVAL;

	exynos_camera = (struct exynos_camera *) dev->priv;

	exynos_camera->callback_lock = 1;
	rc = exynos_camera_preview_thread_start(exynos_camera);
	exynos_camera->callback_lock = 0;

	return rc;
}

void exynos_camera_stop_preview(struct camera_device *dev)
{
	struct exynos_camera *exynos_camera;

	ALOGD("%s(%p)", __func__, dev);

	if (dev == NULL || dev->priv == NULL)
		return;

	exynos_camera = (struct exynos_camera *) dev->priv;

	exynos_camera->callback_lock = 1;
	exynos_camera_preview_thread_stop(exynos_camera);
	exynos_camera->callback_lock = 0;
}

int exynos_camera_preview_enabled(struct camera_device *dev)
{
	struct exynos_camera *exynos_camera;

	ALOGD("%s(%p)", __func__, dev);

	if (dev == NULL || dev->priv == NULL)
		return -EINVAL;

	exynos_camera = (struct exynos_camera *) dev->priv;

	return exynos_camera->preview_thread_enabled;
}

int exynos_camera_store_meta_data_in_buffers(struct camera_device *dev,
	int enable)
{
	struct exynos_camera *exynos_camera;

	ALOGD("%s(%p, %d)", __func__, dev, enable);

	if (dev == NULL || dev->priv == NULL)
		return -EINVAL;

	exynos_camera = (struct exynos_camera *) dev->priv;

	if (!exynos_camera->recording_thread_enabled)
		exynos_camera->recording_metadata = enable;
	else
		ALOGE("%s: Recording is running!", __func__);

	return 0;
}

int exynos_camera_start_recording(struct camera_device *dev)
{
	struct exynos_camera *exynos_camera;
	int rc;

	ALOGD("%s(%p)", __func__, dev);

	exynos_camera = (struct exynos_camera *) dev->priv;

	exynos_camera->callback_lock = 1;
	rc = exynos_camera_recording_thread_start(exynos_camera);
	exynos_camera->callback_lock = 0;

	return rc;
}

void exynos_camera_stop_recording(struct camera_device *dev)
{
	struct exynos_camera *exynos_camera;

	ALOGD("%s(%p)", __func__, dev);

	exynos_camera = (struct exynos_camera *) dev->priv;

	exynos_camera->callback_lock = 1;
	exynos_camera_recording_thread_stop(exynos_camera);
	exynos_camera->callback_lock = 0;
}

int exynos_camera_recording_enabled(struct camera_device *dev)
{
	struct exynos_camera *exynos_camera;

	ALOGD("%s(%p)", __func__, dev);

	if (dev == NULL || dev->priv == NULL)
		return -EINVAL;

	exynos_camera = (struct exynos_camera *) dev->priv;

	return exynos_camera->recording_thread_enabled;
}

void exynos_camera_release_recording_frame(struct camera_device *dev,
	const void *opaque)
{
	struct exynos_camera *exynos_camera;

//	ALOGD("%s(%p, %p)", __func__, dev, opaque);

	if (dev == NULL || dev->priv == NULL)
		return;

	exynos_camera = (struct exynos_camera *) dev->priv;

	exynos_camera_recording_frame_release(exynos_camera);
}

int exynos_camera_start_auto_focus(struct camera_device *dev)
{
	struct exynos_camera *exynos_camera;

	ALOGD("%s(%p)", __func__, dev);

	if (dev == NULL || dev->priv == NULL)
		return -EINVAL;

	exynos_camera = (struct exynos_camera *) dev->priv;

	return exynos_camera_auto_focus_thread_start(exynos_camera);
}

int exynos_camera_cancel_auto_focus(struct camera_device *dev)
{
	struct exynos_camera *exynos_camera;

	ALOGD("%s(%p)", __func__, dev);

	if (dev == NULL || dev->priv == NULL)
		return -EINVAL;

	exynos_camera = (struct exynos_camera *) dev->priv;

	exynos_camera_auto_focus_thread_stop(exynos_camera);

	return 0;
}

int exynos_camera_take_picture(struct camera_device *dev)
{
	struct exynos_camera *exynos_camera;
	int rc;

	ALOGD("%s(%p)", __func__, dev);

	if (dev == NULL || dev->priv == NULL)
		return -EINVAL;

	exynos_camera = (struct exynos_camera *) dev->priv;

	if (exynos_camera->picture_thread_running
		|| exynos_camera->auto_focus_thread_enabled)
	{
		return 0;
	}

	exynos_camera->callback_lock = 1;
	rc = exynos_camera_picture_thread_start(exynos_camera);
	exynos_camera->callback_lock = 0;

	return rc;
}

int exynos_camera_cancel_picture(struct camera_device *dev)
{
	struct exynos_camera *exynos_camera;
	int rc;

	ALOGD("%s(%p)", __func__, dev);

	if (dev == NULL || dev->priv == NULL)
		return -EINVAL;

	exynos_camera = (struct exynos_camera *) dev->priv;

	exynos_camera->callback_lock = 1;
	exynos_camera_picture_thread_stop(exynos_camera);
	exynos_camera->callback_lock = 0;

	return 0;
}

int exynos_camera_set_parameters(struct camera_device *dev,
	const char *params)
{
	struct exynos_camera *exynos_camera;
	int rc;

	ALOGD("%s(%p, %s)", __func__, dev, params);

	if (dev == NULL || dev->priv == NULL || params == NULL)
		return -EINVAL;

	exynos_camera = (struct exynos_camera *) dev->priv;

	rc = exynos_params_string_set(exynos_camera, (char *) params);
	if (rc < 0) {
		ALOGE("%s: Unable to set params string", __func__);
		return -1;
	}

	rc = exynos_camera_params_apply(exynos_camera, 0);
	if (rc < 0) {
		ALOGE("%s: Unable to apply params", __func__);
		return -1;
	}

	return 0;
}

char *exynos_camera_get_parameters(struct camera_device *dev)
{
	struct exynos_camera *exynos_camera;
	char *params;

	ALOGD("%s(%p)", __func__, dev);

	if (dev == NULL || dev->priv == NULL)
		return NULL;

	exynos_camera = (struct exynos_camera *) dev->priv;

	params = exynos_params_string_get(exynos_camera);
	if (params == NULL) {
		ALOGE("%s: Couldn't find any param", __func__);
		return strdup("");
	}

	return params;
}

void exynos_camera_put_parameters(struct camera_device *dev, char *params)
{
	ALOGD("%s(%p)", __func__, dev);

	if (params != NULL)
		free(params);
}

int exynos_camera_send_command(struct camera_device *dev,
	int32_t cmd, int32_t arg1, int32_t arg2)
{
	ALOGD("%s(%p, %d, %d, %d)", __func__, dev, cmd, arg1, arg2);

	return 0;
}

void exynos_camera_release(struct camera_device *dev)
{
	struct exynos_camera *exynos_camera;

	ALOGD("%s(%p)", __func__, dev);

	if (dev == NULL || dev->priv == NULL)
		return;

	exynos_camera = (struct exynos_camera *) dev->priv;

	exynos_camera_capture_thread_stop(exynos_camera);

	exynos_camera_stop(exynos_camera);
}

int exynos_camera_dump(struct camera_device *dev, int fd)
{
	ALOGD("%s(%p, %d)", __func__, dev, fd);

	return 0;
}

/*
 * Interface
 */

struct camera_device_ops exynos_camera_ops = {
	.set_preview_window = exynos_camera_set_preview_window,
	.set_callbacks = exynos_camera_set_callbacks,
	.enable_msg_type = exynos_camera_enable_msg_type,
	.disable_msg_type = exynos_camera_disable_msg_type,
	.msg_type_enabled = exynos_camera_msg_type_enabled,
	.start_preview = exynos_camera_start_preview,
	.stop_preview = exynos_camera_stop_preview,
	.preview_enabled = exynos_camera_preview_enabled,
	.store_meta_data_in_buffers = exynos_camera_store_meta_data_in_buffers,
	.start_recording = exynos_camera_start_recording,
	.stop_recording = exynos_camera_stop_recording,
	.recording_enabled = exynos_camera_recording_enabled,
	.release_recording_frame = exynos_camera_release_recording_frame,
	.auto_focus = exynos_camera_start_auto_focus,
	.cancel_auto_focus = exynos_camera_cancel_auto_focus,
	.take_picture = exynos_camera_take_picture,
	.cancel_picture = exynos_camera_cancel_picture,
	.set_parameters = exynos_camera_set_parameters,
	.get_parameters = exynos_camera_get_parameters,
	.put_parameters = exynos_camera_put_parameters,
	.send_command = exynos_camera_send_command,
	.release = exynos_camera_release,
	.dump = exynos_camera_dump,
};

int exynos_camera_close(hw_device_t *device)
{
	struct camera_device *camera_device;
	struct exynos_camera *exynos_camera;

	ALOGD("%s(%p)", __func__, device);

	if (device == NULL)
		return -EINVAL;

	camera_device = (struct camera_device *) device;

	if (camera_device->priv != NULL) {
		free(camera_device->priv);
	}

	free(camera_device);

	return 0;
}

int exynos_camera_open(const struct hw_module_t* module, const char *camera_id,
	struct hw_device_t** device)
{
	struct camera_device *camera_device = NULL;
	struct exynos_camera *exynos_camera = NULL;
	int id;
	int rc;

	ALOGD("%s(%p, %s, %p)", __func__, module, camera_id, device);

	if (module == NULL || camera_id == NULL || device == NULL)
		return -EINVAL;

	id = atoi(camera_id);
	if (id < 0)
		return -EINVAL;
	
	exynos_camera = calloc(1, sizeof(struct exynos_camera));
	exynos_camera->config = exynos_camera_config;

	if (exynos_camera->config->v4l2_nodes_count > EXYNOS_CAMERA_MAX_V4L2_NODES_COUNT)
		goto error_preset;

	if (id >= exynos_camera->config->presets_count)
		goto error_preset;

	rc = exynos_camera_start(exynos_camera, id);
	if (rc < 0) {
		ALOGE("%s: Unable to start camera", __func__);
		goto error;
	}

	rc = exynos_camera_capture_thread_start(exynos_camera);
	if (rc < 0) {
		ALOGE("%s: Unable to start capture thread", __func__);
		goto error;
	}

	camera_device = calloc(1, sizeof(struct camera_device));
	camera_device->common.tag = HARDWARE_DEVICE_TAG;
	camera_device->common.version = 0;
	camera_device->common.module = (struct hw_module_t *) module;
	camera_device->common.close = exynos_camera_close;

	camera_device->ops = &exynos_camera_ops;
	camera_device->priv = exynos_camera;

	*device = (struct hw_device_t *) &(camera_device->common);

	return 0;

error:
	exynos_camera_stop(exynos_camera);

error_device:
	if (camera_device != NULL)
		free(camera_device);

error_preset:
	if (exynos_camera != NULL)
		free(exynos_camera);

	return -1;
}

int exynos_camera_get_number_of_cameras(void)
{
	ALOGD("%s()", __func__);

	if (exynos_camera_config == NULL || exynos_camera_config->presets == NULL) {
		ALOGE("%s: Unable to find proper camera config", __func__);
		return -1;
	}

	return exynos_camera_config->presets_count;
}

int exynos_camera_get_camera_info(int id, struct camera_info *info)
{
	ALOGD("%s(%d, %p)", __func__, id, info);

	if (id < 0 || info == NULL)
		return -EINVAL;

	if (exynos_camera_config == NULL || exynos_camera_config->presets == NULL) {
		ALOGE("%s: Unable to find proper camera config", __func__);
		return -1;
	}

	if (id >= exynos_camera_config->presets_count)
		return -EINVAL;

	ALOGD("Selected camera: %s", exynos_camera_config->presets[id].name);

	info->facing = exynos_camera_config->presets[id].facing;
	info->orientation = exynos_camera_config->presets[id].orientation;

	return 0;
}

struct hw_module_methods_t exynos_camera_module_methods = {
	.open = exynos_camera_open,
};

struct camera_module HAL_MODULE_INFO_SYM = {
	.common = {
		.tag = HARDWARE_MODULE_TAG,
		.hal_api_version = HARDWARE_HAL_API_VERSION,
		.module_api_version = CAMERA_MODULE_API_VERSION_1_0,
		.id = CAMERA_HARDWARE_MODULE_ID,
		.name = "Exynos Camera",
		.author = "Paul Kocialkowski",
		.methods = &exynos_camera_module_methods,
	},
	.get_number_of_cameras = exynos_camera_get_number_of_cameras,
	.get_camera_info = exynos_camera_get_camera_info,
};
