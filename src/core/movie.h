// Copyright 2017 Citra Emulator Project
// Copyright 2024 Borked3DS Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <functional>
#include <span>
#include <boost/serialization/vector.hpp>
#include "common/common_types.h"

namespace Service {
namespace HID {
struct AccelerometerDataEntry;
struct GyroscopeDataEntry;
struct PadState;
struct TouchDataEntry;
} // namespace HID
namespace IR {
struct ExtraHIDResponse;
union PadState;
} // namespace IR
} // namespace Service

namespace Core {

class System;
struct CTMHeader;
struct ControllerState;

class Movie {
public:
    enum class PlayMode : u32 {
        None,
        Recording,
        Playing,
        MovieFinished,
    };

    enum class ValidationResult : u32 {
        OK,
        RevisionDismatch,
        InputCountDismatch,
        Invalid,
    };

    explicit Movie(const Core::System& system);
    ~Movie();

    void SetPlaybackCompletionCallback(std::function<void()> completion_callback);
    void StartPlayback(const std::string& movie_file);
    void StartRecording(const std::string& movie_file, const std::string& author);

    /**
     * Sets the read-only status.
     * When true, movies will be opened in read-only mode. Loading a state will resume playback
     * from that state.
     * When false, movies will be opened in read/write mode. Loading a state will start recording
     * from that state (rerecording). To start rerecording without loading a state, one can save
     * and then immediately load while in R/W.
     *
     * The default is true.
     */
    void SetReadOnly(bool read_only);

    /// Prepare to override the clock before playing back movies
    void PrepareForPlayback(const std::string& movie_file);

    /// Prepare to override the clock before recording movies
    void PrepareForRecording();

    ValidationResult ValidateMovie(const std::string& movie_file) const;

    /// Get the init time that would override the one in the settings
    u64 GetOverrideInitTime() const;

    /// Get the base system ticks value that would override the one generated by core timing
    s64 GetOverrideBaseTicks() const;

    struct MovieMetadata {
        u64 program_id;
        std::string author;
        u32 rerecord_count;
        u64 input_count;
    };
    MovieMetadata GetMovieMetadata(const std::string& movie_file) const;

    /// Get the current movie's unique ID. Used to provide separate savestate slots for movies.
    u64 GetCurrentMovieID() const {
        return id;
    }

    void Shutdown();

    /**
     * When recording: Takes a copy of the given input states so they can be used for playback
     * When playing: Replaces the given input states with the ones stored in the playback file
     */
    void HandlePadAndCircleStatus(Service::HID::PadState& pad_state, s16& circle_pad_x,
                                  s16& circle_pad_y);

    /**
     * When recording: Takes a copy of the given input states so they can be used for playback
     * When playing: Replaces the given input states with the ones stored in the playback file
     */
    void HandleTouchStatus(Service::HID::TouchDataEntry& touch_data);

    /**
     * When recording: Takes a copy of the given input states so they can be used for playback
     * When playing: Replaces the given input states with the ones stored in the playback file
     */
    void HandleAccelerometerStatus(Service::HID::AccelerometerDataEntry& accelerometer_data);

    /**
     * When recording: Takes a copy of the given input states so they can be used for playback
     * When playing: Replaces the given input states with the ones stored in the playback file
     */
    void HandleGyroscopeStatus(Service::HID::GyroscopeDataEntry& gyroscope_data);

    /**
     * When recording: Takes a copy of the given input states so they can be used for playback
     * When playing: Replaces the given input states with the ones stored in the playback file
     */
    void HandleIrRst(Service::IR::PadState& pad_state, s16& c_stick_x, s16& c_stick_y);

    /**
     * When recording: Takes a copy of the given input states so they can be used for playback
     * When playing: Replaces the given input states with the ones stored in the playback file
     */
    void HandleExtraHidResponse(Service::IR::ExtraHIDResponse& extra_hid_response);
    PlayMode GetPlayMode() const;

    u64 GetCurrentInputIndex() const;
    u64 GetTotalInputCount() const;

    /**
     * Saves the movie immediately, in its current state.
     * This is called in Shutdown.
     */
    void SaveMovie();

private:
    void CheckInputEnd();

    template <typename... Targs>
    void Handle(Targs&... Fargs);

    void Play(Service::HID::PadState& pad_state, s16& circle_pad_x, s16& circle_pad_y);
    void Play(Service::HID::TouchDataEntry& touch_data);
    void Play(Service::HID::AccelerometerDataEntry& accelerometer_data);
    void Play(Service::HID::GyroscopeDataEntry& gyroscope_data);
    void Play(Service::IR::PadState& pad_state, s16& c_stick_x, s16& c_stick_y);
    void Play(Service::IR::ExtraHIDResponse& extra_hid_response);

    void Record(const ControllerState& controller_state);
    void Record(const Service::HID::PadState& pad_state, const s16& circle_pad_x,
                const s16& circle_pad_y);
    void Record(const Service::HID::TouchDataEntry& touch_data);
    void Record(const Service::HID::AccelerometerDataEntry& accelerometer_data);
    void Record(const Service::HID::GyroscopeDataEntry& gyroscope_data);
    void Record(const Service::IR::PadState& pad_state, const s16& c_stick_x, const s16& c_stick_y);
    void Record(const Service::IR::ExtraHIDResponse& extra_hid_response);

    ValidationResult ValidateHeader(const CTMHeader& header) const;
    ValidationResult ValidateInput(std::span<const u8> input, u64 expected_count) const;

private:
    const Core::System& system;
    PlayMode play_mode;

    std::string record_movie_file;
    std::string record_movie_author;

    u64 init_time;       // Clock init time override for RNG consistency
    s64 base_ticks = -1; // Core timing base system ticks override for RNG consistency

    std::vector<u8> recorded_input;
    std::size_t current_byte = 0;
    u64 current_input = 0;
    // Total input count of the current movie being played. Not used for recording.
    u64 total_input = 0;

    u64 id = 0; // ID of the current movie loaded
    u64 program_id = 0;
    u32 rerecord_count = 1;
    bool read_only = true;

    std::function<void()> playback_completion_callback = [] {};

    template <class Archive>
    void serialize(Archive& ar, const unsigned int file_version);
    friend class boost::serialization::access;
};
} // namespace Core

BOOST_CLASS_VERSION(Core::Movie, 1)
