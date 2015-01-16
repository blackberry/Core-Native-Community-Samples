/*
 * Copyright (c) 2011-2012 Research In Motion Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <screen/screen.h>
#include <bps/navigator.h>
#include <bps/screen.h>
#include <bps/bps.h>
#include <bps/event.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <math.h>

#include "bbutil_bg.h"

font_t* fontBold16;

static screen_context_t screen_ctx;
static int nScreenWidth, nScreenHeight;

int initialize() {
    int dpi = bbutil_calculate_dpi(screen_ctx);

    if (dpi == EXIT_FAILURE) {
        fprintf(stderr, "init(): Unable to calculate dpi\n");
        return EXIT_FAILURE;
    }
    //As bbutil renders text using device-specifc dpi, we need to compute a point size
    //for the font, so that the text string fits into the bubble. Note that Playbook is used
    //as a reference point in this equation as we know that at dpi of 170, font with point size of
    //15 fits into the bubble texture.
    int point_size16 = (int)(16.0f / ((float)dpi / 170.0f ));

    fontBold16 = bbutil_load_font("/usr/fonts/font_repository/dejavu/DejaVuSans-Bold.ttf", point_size16, dpi);
    if (!fontBold16) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void render() {
    // Increment the angle by 0.5 degrees
    static float angle = 0.0f;
    angle += 0.5f * M_PI / 180.0f;

    //Typical render pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw text
    bbutil_render_text_angle(fontBold16, "Rotate Text", nScreenWidth/2, nScreenHeight/2, 0.75f, 0.75f, 0.75f, 1.0f, angle);

    bbutil_swap();
}

int main(int argc, char *argv[]) {
    int exit_application = 0;

    //Create a screen context that will be used to create an EGL surface to to receive libscreen events
    screen_create_context(&screen_ctx, 0);
	// Get display configuration (dimensions)
	int count = 0;
	screen_get_context_property_iv(screen_ctx, SCREEN_PROPERTY_DISPLAY_COUNT, &count);
	screen_display_t *screen_disps = (screen_display_t *)calloc(count, sizeof(screen_display_t));
	screen_get_context_property_pv(screen_ctx, SCREEN_PROPERTY_DISPLAYS, (void **)screen_disps);
	screen_display_t screen_disp = screen_disps[0];
	free(screen_disps);
	int dims[2] = { 0, 0 };
	screen_get_display_property_iv(screen_disp, SCREEN_PROPERTY_SIZE, dims);
	nScreenWidth = dims[0];
	nScreenHeight = dims[1];

    //Initialize BPS library
    bps_initialize();

    //Use utility code to initialize EGL for rendering with GL ES 2.0
    if (EXIT_SUCCESS != bbutil_init_egl(screen_ctx)) {
        fprintf(stderr, "bbutil_init_egl failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_ctx);
        return 0;
    }

    //Initialize application logic
    if (EXIT_SUCCESS != initialize()) {
        fprintf(stderr, "initialize failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_ctx);
        bps_shutdown();
        return 0;
    }

    while (!exit_application) {
        //Request and process all available BPS events
        bps_event_t *event = NULL;

        for(;;) {
            if (BPS_SUCCESS != bps_get_event(&event, 0)) {
                fprintf(stderr, "bps_get_event failed\n");
                break;
            }

            if (event) {
                int domain = bps_event_get_domain(event);

                if ((domain == navigator_get_domain())
                     && (NAVIGATOR_EXIT == bps_event_get_code(event))) {
                    exit_application = 1;
                }
            } else {
                break;
            }
        }
        render();
    }

    //Stop requesting events from libscreen
    screen_stop_events(screen_ctx);

    //Shut down BPS library for this process
    bps_shutdown();

    //Use utility code to terminate EGL setup
    bbutil_terminate();

    //Destroy libscreen context
    screen_destroy_context(screen_ctx);
    return 0;
}
