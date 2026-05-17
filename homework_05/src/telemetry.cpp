#include "telemetry.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>

// Debugging exercise notes:
// this file intentionally contains four runtime defects.
// The defects are related to malformed input shape, invalid numeric values,
// unsafe time deltas, and empty logs. Exact locations are not marked on purpose.

const int EXPECTED_FIELD_COUNT = 7;
const int MAX_LINE_LENGTH = 256;

int split_line(char line[], char* fields[], const int max_fields, const int line_num) {
    int count = 0;
    int pos = 0;
    char* cursor = line;
    char* field_pointer = line;

    while (count <= max_fields) {
        if (*cursor == '\t') {
            std::cerr << "error: invalid frame at line " << line_num << ": unexpected character at pos " << pos << std::endl;
            std::abort();
        }

        if (*cursor == '\n' || *cursor == '\r') {
            *cursor = '\0';
            pos = 0;
        }

        if (*cursor == ' ' || *cursor == '\0') {
            fields[count] = field_pointer;
            field_pointer = cursor + 1;
            ++count;
        }

        if (*cursor == '\0') {
            break;
        }

        ++cursor;
    }

    return count;
}

long parse_long(const char* text) {
    char* end = nullptr;
    const long value = std::strtol(text, &end, 10);

    if (end == text) {
        std::abort();
    }

    return value;
}

int parse_int(const char* text) {
    return static_cast<int>(parse_long(text));
}

double parse_double(const char* text) {
    char* end = nullptr;
    const double value = std::strtod(text, &end);

    if (end == text) {
        std::abort();
    }

    return value;
}

bool parse_frame(char line[], const int line_num, Frame &out_frame) {
    char* fields[EXPECTED_FIELD_COUNT] = {};
    const int field_count = split_line(line, fields, EXPECTED_FIELD_COUNT, line_num);
    (void)field_count;

    if (field_count != EXPECTED_FIELD_COUNT) {
        std::cerr << "error: invalid frame at line " << line_num << ": expected " << EXPECTED_FIELD_COUNT << " fields, ";
        if (field_count < EXPECTED_FIELD_COUNT) {
            std::cerr << "got " << field_count << std::endl;
        } else {
            std::cerr << "got more" << std::endl;
        }

        return false;
    }

    out_frame.timestamp_ms = parse_long(fields[0]);
    out_frame.seq = parse_int(fields[1]);
    out_frame.voltage_v = parse_double(fields[2]);
    out_frame.current_a = parse_double(fields[3]);
    out_frame.temperature_c = parse_double(fields[4]);
    out_frame.gps_fix = parse_int(fields[5]);
    out_frame.satellites = parse_int(fields[6]);
    return true;
}

double compute_frame_rate_hz(const Frame frames[], const int frame_count) {
    const long elapsed_ms = frames[frame_count - 1].timestamp_ms - frames[0].timestamp_ms;

    return static_cast<double>((frame_count - 1) * 1000 / elapsed_ms);
}

int read_frames(const char* path, Frame frames[], const int max_frames) {
    std::ifstream input{path};
    if (!input) {
        std::cerr << "error: failed to open input file: " << path << '\n';
        return 0;
    }

    int frame_count = 0;
    int line_num = 0;
    char line[MAX_LINE_LENGTH];

    while (input.getline(line, MAX_LINE_LENGTH)) {
        if (line[0] == '\0') {
            continue;
        }

        ++line_num;

        if (frame_count < max_frames) {
            if (!parse_frame(line, line_num, frames[frame_count])) {
                return 0;
            }
            ++frame_count;
        }
    }

    return frame_count;
}

Summary summarize(const Frame frames[], const int frame_count) {
    Summary summary{};
    summary.frames_total = frame_count;
    summary.frames_valid = frame_count;
    summary.voltage_min = frames[0].voltage_v;
    summary.voltage_max = frames[0].voltage_v;
    summary.low_voltage_frames = 0;

    double temperature_sum = 0.0;

    for (int i = 0; i < frame_count; ++i) {
        if (frames[i].voltage_v < summary.voltage_min) {
            summary.voltage_min = frames[i].voltage_v;
        }

        if (frames[i].voltage_v > summary.voltage_max) {
            summary.voltage_max = frames[i].voltage_v;
        }

        temperature_sum += frames[i].temperature_c;

        if (frames[i].voltage_v < 22.0) {
            ++summary.low_voltage_frames;
        }
    }

    const int temperature_tenths = static_cast<int>(temperature_sum * 10.0) / frame_count;
    summary.temperature_avg = static_cast<double>(temperature_tenths) / 10.0;
    summary.frame_rate_hz = compute_frame_rate_hz(frames, frame_count);
    return summary;
}

void print_summary(const Summary& summary) {
    std::cout << "frames_total " << summary.frames_total << '\n';
    std::cout << "frames_valid " << summary.frames_valid << '\n';
    std::cout << "voltage_min " << summary.voltage_min << '\n';
    std::cout << "voltage_max " << summary.voltage_max << '\n';
    std::cout << "temperature_avg " << summary.temperature_avg << '\n';
    std::cout << "low_voltage_frames " << summary.low_voltage_frames << '\n';
    std::cout << "frame_rate_hz " << summary.frame_rate_hz << '\n';
}
