#include "telemetry.hpp"

#include <iostream>

int main(const int argc, char** argv) {
    // The executable expects exactly one telemetry log path.
    if (argc != 2) {
        std::cerr << "usage: telemetry_check <input_path>" << std::endl;
        return 1;
    }

    Frame frames[MAX_TELEMETRY_FRAMES];
    const int frame_count = read_frames(argv[1], frames, MAX_TELEMETRY_FRAMES);
    if (frame_count == 0) {
        std::cerr << "error: failed to read telemetry frames" << std::endl;
        return 1;
    }

    const Summary summary = summarize(frames, frame_count);
    print_summary(summary);

    return 0;
}
