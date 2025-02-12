/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
Part of 6.3.0

FILE NAME
    operators_constants.h

DESCRIPTION
    Constants used by operators.
*/

#ifndef OPERATORS_CONSTANTS_H_
#define OPERATORS_CONSTANTS_H_

#define OPERATOR_CREATE_KEY_SET_PRIORITY   1
#define OPERATOR_CREATE_KEY_SET_PROCESSOR_ID 2

#define RESAMPLER_SET_CONVERSION_RATE   2

#define IIR_RESAMPLER_SET_SAMPLE_RATES 1

#define RTP_SET_MODE                 1
#define RTP_SET_CODEC_TYPE           2
#define RTP_SET_CONTENT_PROTECTION   3
#define RTP_SET_AAC_CODEC            4
#define RTP_SET_MAX_PACKET_LENGTH    5
#define RTP_SET_SSRC_LATENCY_MAPPING 7

#define MIXER_SET_GAINS            1
#define MIXER_SET_STREAM_CHANNELS  2
#define MIXER_SET_RAMP_NUM_SAMPLES 3
#define MIXER_SET_PRIMARY_STREAM   4
#define MIXER_SET_CHANNEL_GAINS    5

#define SOURCE_SYNC_SET_ROUTES       1
#define SOURCE_SYNC_SET_SINK_GROUP   3
#define SOURCE_SYNC_SET_SOURCE_GROUP 4

/* Common Message Ids */
#define ENABLE_FADE_OUT          0x2000
#define DISABLE_FADE_OUT         0x2001
#define SET_CONTROL              0x2002
#define GET_PARAMS               0x2003
#define SET_PARAMS               0x2005
#define SET_UCID                 0x2007
#define SET_BUFFER_SIZE          0x200c
#define SET_SAMPLE_RATE          0x200e

#define SET_TIME_TO_PLAY_LATENCY 0x2012

#define SET_LATENCY_LIMITS       0x2015

#define USB_AUDIO_SET_CONNECTION_CONFIG 0x0002

#define SBC_ENCODER_SET_ENCODING_PARAMS 0x0001

#define CELT_ENCODER_SET_ENCODING_PARAMS 0x0001

#define AEC_REF_SET_SAMPLE_RATES 0x00FE

#define AEC_REF_MUTE_MIC_OUTPUT 0x0009

#define AEC_REF_ENABLE_SPKR_GATE 0x000A

#define SPDIF_SET_OUTPUT_SAMPLE_RATE    0x0003

#define SPC_SET_TRANSITION 0x0001
#define SPC_SET_FORMAT 0x0002

#define MAX_NOTES_NUM 63

#define USB_AUDIO_DATA_FORMAT_PCM   0

#define SPLITTER_SET_RUNNING_STREAMS 1
#define SPLITTER_SET_DATA_FORMAT 10

#define SPLITTER_FIRST_OUTPUT_ACTIVE 1
#define SPLITTER_BOTH_OUTPUTS_ACTIVE 3

#define PASSTHRUOGH_SET_INPUT_DATA_FORMAT 10
#define PASSTHRUOGH_SET_OUTPUT_DATA_FORMAT 11

/* Constants for Volume Operator Control Ids */
#define POST_GAIN    0x0020
#define MAIN_GAIN    0x0021
#define AUX_GAIN1    0x0030
#define AUX_GAIN2    0x0031
#define AUX_GAIN3    0x0032
#define AUX_GAIN4    0x0033
#define AUX_GAIN5    0x0034
#define AUX_GAIN6    0x0035
#define AUX_GAIN7    0x0036
#define AUX_GAIN8    0x0037
#define TRIM_GAIN1   0x0010
#define TRIM_GAIN2   0x0011
#define TRIM_GAIN3   0x0012
#define TRIM_GAIN4   0x0013
#define TRIM_GAIN5   0x0014
#define TRIM_GAIN6   0x0015
#define TRIM_GAIN7   0x0016
#define TRIM_GAIN8   0x0017

/* Constants for Operator Set Control Message */
/* originally defined in the dsp project workspace in opsmsg_prim.h*/
#define OPMSG_CONTROL_MODE_ID 0x0001
#define OPMSG_CONTROL_MUTE_ID 0x0002

/* Constants for Operator Set Control Mode values */
#define CONTROL_MODE_STANDBY         1
#define CONTROL_MODE_FULL_PROCESSING 2
#define CONTROL_MODE_PASSTHROUGH     3

/* Constants for Operator Set Control Mode values */
#define MUSIC_PROCESSING_MODE_STANDBY         1
#define MUSIC_PROCESSING_MODE_FULL_PROCESSING 2
#define MUSIC_PROCESSING_MODE_PASSTHROUGH     3

/* Constants for Set Data Format messages values */
#define SET_DATA_FORMAT_ENCODED 0
#define SET_DATA_FORMAT_PCM     1

#define SOURCE_SYNC_GROUP_META_DATA_FLAG      ((uint32)(1 << 31))
#define SOURCE_SYNC_GROUP_TTP_FLAG            ((uint32)(1 << 30))
#define SOURCE_SYNC_GROUP_RATE_MATCH_FLAG     ((uint32)(1 << 27))

/* Constants for celt encoder */
#define CELT_CODEC_FRAME_SIZE_DEFAULT 220
/*! CELT encoder mode: 512 samples per frame, 48000Hz */
#define CELT_ENC_MODE_512_48000 0
/*! CELT encoder mode: 512 samples per frame, 44100Hz */
#define CELT_ENC_MODE_512_44100 1
#define CELT_ENC_MODE_MONO 0
#define CELT_ENC_MODE_STEREO 1

/* Constants for SVA operator */
#define SVA_TRIGGER_PHRASE_LOAD 0x0002

/* Framework configuration parameters */
#define FRAMEWORK_KICK_PERIOD_PARAM 7

typedef enum {
    aptx_ad_ll_0_ssrc_id = 0xA1,
    aptx_ad_ll_1_ssrc_id = 0xA2,
    aptx_ad_hq_ssrc_id = 0xA3,
    aptx_ad_tws_ssrc_id = 0xA4
} aptx_adaptive_ssrc_ids_t;

#endif /* OPERATORS_CONSTANTS_H_ */
