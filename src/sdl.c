#include "chip8.h"

#define SAMPLE_RATE 44100
#define BEEP_FREQUENCY 440
#define AUDIO_BUFFER_SAMPLES (SAMPLE_RATE / 50)

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_AudioStream *stream = NULL;

static bool audio_enabled = false;
static int waveform_position = 0;

// Array indices are CHIP-8 key values; entries are physical keyboard keys.
static const SDL_Scancode chip8_keymap[16] = {
    SDL_SCANCODE_X,  // 0x0
    SDL_SCANCODE_1,  // 0x1
    SDL_SCANCODE_2,  // 0x2
    SDL_SCANCODE_3,  // 0x3
    SDL_SCANCODE_Q,  // 0x4
    SDL_SCANCODE_W,  // 0x5
    SDL_SCANCODE_E,  // 0x6
    SDL_SCANCODE_A,  // 0x7
    SDL_SCANCODE_S,  // 0x8
    SDL_SCANCODE_D,  // 0x9
    SDL_SCANCODE_Z,  // 0xA
    SDL_SCANCODE_C,  // 0xB
    SDL_SCANCODE_4,  // 0xC
    SDL_SCANCODE_R,  // 0xD
    SDL_SCANCODE_F,  // 0xE
    SDL_SCANCODE_V   // 0xF
};

int initialize_SDL(int width, int height) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("CHIP-8 Emulator", width, height, 0);

    if (!window){
        SDL_Log("Couldn't create SDL window: %s", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, NULL);

    if (!renderer){
        SDL_Log("Couldn't create SDL renderer: %s", SDL_GetError());
        return 1;
    }

    SDL_SetRenderLogicalPresentation(renderer, 64, 32, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    if (SDL_Init(SDL_INIT_AUDIO)) {
        SDL_AudioSpec spec;
        spec.channels = 1;         
        spec.format = SDL_AUDIO_F32;  
        spec.freq = SAMPLE_RATE;           

        stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);

        if (!stream) {
            SDL_Log("Couldn't create SDL audio stream: %s", SDL_GetError());
        }
        else {
            SDL_ResumeAudioStreamDevice(stream);
        }
    }
    else {
        SDL_Log("Couldn't initialize SDL audio: %s", SDL_GetError());
    }

    return 0;
}

int generate_square_waveform(float *samples, int count, int position) {
    // Alternate positive and negative samples to produce a square-wave beep.
    int samples_per_cycle = SAMPLE_RATE / BEEP_FREQUENCY;

    for (int i = 0; i < count; i++) {
        int phase = position % samples_per_cycle;

        if (phase < samples_per_cycle / 2)
            samples[i] = 0.15f;
        else
            samples[i] = -0.15f;

        position = (position + 1) % samples_per_cycle;
    }

    return position;
}

void audio_streaming(float *samples, int count) {
    if (!audio_enabled || !stream)
        return;

    // Keep roughly 20 ms of samples queued without growing the audio buffer.
    int queued_bytes = SDL_GetAudioStreamQueued(stream);

    if (queued_bytes < 0)
        return;

    int queued_samples = queued_bytes / (int)sizeof(float);
    int missing_samples = AUDIO_BUFFER_SAMPLES - queued_samples;

    if (missing_samples <= 0)
        return;

    if (missing_samples > count)
        missing_samples = count;

    int next_position = generate_square_waveform(samples, missing_samples, waveform_position);
    
    if (SDL_PutAudioStreamData(stream, samples, missing_samples * (int)sizeof(float))) {
        waveform_position = next_position;
    } 
}

void update_audio() {
    float samples[AUDIO_BUFFER_SAMPLES];

    audio_streaming(samples, AUDIO_BUFFER_SAMPLES);
}

void set_audio_flag(bool condition) {
    // Discard remaining samples immediately when the sound timer expires.
    if (audio_enabled && !condition && stream) {
        SDL_ClearAudioStream(stream);
    }

    audio_enabled = condition;
}

bool process_input(Chip8 *chip8) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                return false;

            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                for (int i = 0; i < 16; i++) {
                    if (event.key.scancode == chip8_keymap[i]) {
                        chip8->key[i] = (event.type == SDL_EVENT_KEY_DOWN);
                        break;
                    }
                }

                break;

            default:
                break;
        }
    }

    return true;
}

void draw_pixels(uint8_t gfx[64 * 32]) {
    // Clearing display
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int i = 0; i < (64 * 32); i++)
    {
        if (gfx[i] == 1) {
            SDL_RenderPoint(renderer, i % 64, i / 64);
        }
    }

    SDL_RenderPresent(renderer);
}

void destroy_SDL() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyAudioStream(stream);
    SDL_Quit();
}