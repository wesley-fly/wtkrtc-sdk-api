/*
 * Asterisk -- An open source telephony toolkit.
 *
 * Copyright (C) 1999 - 2005, Digium, Inc.
 *
 * Mark Spencer <markster@digium.com>
 *
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2. See the LICENSE file
 * at the top of the source tree.
 */

/*! \file
 *
 * \brief Translate between signed linear and Opus (Open Codec)
 *
 * \author Lorenzo Miniero <lorenzo@meetecho.com>
 *
 * \note This work was motivated by Mozilla
 * 
 * \ingroup codecs
 *
 * \extref The Opus library - http://opus-codec.org
 *
 */

/*** MODULEINFO
	<depend>opus</depend>
	<support_level>core</support_level>
 ***/

#include "asterisk.h"

ASTERISK_FILE_VERSION(__FILE__, "$Revision: 328209 $")

#include <opus/opus.h>

#include "asterisk/translate.h"
#include "asterisk/module.h"
#include "asterisk/config.h"
#include "asterisk/utils.h"
#include "asterisk/cli.h"


#define	BUFFER_SAMPLES	8000
#define	OPUS_SAMPLES	160

#define USE_FEC			0

#define DEFAULT_COMPLEXITY	4


/* Sample frame data */
#include "asterisk/slin.h"
#include "ex_opus.h"

/* FIXME: Test */
#include "asterisk/file.h"


static int encid = 0;
static int decid = 0;

static int opusdebug = 0;


/* Private structures */
struct opus_coder_pvt {
	void *opus;	/* May be encoder or decoder */
	int sampling_rate;
	int multiplier;
	int fec;

	int id;

	int16_t buf[BUFFER_SAMPLES];	/* FIXME */
	int framesize;
	
	FILE *file;
};


/* Helper methods */
static int opus_encoder_construct(struct ast_trans_pvt *pvt, int sampling_rate) {
	if(sampling_rate != 8000 && sampling_rate != 12000 && sampling_rate != 16000 && sampling_rate != 24000 && sampling_rate != 48000)
		return -1;
	struct opus_coder_pvt *opvt = pvt->pvt;
	opvt->sampling_rate = sampling_rate;
	opvt->multiplier = 48000/sampling_rate;
	opvt->fec = USE_FEC;
	int error = 0;
	opvt->opus = opus_encoder_create(sampling_rate, 1, OPUS_APPLICATION_VOIP, &error);
	if(error != OPUS_OK) {
		if(opusdebug)
			ast_verbose("[Opus] Ops! got an error creating the Opus encoder: %d (%s)\n", error, opus_strerror(error));
		return -1;
	}
	if(sampling_rate == 8000)
		opus_encoder_ctl(opvt->opus, OPUS_SET_MAX_BANDWIDTH(OPUS_BANDWIDTH_NARROWBAND));
	else if(sampling_rate == 12000)
		opus_encoder_ctl(opvt->opus, OPUS_SET_MAX_BANDWIDTH(OPUS_BANDWIDTH_MEDIUMBAND));
	else if(sampling_rate == 16000)
		opus_encoder_ctl(opvt->opus, OPUS_SET_MAX_BANDWIDTH(OPUS_BANDWIDTH_WIDEBAND));
	else if(sampling_rate == 24000)
		opus_encoder_ctl(opvt->opus, OPUS_SET_MAX_BANDWIDTH(OPUS_BANDWIDTH_SUPERWIDEBAND));
	else if(sampling_rate == 48000)
		opus_encoder_ctl(opvt->opus, OPUS_SET_MAX_BANDWIDTH(OPUS_BANDWIDTH_FULLBAND));
	opus_encoder_ctl(opvt->opus, OPUS_SET_INBAND_FEC(opvt->fec));
	opus_encoder_ctl(opvt->opus, OPUS_SET_COMPLEXITY(DEFAULT_COMPLEXITY));
	opvt->framesize = sampling_rate/50;
	opvt->id = ++encid;
	if(opusdebug)
		ast_verbose("[Opus] Created encoder #%d (%d->opus)\n", opvt->id, sampling_rate);

	return 0;
}

static int opus_decoder_construct(struct ast_trans_pvt *pvt, int sampling_rate) {
	if(sampling_rate != 8000 && sampling_rate != 12000 && sampling_rate != 16000 && sampling_rate != 24000 && sampling_rate != 48000)
		return -1;
	struct opus_coder_pvt *opvt = pvt->pvt;
	opvt->sampling_rate = sampling_rate;
	opvt->multiplier = 48000/sampling_rate;
	opvt->fec = USE_FEC;	/* FIXME: should be triggered by chan_sip */
	int error = 0;
	opvt->opus = opus_decoder_create(sampling_rate, 1, &error);
	if(error != OPUS_OK) {
		if(opusdebug)
			ast_verbose("[Opus] Ops! got an error creating the Opus decoder: %d (%s)\n", error, opus_strerror(error));
		return -1;
	}
	opvt->id = ++decid;
	if(opusdebug)
		ast_verbose("[Opus] Created decoder #%d (opus->%d)\n", opvt->id, sampling_rate);

	if(opusdebug > 1) {
		char filename[50];
		sprintf(filename, "/home/lminiero/opusdec-%04d-%d.raw", opvt->id, opvt->sampling_rate);
		opvt->file = fopen(filename, "wb");
	}
	
	return 0;
}

/* Translator callbacks */
static int lintoopus_new(struct ast_trans_pvt *pvt) {
	return opus_encoder_construct(pvt, 8000);
}

static int lin12toopus_new(struct ast_trans_pvt *pvt) {
	return opus_encoder_construct(pvt, 12000);
}

static int lin16toopus_new(struct ast_trans_pvt *pvt) {
	return opus_encoder_construct(pvt, 16000);
}

static int lin24toopus_new(struct ast_trans_pvt *pvt) {
	return opus_encoder_construct(pvt, 24000);
}

static int lin48toopus_new(struct ast_trans_pvt *pvt) {
	return opus_encoder_construct(pvt, 48000);
}

static int opustolin_new(struct ast_trans_pvt *pvt) {
	return opus_decoder_construct(pvt, 8000);
}

static int opustolin12_new(struct ast_trans_pvt *pvt) {
	return opus_decoder_construct(pvt, 12000);
}

static int opustolin16_new(struct ast_trans_pvt *pvt) {
	return opus_decoder_construct(pvt, 16000);
}

static int opustolin24_new(struct ast_trans_pvt *pvt) {
	return opus_decoder_construct(pvt, 24000);
}

static int opustolin48_new(struct ast_trans_pvt *pvt) {
	return opus_decoder_construct(pvt, 48000);
}

static int lintoopus_framein(struct ast_trans_pvt *pvt, struct ast_frame *f) {
	struct opus_coder_pvt *opvt = pvt->pvt;

	/* XXX We should look at how old the rest of our stream is, and if it
	   is too old, then we should overwrite it entirely, otherwise we can
	   get artifacts of earlier talk that do not belong */
	memcpy(opvt->buf + pvt->samples, f->data.ptr, f->datalen);
	pvt->samples += f->samples;

	return 0;
}

static struct ast_frame *lintoopus_frameout(struct ast_trans_pvt *pvt) {
	struct opus_coder_pvt *opvt = pvt->pvt;

	/* We can't work on anything less than a frame in size */
	if (pvt->samples < opvt->framesize)
		return NULL;
		
	int datalen = 0;	/* output bytes */
	int samples = 0;	/* output samples */

	while(pvt->samples >= opvt->framesize) {
		/* Encode 160 samples (or more if it's not narrowband) */
		if(opusdebug > 1)
			ast_verbose("[Opus] [Encoder #%d (%d)] %d samples, %d bytes\n", opvt->id, opvt->sampling_rate, opvt->framesize, opvt->framesize*2);
		int len = opus_encode(opvt->opus, opvt->buf, opvt->framesize, pvt->outbuf.uc, BUFFER_SAMPLES);
		if(len < 0) {
			if(opusdebug)
				ast_verbose("[Opus] Ops! got an error encoding the Opus frame: %d (%s)\n", len, opus_strerror(len));
			return NULL;
		}
		datalen += len;
		samples += opvt->framesize;
		pvt->samples -= opvt->framesize;
		/* Move the data at the end of the buffer to the front */
		if (pvt->samples)
			memmove(opvt->buf, opvt->buf + samples, pvt->samples * 2);

		if(opusdebug > 1)
			ast_verbose("[Opus] [Encoder #%d (%d)]   >> Got %d samples, %d bytes\n", opvt->id, opvt->sampling_rate, opvt->multiplier*samples, datalen);

		if(opvt->file)
			fwrite(opvt->buf, sizeof(int16_t), opvt->multiplier*samples, opvt->file);
	}
	
	/* Move the data at the end of the buffer to the front */
	if(pvt->samples)
		memmove(opvt->buf, opvt->buf+samples, pvt->samples*2);

	return ast_trans_frameout(pvt, datalen, opvt->multiplier*samples);	
}

static int opustolin_framein(struct ast_trans_pvt *pvt, struct ast_frame *f) {
	struct opus_coder_pvt *opvt = pvt->pvt;
	/* Decode */
	if(opusdebug > 1)
		ast_verbose("[Opus] [Decoder #%d (%d)] %d samples, %d bytes\n", opvt->id, opvt->sampling_rate, f->samples, f->datalen);
	int error = opus_decode(opvt->opus, f->data.ptr, f->datalen, pvt->outbuf.i16, BUFFER_SAMPLES, opvt->fec);
	if(error < 0) {
		if(opusdebug)
			ast_verbose("[Opus] Ops! got an error decoding the Opus frame: %d (%s)\n", error, opus_strerror(error));
		return -1;
	}
	pvt->samples += error;
	pvt->datalen += error*2;

	if(opusdebug > 1)
		ast_verbose("[Opus] [Decoder #%d (%d)]   >> Got %d samples, %d bytes\n", opvt->id, opvt->sampling_rate, pvt->samples, pvt->datalen);

	if(opvt->file)
		fwrite(pvt->outbuf.i16, sizeof(int16_t), pvt->samples, opvt->file);

	return 0;
}

static void lintoopus_destroy(struct ast_trans_pvt *arg) {
	struct opus_coder_pvt *opvt = arg->pvt;
	if(opvt == NULL || opvt->opus == NULL)
		return;
	opus_encoder_destroy(opvt->opus);
	if(opusdebug)
		ast_verbose("[Opus] Destroyed encoder #%d (%d->opus)\n", opvt->id, opvt->sampling_rate);
	opvt->opus = NULL;

	if(opvt->file)
		fclose(opvt->file);
	opvt->file = NULL;
}

static void opustolin_destroy(struct ast_trans_pvt *arg) {
	struct opus_coder_pvt *opvt = arg->pvt;
	if(opvt == NULL || opvt->opus == NULL)
		return;
	opus_decoder_destroy(opvt->opus);
	if(opusdebug)
		ast_verbose("[Opus] Destroyed decoder #%d (opus->%d)\n", opvt->id, opvt->sampling_rate);
	opvt->opus = NULL;

	if(opvt->file)
		fclose(opvt->file);
	opvt->file = NULL;
}

	
/* Translators */
static struct ast_translator lintoopus = {
	.name = "lintoopus", 
	.newpvt = lintoopus_new,
	.framein = lintoopus_framein,
	.frameout = lintoopus_frameout,
	.destroy = lintoopus_destroy,
	.sample = slin8_sample,
	.desc_size = sizeof(struct opus_coder_pvt),
	.buffer_samples = BUFFER_SAMPLES,
	.buf_size = BUFFER_SAMPLES * 2,
};

static struct ast_translator lin12toopus = {
	.name = "lin12toopus", 
	.newpvt = lin12toopus_new,
	.framein = lintoopus_framein,
	.frameout = lintoopus_frameout,
	.destroy = lintoopus_destroy,
	.desc_size = sizeof(struct opus_coder_pvt),
	.buffer_samples = BUFFER_SAMPLES,
	.buf_size = BUFFER_SAMPLES * 2,
};

static struct ast_translator lin16toopus = {
	.name = "lin16toopus", 
	.newpvt = lin16toopus_new,
	.framein = lintoopus_framein,
	.frameout = lintoopus_frameout,
	.destroy = lintoopus_destroy,
	.sample = slin16_sample,
	.desc_size = sizeof(struct opus_coder_pvt),
	.buffer_samples = BUFFER_SAMPLES,
	.buf_size = BUFFER_SAMPLES * 2,
};

static struct ast_translator lin24toopus = {
	.name = "lin24toopus", 
	.newpvt = lin24toopus_new,
	.framein = lintoopus_framein,
	.frameout = lintoopus_frameout,
	.destroy = lintoopus_destroy,
	.desc_size = sizeof(struct opus_coder_pvt),
	.buffer_samples = BUFFER_SAMPLES,
	.buf_size = BUFFER_SAMPLES * 2,
};

static struct ast_translator lin48toopus = {
	.name = "lin48toopus", 
	.newpvt = lin48toopus_new,
	.framein = lintoopus_framein,
	.frameout = lintoopus_frameout,
	.destroy = lintoopus_destroy,
	.desc_size = sizeof(struct opus_coder_pvt),
	.buffer_samples = BUFFER_SAMPLES,
	.buf_size = BUFFER_SAMPLES * 2,
};

static struct ast_translator opustolin = {
	.name = "opustolin", 
	.newpvt = opustolin_new,
	.framein = opustolin_framein,
	.destroy = opustolin_destroy,
	.sample = opus_sample,
	.desc_size = sizeof(struct opus_coder_pvt),
	.buffer_samples = BUFFER_SAMPLES,
	.buf_size = BUFFER_SAMPLES * 2,
	.native_plc = 1,	/* FIXME: needed? */
};

static struct ast_translator opustolin12 = {
	.name = "opustolin12", 
	.newpvt = opustolin12_new,
	.framein = opustolin_framein,
	.destroy = opustolin_destroy,
	.sample = opus_sample,
	.desc_size = sizeof(struct opus_coder_pvt),
	.buffer_samples = BUFFER_SAMPLES,
	.buf_size = BUFFER_SAMPLES * 2,
	.native_plc = 1,	/* FIXME: needed? */
};

static struct ast_translator opustolin16 = {
	.name = "opustolin16", 
	.newpvt = opustolin16_new,
	.framein = opustolin_framein,
	.destroy = opustolin_destroy,
	.sample = opus_sample,
	.desc_size = sizeof(struct opus_coder_pvt),
	.buffer_samples = BUFFER_SAMPLES,
	.buf_size = BUFFER_SAMPLES * 2,
	.native_plc = 1,	/* FIXME: needed? */
};

static struct ast_translator opustolin24 = {
	.name = "opustolin24", 
	.newpvt = opustolin24_new,
	.framein = opustolin_framein,
	.destroy = opustolin_destroy,
	.sample = opus_sample,
	.desc_size = sizeof(struct opus_coder_pvt),
	.buffer_samples = BUFFER_SAMPLES,
	.buf_size = BUFFER_SAMPLES * 2,
	.native_plc = 1,	/* FIXME: needed? */
};

static struct ast_translator opustolin48 = {
	.name = "opustolin48", 
	.newpvt = opustolin48_new,
	.framein = opustolin_framein,
	.destroy = opustolin_destroy,
	.sample = opus_sample,
	.desc_size = sizeof(struct opus_coder_pvt),
	.buffer_samples = BUFFER_SAMPLES,
	.buf_size = BUFFER_SAMPLES * 2,
	.native_plc = 1,	/* FIXME: needed? */
};


/* Simple CLI interface to enable/disable debugging */
static char *handle_cli_opus_set_debug(struct ast_cli_entry *e, int cmd, struct ast_cli_args *a)
{
	switch (cmd) {
	case CLI_INIT:
		e->command = "opus set debug";
		e->usage =
			"Usage: opus set debug {status|none|normal|huge}\n"
			"       Enable/Disable Opus debugging: normal only debugs setup and errors, huge debugs every single packet\n";
		return NULL;
	case CLI_GENERATE:
		return NULL;
	}

	if (a->argc != 4)
		return CLI_SHOWUSAGE;

	if (!strncasecmp(a->argv[a->argc-1], "status", 6)) {
		ast_cli(a->fd, "Opus debugging %s\n", opusdebug > 1 ? "huge" : opusdebug > 0 ? "normal" : "none");
		return CLI_SUCCESS;
	}
	if (!strncasecmp(a->argv[a->argc-1], "huge", 4))
		opusdebug = 2;
	else if (!strncasecmp(a->argv[a->argc-1], "normal", 6))
		opusdebug = 1;
	else if (!strncasecmp(a->argv[a->argc-1], "none", 4))
		opusdebug = 0;
	else
		return CLI_SHOWUSAGE;

	ast_cli(a->fd, "Opus debugging %s\n", opusdebug > 1 ? "huge" : opusdebug > 0 ? "normal" : "none");
	return CLI_SUCCESS;
}

static struct ast_cli_entry cli_opus[] = {
	AST_CLI_DEFINE(handle_cli_opus_set_debug, "Enable/Disable Opus debugging"),
};


/* Configuration and module setup */
static int parse_config(int reload) {
	// TODO
	return 0;
}

static int reload(void) {
	if(parse_config(1))
		return AST_MODULE_LOAD_DECLINE;
	return AST_MODULE_LOAD_SUCCESS;
}

static int unload_module(void) {
	int res = 0;

	res |= ast_unregister_translator(&opustolin);
	res |= ast_unregister_translator(&lintoopus);
	res |= ast_unregister_translator(&opustolin12);
	res |= ast_unregister_translator(&lin12toopus);
	res |= ast_unregister_translator(&opustolin16);
	res |= ast_unregister_translator(&lin16toopus);
	res |= ast_unregister_translator(&opustolin24);
	res |= ast_unregister_translator(&lin24toopus);
	res |= ast_unregister_translator(&opustolin48);
	res |= ast_unregister_translator(&lin48toopus);

	ast_cli_unregister_multiple(cli_opus, ARRAY_LEN(cli_opus));

	return res;
}

static int load_module(void) {
	int res = 0;

	if(parse_config(0))
		return AST_MODULE_LOAD_DECLINE;

	/* 8khz (nb) */
	ast_format_set(&opustolin.src_format, AST_FORMAT_OPUS, 0);
	ast_format_set(&opustolin.dst_format, AST_FORMAT_SLINEAR, 0);
	ast_format_set(&lintoopus.src_format, AST_FORMAT_SLINEAR, 0);
	ast_format_set(&lintoopus.dst_format, AST_FORMAT_OPUS, 0);
	/* 12khz (mb) */
	ast_format_set(&opustolin12.src_format, AST_FORMAT_OPUS, 0);
	ast_format_set(&opustolin12.dst_format, AST_FORMAT_SLINEAR12, 0);
	ast_format_set(&lin12toopus.src_format, AST_FORMAT_SLINEAR12, 0);
	ast_format_set(&lin12toopus.dst_format, AST_FORMAT_OPUS, 0);
	/* 16khz (wb) */
	ast_format_set(&opustolin16.src_format, AST_FORMAT_OPUS, 0);
	ast_format_set(&opustolin16.dst_format, AST_FORMAT_SLINEAR16, 0);
	ast_format_set(&lin16toopus.src_format, AST_FORMAT_SLINEAR16, 0);
	ast_format_set(&lin16toopus.dst_format, AST_FORMAT_OPUS, 0);
	/* 24khz (swb) */
	ast_format_set(&opustolin24.src_format, AST_FORMAT_OPUS, 0);
	ast_format_set(&opustolin24.dst_format, AST_FORMAT_SLINEAR24, 0);
	ast_format_set(&lin24toopus.src_format, AST_FORMAT_SLINEAR24, 0);
	ast_format_set(&lin24toopus.dst_format, AST_FORMAT_OPUS, 0);
	/* 48khz (fb) */
	ast_format_set(&opustolin48.src_format, AST_FORMAT_OPUS, 0);
	ast_format_set(&opustolin48.dst_format, AST_FORMAT_SLINEAR48, 0);
	ast_format_set(&lin48toopus.src_format, AST_FORMAT_SLINEAR48, 0);
	ast_format_set(&lin48toopus.dst_format, AST_FORMAT_OPUS, 0);

	res |= ast_register_translator(&opustolin);
	res |= ast_register_translator(&lintoopus);
	res |= ast_register_translator(&opustolin12);
	res |= ast_register_translator(&lin12toopus);
	res |= ast_register_translator(&opustolin16);
	res |= ast_register_translator(&lin16toopus);
	res |= ast_register_translator(&opustolin24);
	res |= ast_register_translator(&lin24toopus);
	res |= ast_register_translator(&opustolin48);
	res |= ast_register_translator(&lin48toopus);

	ast_cli_register_multiple(cli_opus, sizeof(cli_opus) / sizeof(struct ast_cli_entry));

	return res;
}

AST_MODULE_INFO(ASTERISK_GPL_KEY, AST_MODFLAG_DEFAULT, "Opus Coder/Decoder",
		.load = load_module,
		.unload = unload_module,
		.reload = reload,
	       );
