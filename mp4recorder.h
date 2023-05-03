/*
 * minimp4_test.h
 *
 *  Created on: Oct 7, 2022
 *      Author: phongdv
 */

#ifndef MP4RECORDER_H_
#define MP4RECORDER_H_

int pps_dua_convert_to_mp4(int codec_type, char *video_path, char *audio_path, char *mp4_file_path, int video_max_frame, int audio_max_frame, int resolution_type);

#endif /* MP4RECORDER_H_ */
