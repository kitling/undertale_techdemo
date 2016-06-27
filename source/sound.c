#include <tremor/ivorbiscodec.h>
#include <tremor/ivorbisfile.h>
#include "sound.h"
#include <3ds.h>

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

void audio_init() {
    ndspInit();
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);
}

struct sound* sound_create(enum channel chan) {
    struct sound *new_sound = (struct sound*)malloc(sizeof(struct sound));
    if (new_sound == NULL) return NULL;

    new_sound->block_pos = 0;
    new_sound->block = false;
    new_sound->channel = chan;

    memset(&(new_sound->waveBuf[0]), 0, sizeof(ndspWaveBuf));
    memset(&(new_sound->waveBuf[1]), 0, sizeof(ndspWaveBuf));

    memset(new_sound->mix, 0, sizeof(new_sound->mix));
    new_sound->mix[0] =
    new_sound->mix[1] = 1.0;

    ndspChnSetInterp(new_sound->channel, NDSP_INTERP_LINEAR);
    ndspChnSetFormat(new_sound->channel, NDSP_FORMAT_STEREO_PCM16);
    ndspChnSetMix(new_sound->channel, new_sound->mix);

    return new_sound;
}

// Audio load/play
void audio_load_ogg(const char *name, struct sound *sound) {
	const unsigned long sample_size = 4;
    const unsigned long buffer_size = 40960;
    const unsigned long num_samples = buffer_size / sample_size;

    /// Copied from ivorbisfile_example.c
    FILE *mus = fopen(name, "rb");
    sound->vf = (OggVorbis_File*)malloc(sizeof(OggVorbis_File));
    if ((sound->status = ov_open(mus, sound->vf, NULL, 0))) {
        free(sound->vf);
        return;
    }

    ndspChnSetRate(sound->channel, ov_bitrate(sound->vf, -1));

    sound->waveBuf[0].nsamples =
    sound->waveBuf[1].nsamples = num_samples;

    sound->waveBuf[0].status =
    sound->waveBuf[1].status = NDSP_WBUF_DONE;

    sound->waveBuf[0].data_vaddr = linearAlloc(buffer_size);
    sound->waveBuf[1].data_vaddr = linearAlloc(buffer_size);
}

void sound_loop(struct sound *sound) {
    // if (mus_failure <= 0) return;

    long size = sound->waveBuf[sound->block].nsamples * 4 - sound->block_pos;

    if (sound->waveBuf[sound->block].status == NDSP_WBUF_DONE){
        sound->status = ov_read(sound->vf, (char*)sound->waveBuf[sound->block].data_vaddr + sound->block_pos, size, &sound->section);

        if (sound->status <= 0) {
            ov_clear(sound->vf);

            if (sound->status < 0) ndspChnReset(sound->channel);

        } else {
            sound->block_pos += sound->status;
            if (sound->status == size) {
                sound->block_pos = 0;
                ndspChnWaveBufAdd(sound->channel, &sound->waveBuf[sound->block]);
                sound->block = !sound->block;
            }
        }
    }
}

void sound_stop(struct sound *sound) {
    ndspChnReset(sound->channel);
    GSPGPU_FlushDataCache(sound->waveBuf[0].data_vaddr, sound->waveBuf[0].nsamples * 4);
    GSPGPU_FlushDataCache(sound->waveBuf[1].data_vaddr, sound->waveBuf[1].nsamples * 4);
    linearFree((void*)sound->waveBuf[0].data_vaddr);
    linearFree((void*)sound->waveBuf[1].data_vaddr);
    free(sound->vf);
    // memset (buffer, 0, size);
}

void audio_stop(void) {
    ndspExit();
}
