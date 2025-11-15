/* common.h
 *
 * Copyright 2022 abb128
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

#pragma once

#include <string.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#define SWEAR_REPLACEMENT " [__]"

#define LIVECAPTIONS_VERSION "0.4.1"

#define MINIMUM_BENCHMARK_RESULT (0.6)

// Helper function to get the model path
static inline const char* get_model_path_impl(void) {
    // First check environment variable
    const char *env_path = getenv("APRIL_MODEL_PATH");
    if (env_path != NULL) return env_path;
    
    // Try macOS bundle path
    static char bundle_path[1024] = {0};
    if (bundle_path[0] == '\0') {
        // Get the executable path
        uint32_t size = sizeof(bundle_path);
        if (_NSGetExecutablePath(bundle_path, &size) == 0) {
            // Remove "/Contents/MacOS/livecaptions" and append "/Contents/Resources/april-english-dev-01110_en.april"
            char *contents = strstr(bundle_path, "/Contents/MacOS");
            if (contents != NULL) {
                strcpy(contents, "/Contents/Resources/april-english-dev-01110_en.april");
                return bundle_path;
            }
        }
    } else {
        return bundle_path;
    }
    
    // Fall back to Flatpak path
    return "/app/LiveCaptions/models/aprilv0_en-us.april";
}

#define GET_MODEL_PATH() get_model_path_impl()
