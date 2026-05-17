#include "telemetry.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>

// Debugging exercise notes:
// this file intentionally contains four runtime defects.
// The defects are related to malformed input shape, invalid numeric values,
// unsafe time deltas, and empty logs. Exact locations are not marked on purpose.

constexpr int EXPECTED_FIELD_COUNT = 7;
constexpr int MAX_LINE_LENGTH = 256;

int split_line(char line[], char* fields[], const int max_fields) {
    int count = 0;
    int pos = 0;
    char* cursor = line;
    char* field_pointer = line;

    while (count <= max_fields) {
        if (*cursor == '\t') {
            throw std::runtime_error("unexpected character at pos " + std::to_string(pos));
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
        throw std::runtime_error("invalid long value \"" + std::string(text) + "\"");
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
        throw std::runtime_error("invalid double value \"" + std::string(text) + "\"");
    }

    return value;
}

Frame parse_frame(char line[]) {
    char* fields[EXPECTED_FIELD_COUNT] = {};
    const int field_count = split_line(line, fields, EXPECTED_FIELD_COUNT);
    (void)field_count;

    if (field_count != EXPECTED_FIELD_COUNT) {
        const std::string details = (field_count < EXPECTED_FIELD_COUNT) ? fields[field_count] : std::string("more");
        throw std::runtime_error("expected " + std::to_string(EXPECTED_FIELD_COUNT) + " fields, got " + details);
    }

    auto frame = Frame{};
    frame.timestamp_ms = parse_long(fields[0]);
    frame.seq = parse_int(fields[1]);
    frame.voltage_v = parse_double(fields[2]);
    frame.current_a = parse_double(fields[3]);
    frame.temperature_c = parse_double(fields[4]);
    frame.gps_fix = parse_int(fields[5]);
    frame.satellites = parse_int(fields[6]);

    return frame;
}

double compute_frame_rate_hz(const Frame frames[], const int frame_count) {
    const long elapsed_ms = frames[frame_count - 1].timestamp_ms - frames[0].timestamp_ms;

    return static_cast<double>((frame_count - 1) * 1000 / elapsed_ms);
}


// 5. Валідація й очікувана поведінка
void validate_frame(const Frame frames[], const int frame_count) {
    if (frame_count > 0) {
        // timestamp_ms зростає;
        const long delta_ms = frames[frame_count].timestamp_ms - frames[frame_count - 1].timestamp_ms;
        if (delta_ms <= 0) {
            throw std::runtime_error("non-positive time delta");
        }

        // seq зростає на 1
        const int delta_seq = frames[frame_count].seq - frames[frame_count - 1].seq;
        if (delta_seq != 1) {
            throw std::runtime_error("seq not incremented by 1");
        }
    }

    // voltage_v > 0;
    if (frames[frame_count].voltage_v <= 0) {
        throw std::runtime_error("non-positive voltage \"" + std::to_string(frames[frame_count].voltage_v) + "\"");
    }

    // temperature_c у діапазоні [-40, 120];
    if (frames[frame_count].temperature_c < -40.0f || frames[frame_count].temperature_c > 120.0f) {
        throw std::runtime_error("temperature \"" + std::to_string(frames[frame_count].temperature_c) + "\" out of range");;
    }

    // gps_fix дорівнює 0 або 1;
    if (frames[frame_count].gps_fix != 0 && frames[frame_count].gps_fix != 1) {
        throw std::runtime_error("invalid gps_fix value \"" + std::to_string(frames[frame_count].gps_fix) + "\"");
    }

    // satellites >= 0.
    if (frames[frame_count].satellites < 0) {
        throw std::runtime_error("negative satellites count");
    }
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
            try {
                frames[frame_count] = parse_frame(line);
                validate_frame(frames, frame_count);
            } catch (const std::exception& e) {
                std::cerr << "error: invalid frame at line " << line_num << ": " << e.what() << std::endl;
                return 0;
            }

            // Validate delta
            if (frame_count > 0) {
                const long delta_ms = frames[frame_count].timestamp_ms - frames[frame_count - 1].timestamp_ms;
                if (delta_ms <= 0) {
                    std::cerr << "error: invalid frame at line " << line_num << ": non-positive time delta" << std::endl;
                    return 0;
                }
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
