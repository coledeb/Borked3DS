// Copyright 2014 Citra Emulator Project
// Copyright 2024 Borked3DS Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <algorithm>
#include <array>
#include <QKeySequence>
#include <QSettings>
#include "borked3ds_qt/configuration/config.h"
#include "common/file_util.h"
#include "common/settings.h"
#include "core/hle/service/service.h"
#include "input_common/main.h"
#include "input_common/udp/client.h"
#include "network/network.h"
#include "network/network_settings.h"

Config::Config(const std::string& config_name, ConfigType config_type) : type{config_type} {
    global = config_type == ConfigType::GlobalConfig;
    Initialize(config_name);
}

Config::~Config() {
    if (global) {
        Save();
    }
}

const std::array<int, Settings::NativeButton::NumButtons> Config::default_buttons = {
    Qt::Key_A, Qt::Key_S, Qt::Key_Z, Qt::Key_X, Qt::Key_T, Qt::Key_G,
    Qt::Key_F, Qt::Key_H, Qt::Key_Q, Qt::Key_W, Qt::Key_M, Qt::Key_N,
    Qt::Key_O, Qt::Key_P, Qt::Key_1, Qt::Key_2, Qt::Key_B, Qt::Key_V,
};

const std::array<std::array<int, 9>, Settings::NativeAnalog::NumAnalogs> Config::default_analogs{{
    {
        Qt::Key_Up,    // Up
        Qt::Key_Down,  // Down
        Qt::Key_Left,  // Left
        Qt::Key_Right, // Right
        Qt::Key_W,     // Up-Right
        Qt::Key_Q,     // Up-Left
        Qt::Key_E,     // Down-Right
        Qt::Key_A,     // Down-Left
        Qt::Key_D      // Modifier
    },
    {
        Qt::Key_I, // Up
        Qt::Key_K, // Down
        Qt::Key_J, // Left
        Qt::Key_L, // Right
        Qt::Key_U, // Up-Right
        Qt::Key_H, // Up-Left
        Qt::Key_O, // Down-Right
        Qt::Key_B, // Down-Left
        Qt::Key_D  // Modifier
    },
}};

// This shouldn't have anything except static initializers (no functions). So
// QKeySequence(...).toString() is NOT ALLOWED HERE.
// This must be in alphabetical order according to action name as it must have the same order as
// UISetting::values.shortcuts, which is alphabetically ordered.
// clang-format off
const std::array<UISettings::Shortcut, 38> Config::default_hotkeys {{
     {QStringLiteral("Advance Frame"),            QStringLiteral("Main Window"), {QStringLiteral(""),       Qt::ApplicationShortcut}},
     {QStringLiteral("Audio Mute/Unmute"),        QStringLiteral("Main Window"), {QStringLiteral("Ctrl+M"), Qt::WindowShortcut}},
     {QStringLiteral("Audio Volume Down"),        QStringLiteral("Main Window"), {QStringLiteral(""),       Qt::WindowShortcut}},
     {QStringLiteral("Audio Volume Up"),          QStringLiteral("Main Window"), {QStringLiteral(""),       Qt::WindowShortcut}},
     {QStringLiteral("Capture Screenshot"),       QStringLiteral("Main Window"), {QStringLiteral("Ctrl+P"), Qt::WidgetWithChildrenShortcut}},
     {QStringLiteral("Continue/Pause Emulation"), QStringLiteral("Main Window"), {QStringLiteral("F4"),     Qt::WindowShortcut}},
     {QStringLiteral("Decrease 3D Factor"),       QStringLiteral("Main Window"), {QStringLiteral("Ctrl+-"), Qt::ApplicationShortcut}},
     {QStringLiteral("Decrease Speed Limit"),     QStringLiteral("Main Window"), {QStringLiteral("-"),      Qt::ApplicationShortcut}},
     {QStringLiteral("Exit Borked3DS"),               QStringLiteral("Main Window"), {QStringLiteral("Ctrl+Q"), Qt::WindowShortcut}},
     {QStringLiteral("Exit Fullscreen"),          QStringLiteral("Main Window"), {QStringLiteral("Esc"),    Qt::WindowShortcut}},
     {QStringLiteral("Fullscreen"),               QStringLiteral("Main Window"), {QStringLiteral("F11"),    Qt::WindowShortcut}},
     {QStringLiteral("Increase 3D Factor"),       QStringLiteral("Main Window"), {QStringLiteral("Ctrl++"), Qt::ApplicationShortcut}},
     {QStringLiteral("Increase Speed Limit"),     QStringLiteral("Main Window"), {QStringLiteral("+"),      Qt::ApplicationShortcut}},
     {QStringLiteral("Load Amiibo"),              QStringLiteral("Main Window"), {QStringLiteral("F2"),     Qt::WidgetWithChildrenShortcut}},
     {QStringLiteral("Load File"),                QStringLiteral("Main Window"), {QStringLiteral("Ctrl+O"), Qt::WidgetWithChildrenShortcut}},
     {QStringLiteral("Load from Newest Non-Quick Slot"),    QStringLiteral("Main Window"), {QStringLiteral("Ctrl+V"), Qt::WindowShortcut}},
     {QStringLiteral("Multiplayer Browse Public Game Lobby"), QStringLiteral("Main Window"), {QStringLiteral("Ctrl+B"), Qt::ApplicationShortcut}},
     {QStringLiteral("Multiplayer Create Room"),              QStringLiteral("Main Window"), {QStringLiteral("Ctrl+N"), Qt::ApplicationShortcut}},
     {QStringLiteral("Multiplayer Direct Connect to Room"),   QStringLiteral("Main Window"), {QStringLiteral("Ctrl+Shift"), Qt::ApplicationShortcut}},
     {QStringLiteral("Multiplayer Leave Room"),               QStringLiteral("Main Window"), {QStringLiteral("Ctrl+L"), Qt::ApplicationShortcut}},
     {QStringLiteral("Multiplayer Show Current Room"),        QStringLiteral("Main Window"), {QStringLiteral("Ctrl+R"), Qt::ApplicationShortcut}},
     {QStringLiteral("Quick Save"),                QStringLiteral("Main Window"), {QStringLiteral(""),       Qt::WindowShortcut}},
     {QStringLiteral("Quick Load"),               QStringLiteral("Main Window"), {QStringLiteral(""),       Qt::WindowShortcut}},
     {QStringLiteral("Remove Amiibo"),            QStringLiteral("Main Window"), {QStringLiteral("F3"),     Qt::ApplicationShortcut}},
     {QStringLiteral("Restart Emulation"),        QStringLiteral("Main Window"), {QStringLiteral("F6"),     Qt::WindowShortcut}},
     {QStringLiteral("Rotate Screens Upright"),   QStringLiteral("Main Window"), {QStringLiteral("F8"),     Qt::WindowShortcut}},
     {QStringLiteral("Save to Oldest Non-Quick Slot"),      QStringLiteral("Main Window"), {QStringLiteral("Ctrl+C"), Qt::WindowShortcut}},
     {QStringLiteral("Stop Emulation"),           QStringLiteral("Main Window"), {QStringLiteral("F5"),     Qt::WindowShortcut}},
     {QStringLiteral("Swap Screens"),             QStringLiteral("Main Window"), {QStringLiteral("F9"),     Qt::WindowShortcut}},
     {QStringLiteral("Toggle 3D"),                QStringLiteral("Main Window"), {QStringLiteral("Ctrl+3"), Qt::ApplicationShortcut}},
     {QStringLiteral("Toggle Turbo Mode"),     QStringLiteral("Main Window"), {QStringLiteral(""),      Qt::ApplicationShortcut}},
     {QStringLiteral("Toggle Custom Textures"),   QStringLiteral("Main Window"), {QStringLiteral("F7"),     Qt::ApplicationShortcut}},
     {QStringLiteral("Toggle Filter Bar"),        QStringLiteral("Main Window"), {QStringLiteral("Ctrl+F"), Qt::WindowShortcut}},
     {QStringLiteral("Toggle Frame Advancing"),   QStringLiteral("Main Window"), {QStringLiteral("Ctrl+A"), Qt::ApplicationShortcut}},
     {QStringLiteral("Toggle Per-Game Speed"),    QStringLiteral("Main Window"), {QStringLiteral("Ctrl+Z"), Qt::ApplicationShortcut}},
     {QStringLiteral("Toggle Screen Layout"),     QStringLiteral("Main Window"), {QStringLiteral("F10"),    Qt::WindowShortcut}},
     {QStringLiteral("Toggle Status Bar"),        QStringLiteral("Main Window"), {QStringLiteral("Ctrl+S"), Qt::WindowShortcut}},
     {QStringLiteral("Toggle Texture Dumping"),   QStringLiteral("Main Window"), {QStringLiteral(""),       Qt::ApplicationShortcut}},
    }};
// clang-format on

void Config::Initialize(const std::string& config_name) {
    const std::string fs_config_loc = FileUtil::GetUserPath(FileUtil::UserPath::ConfigDir);
    const std::string config_file = fmt::format("{}.ini", config_name);

    switch (type) {
    case ConfigType::GlobalConfig:
        qt_config_loc = fmt::format("{}/{}", fs_config_loc, config_file);
        break;
    case ConfigType::PerGameConfig:
        qt_config_loc = fmt::format("{}/custom/{}", fs_config_loc, config_file);
        break;
    }

    FileUtil::CreateFullPath(qt_config_loc);
    qt_config =
        std::make_unique<QSettings>(QString::fromStdString(qt_config_loc), QSettings::IniFormat);
    Reload();
}

/* {Read,Write}BasicSetting and WriteGlobalSetting templates must be defined here before their
 * usages later in this file. This allows explicit definition of some types that don't work
 * nicely with the general version.
 */

// Explicit std::string definition: Qt can't implicitly convert a std::string to a QVariant, nor
// can it implicitly convert a QVariant back to a {std::,Q}string
template <>
void Config::ReadBasicSetting(Settings::Setting<std::string>& setting) {
    const QString name = QString::fromStdString(setting.GetLabel());
    const auto default_value = QString::fromStdString(setting.GetDefault());
    if (qt_config->value(name + QStringLiteral("/default"), false).toBool()) {
        setting.SetValue(default_value.toStdString());
    } else {
        setting.SetValue(qt_config->value(name, default_value).toString().toStdString());
    }
}

template <typename Type, bool ranged>
void Config::ReadBasicSetting(Settings::Setting<Type, ranged>& setting) {
    const QString name = QString::fromStdString(setting.GetLabel());
    const Type default_value = setting.GetDefault();
    if (qt_config->value(name + QStringLiteral("/default"), false).toBool()) {
        setting.SetValue(default_value);
    } else {
        QVariant value{};
        if constexpr (std::is_enum_v<Type>) {
            using TypeU = std::underlying_type_t<Type>;
            value = qt_config->value(name, static_cast<TypeU>(default_value));
            setting.SetValue(static_cast<Type>(value.value<TypeU>()));
        } else {
            value = qt_config->value(name, QVariant::fromValue(default_value));
            setting.SetValue(value.value<Type>());
        }
    }
}

template <typename Type, bool ranged>
void Config::ReadGlobalSetting(Settings::SwitchableSetting<Type, ranged>& setting) {
    QString name = QString::fromStdString(setting.GetLabel());
    const bool use_global = qt_config->value(name + QStringLiteral("/use_global"), true).toBool();
    setting.SetGlobal(use_global);
    if (global || !use_global) {
        QVariant default_value{};
        if constexpr (std::is_enum_v<Type>) {
            using TypeU = std::underlying_type_t<Type>;
            default_value = QVariant::fromValue<TypeU>(static_cast<TypeU>(setting.GetDefault()));
            setting.SetValue(static_cast<Type>(ReadSetting(name, default_value).value<TypeU>()));
        } else {
            default_value = QVariant::fromValue<Type>(setting.GetDefault());
            setting.SetValue(ReadSetting(name, default_value).value<Type>());
        }
    }
}

template <>
void Config::ReadGlobalSetting(Settings::SwitchableSetting<std::string>& setting) {
    QString name = QString::fromStdString(setting.GetLabel());
    const bool use_global = qt_config->value(name + QStringLiteral("/use_global"), true).toBool();
    setting.SetGlobal(use_global);
    if (global || !use_global) {
        const QString default_value = QString::fromStdString(setting.GetDefault());
        setting.SetValue(
            ReadSetting(name, QVariant::fromValue(default_value)).toString().toStdString());
    }
}

// Explicit std::string definition: Qt can't implicitly convert a std::string to a QVariant
template <>
void Config::WriteBasicSetting(const Settings::Setting<std::string>& setting) {
    const QString name = QString::fromStdString(setting.GetLabel());
    const std::string& value = setting.GetValue();
    qt_config->setValue(name + QStringLiteral("/default"), value == setting.GetDefault());
    qt_config->setValue(name, QString::fromStdString(value));
}

// Explicit u16 definition: Qt would store it as QMetaType otherwise, which is not human-readable
template <>
void Config::WriteBasicSetting(const Settings::Setting<u16>& setting) {
    const QString name = QString::fromStdString(setting.GetLabel());
    const u16& value = setting.GetValue();
    qt_config->setValue(name + QStringLiteral("/default"), value == setting.GetDefault());
    qt_config->setValue(name, static_cast<u32>(value));
}

template <typename Type, bool ranged>
void Config::WriteBasicSetting(const Settings::Setting<Type, ranged>& setting) {
    const QString name = QString::fromStdString(setting.GetLabel());
    const Type value = setting.GetValue();
    qt_config->setValue(name + QStringLiteral("/default"), value == setting.GetDefault());
    if constexpr (std::is_enum_v<Type>) {
        qt_config->setValue(name, static_cast<std::underlying_type_t<Type>>(value));
    } else {
        qt_config->setValue(name, QVariant::fromValue(value));
    }
}

template <typename Type, bool ranged>
void Config::WriteGlobalSetting(const Settings::SwitchableSetting<Type, ranged>& setting) {
    const QString name = QString::fromStdString(setting.GetLabel());
    const Type& value = setting.GetValue(global);
    if (!global) {
        qt_config->setValue(name + QStringLiteral("/use_global"), setting.UsingGlobal());
    }
    if (global || !setting.UsingGlobal()) {
        qt_config->setValue(name + QStringLiteral("/default"), value == setting.GetDefault());
        if constexpr (std::is_enum_v<Type>) {
            qt_config->setValue(name, static_cast<std::underlying_type_t<Type>>(value));
        } else {
            qt_config->setValue(name, QVariant::fromValue(value));
        }
    }
}

template <>
void Config::WriteGlobalSetting(const Settings::SwitchableSetting<std::string>& setting) {
    const QString name = QString::fromStdString(setting.GetLabel());
    const std::string& value = setting.GetValue(global);
    if (!global) {
        qt_config->setValue(name + QStringLiteral("/use_global"), setting.UsingGlobal());
    }
    if (global || !setting.UsingGlobal()) {
        qt_config->setValue(name + QStringLiteral("/default"), value == setting.GetDefault());
        qt_config->setValue(name, QString::fromStdString(value));
    }
}

// Explicit u16 definition: Qt would store it as QMetaType otherwise, which is not human-readable
template <>
void Config::WriteGlobalSetting(const Settings::SwitchableSetting<u16, true>& setting) {
    const QString name = QString::fromStdString(setting.GetLabel());
    const u16& value = setting.GetValue(global);
    if (!global) {
        qt_config->setValue(name + QStringLiteral("/use_global"), setting.UsingGlobal());
    }
    if (global || !setting.UsingGlobal()) {
        qt_config->setValue(name + QStringLiteral("/default"), value == setting.GetDefault());
        qt_config->setValue(name, static_cast<u32>(value));
    }
}

void Config::ReadValues() {
    if (global) {
        ReadControlValues();
        ReadCameraValues();
        ReadDataStorageValues();
        ReadMiscellaneousValues();
        ReadDebuggingValues();
        ReadWebServiceValues();
        ReadVideoDumpingValues();
    }

    ReadUIValues();
    ReadCoreValues();
    ReadRendererValues();
    ReadLayoutValues();
    ReadAudioValues();
    ReadSystemValues();
    ReadUtilityValues();
}

void Config::ReadAudioValues() {
    qt_config->beginGroup(QStringLiteral("Audio"));

    ReadGlobalSetting(Settings::values.audio_emulation);
    ReadGlobalSetting(Settings::values.enable_audio_stretching);
    ReadGlobalSetting(Settings::values.enable_realtime_audio);
    ReadGlobalSetting(Settings::values.volume);

    if (global) {
        ReadBasicSetting(Settings::values.output_type);
        ReadBasicSetting(Settings::values.output_device);
        ReadBasicSetting(Settings::values.input_type);
        ReadBasicSetting(Settings::values.input_device);
    }

    qt_config->endGroup();
}

void Config::ReadCameraValues() {
    using namespace Service::CAM;
    qt_config->beginGroup(QStringLiteral("Camera"));

    Settings::values.camera_name[OuterRightCamera] =
        ReadSetting(QStringLiteral("camera_outer_right_name"), QStringLiteral("blank"))
            .toString()
            .toStdString();
    Settings::values.camera_config[OuterRightCamera] =
        ReadSetting(QStringLiteral("camera_outer_right_config"), QString{})
            .toString()
            .toStdString();
    Settings::values.camera_flip[OuterRightCamera] =
        ReadSetting(QStringLiteral("camera_outer_right_flip"), 0).toInt();
    Settings::values.camera_name[InnerCamera] =
        ReadSetting(QStringLiteral("camera_inner_name"), QStringLiteral("blank"))
            .toString()
            .toStdString();
    Settings::values.camera_config[InnerCamera] =
        ReadSetting(QStringLiteral("camera_inner_config"), QString{}).toString().toStdString();
    Settings::values.camera_flip[InnerCamera] =
        ReadSetting(QStringLiteral("camera_inner_flip"), 0).toInt();
    Settings::values.camera_name[OuterLeftCamera] =
        ReadSetting(QStringLiteral("camera_outer_left_name"), QStringLiteral("blank"))
            .toString()
            .toStdString();
    Settings::values.camera_config[OuterLeftCamera] =
        ReadSetting(QStringLiteral("camera_outer_left_config"), QString{}).toString().toStdString();
    Settings::values.camera_flip[OuterLeftCamera] =
        ReadSetting(QStringLiteral("camera_outer_left_flip"), 0).toInt();

    qt_config->endGroup();
}

void Config::ReadControlValues() {
    qt_config->beginGroup(QStringLiteral("Controls"));

    ReadBasicSetting(Settings::values.use_artic_base_controller);

    int num_touch_from_button_maps =
        qt_config->beginReadArray(QStringLiteral("touch_from_button_maps"));

    if (num_touch_from_button_maps > 0) {
        const auto append_touch_from_button_map = [this] {
            Settings::TouchFromButtonMap map;
            map.name = ReadSetting(QStringLiteral("name"), QStringLiteral("default"))
                           .toString()
                           .toStdString();
            const int num_touch_maps = qt_config->beginReadArray(QStringLiteral("entries"));
            map.buttons.reserve(num_touch_maps);
            for (int i = 0; i < num_touch_maps; i++) {
                qt_config->setArrayIndex(i);
                std::string touch_mapping =
                    ReadSetting(QStringLiteral("bind")).toString().toStdString();
                map.buttons.emplace_back(std::move(touch_mapping));
            }
            qt_config->endArray(); // entries
            Settings::values.touch_from_button_maps.emplace_back(std::move(map));
        };

        for (int i = 0; i < num_touch_from_button_maps; ++i) {
            qt_config->setArrayIndex(i);
            append_touch_from_button_map();
        }
    } else {
        Settings::values.touch_from_button_maps.emplace_back(
            Settings::TouchFromButtonMap{"default", {}});
        num_touch_from_button_maps = 1;
    }
    qt_config->endArray();

    Settings::values.current_input_profile_index =
        ReadSetting(QStringLiteral("profile"), 0).toInt();

    const auto append_profile = [this, num_touch_from_button_maps] {
        Settings::InputProfile profile;
        profile.name =
            ReadSetting(QStringLiteral("name"), QStringLiteral("default")).toString().toStdString();
        for (int i = 0; i < Settings::NativeButton::NumButtons; ++i) {
            std::string default_param = InputCommon::GenerateKeyboardParam(default_buttons[i]);
            profile.buttons[i] = ReadSetting(QString::fromUtf8(Settings::NativeButton::mapping[i]),
                                             QString::fromStdString(default_param))
                                     .toString()
                                     .toStdString();
            if (profile.buttons[i].empty())
                profile.buttons[i] = default_param;
        }
        for (int i = 0; i < Settings::NativeAnalog::NumAnalogs; ++i) {
            std::string default_param = InputCommon::GenerateAnalogParamFromKeys(
                default_analogs[i][0], default_analogs[i][1], default_analogs[i][2],
                default_analogs[i][3], default_analogs[i][4], 0.5f);
            profile.analogs[i] =
                ReadSetting(QString::fromStdString(Settings::NativeAnalog::mapping[i]),
                            QString::fromStdString(default_param))
                    .toString()
                    .toStdString();
            if (profile.analogs[i].empty())
                profile.analogs[i] = default_param;
        }
        profile.motion_device =
            ReadSetting(QStringLiteral("motion_device"),
                        QStringLiteral(
                            "engine:motion_emu,update_period:100,sensitivity:0.01,tilt_clamp:90.0"))
                .toString()
                .toStdString();
        profile.touch_device =
            ReadSetting(QStringLiteral("touch_device"), QStringLiteral("engine:emu_window"))
                .toString()
                .toStdString();
        profile.use_touch_from_button =
            ReadSetting(QStringLiteral("use_touch_from_button"), false).toBool();
        profile.touch_from_button_map_index =
            ReadSetting(QStringLiteral("touch_from_button_map"), 0).toInt();
        profile.touch_from_button_map_index =
            std::clamp(profile.touch_from_button_map_index, 0, num_touch_from_button_maps - 1);
        profile.udp_input_address =
            ReadSetting(QStringLiteral("udp_input_address"),
                        QString::fromUtf8(InputCommon::CemuhookUDP::DEFAULT_ADDR))
                .toString()
                .toStdString();
        profile.udp_input_port = static_cast<u16>(
            ReadSetting(QStringLiteral("udp_input_port"), InputCommon::CemuhookUDP::DEFAULT_PORT)
                .toInt());
        profile.udp_pad_index =
            static_cast<u8>(ReadSetting(QStringLiteral("udp_pad_index"), 0).toUInt());
        Settings::values.input_profiles.emplace_back(std::move(profile));
    };

    int num_input_profiles = qt_config->beginReadArray(QStringLiteral("profiles"));

    for (int i = 0; i < num_input_profiles; ++i) {
        qt_config->setArrayIndex(i);
        append_profile();
    }

    qt_config->endArray();

    // create a input profile if no input profiles exist, with the default or old settings
    if (num_input_profiles == 0) {
        append_profile();
        num_input_profiles = 1;
    }

    // ensure that the current input profile index is valid.
    Settings::values.current_input_profile_index =
        std::clamp(Settings::values.current_input_profile_index, 0, num_input_profiles - 1);

    Settings::LoadProfile(Settings::values.current_input_profile_index);

    qt_config->endGroup();
}

void Config::ReadUtilityValues() {
    qt_config->beginGroup(QStringLiteral("Utility"));

    ReadGlobalSetting(Settings::values.dump_textures);
    ReadGlobalSetting(Settings::values.custom_textures);
    ReadGlobalSetting(Settings::values.preload_textures);
    ReadGlobalSetting(Settings::values.async_custom_loading);

    qt_config->endGroup();
}

void Config::ReadCoreValues() {
    qt_config->beginGroup(QStringLiteral("Core"));

    ReadGlobalSetting(Settings::values.cpu_clock_percentage);
    ReadGlobalSetting(Settings::values.enable_custom_cpu_ticks);
    ReadGlobalSetting(Settings::values.custom_cpu_ticks);
    ReadGlobalSetting(Settings::values.core_downcount_hack);
    ReadGlobalSetting(Settings::values.priority_boost);

    if (global) {
        ReadBasicSetting(Settings::values.use_cpu_jit);
        ReadBasicSetting(Settings::values.delay_start_for_lle_modules);
    }

    qt_config->endGroup();
}

void Config::ReadDataStorageValues() {
    qt_config->beginGroup(QStringLiteral("Data Storage"));

    ReadBasicSetting(Settings::values.use_virtual_sd);
    ReadBasicSetting(Settings::values.use_custom_storage);

    const std::string nand_dir =
        ReadSetting(QStringLiteral("nand_directory"), QStringLiteral("")).toString().toStdString();
    const std::string sdmc_dir =
        ReadSetting(QStringLiteral("sdmc_directory"), QStringLiteral("")).toString().toStdString();

    if (Settings::values.use_custom_storage) {
        FileUtil::UpdateUserPath(FileUtil::UserPath::NANDDir, nand_dir);
        FileUtil::UpdateUserPath(FileUtil::UserPath::SDMCDir, sdmc_dir);
    }

    qt_config->endGroup();
}

void Config::ReadDebuggingValues() {
    qt_config->beginGroup(QStringLiteral("Debugging"));

    ReadBasicSetting(Settings::values.record_frame_times);
    ReadBasicSetting(Settings::values.use_gdbstub);
    ReadBasicSetting(Settings::values.gdbstub_port);
    ReadBasicSetting(Settings::values.renderer_debug);
    ReadBasicSetting(Settings::values.dump_command_buffers);
    ReadBasicSetting(Settings::values.instant_debug_log);

    qt_config->beginGroup(QStringLiteral("LLE"));
    for (const auto& service_module : Service::service_module_map) {
        bool use_lle = ReadSetting(QString::fromStdString(service_module.name), false).toBool();
        Settings::values.lle_modules.emplace(service_module.name, use_lle);
    }
    qt_config->endGroup();
    qt_config->endGroup();
}

void Config::ReadLayoutValues() {
    qt_config->beginGroup(QStringLiteral("Layout"));

    ReadGlobalSetting(Settings::values.render_3d);
    ReadGlobalSetting(Settings::values.factor_3d);
    ReadGlobalSetting(Settings::values.swap_eyes_3d);
    ReadGlobalSetting(Settings::values.filter_mode);
    ReadGlobalSetting(Settings::values.pp_shader_name);
    ReadGlobalSetting(Settings::values.anaglyph_shader_name);
    ReadGlobalSetting(Settings::values.layout_option);
    ReadGlobalSetting(Settings::values.swap_screen);
    ReadGlobalSetting(Settings::values.upright_screen);
    ReadGlobalSetting(Settings::values.large_screen_proportion);
    ReadGlobalSetting(Settings::values.screen_gap);
    ReadGlobalSetting(Settings::values.small_screen_position);

    if (global) {
        ReadBasicSetting(Settings::values.mono_render_option);
        ReadBasicSetting(Settings::values.custom_top_x);
        ReadBasicSetting(Settings::values.custom_top_y);
        ReadBasicSetting(Settings::values.custom_top_width);
        ReadBasicSetting(Settings::values.custom_top_height);
        ReadBasicSetting(Settings::values.custom_bottom_x);
        ReadBasicSetting(Settings::values.custom_bottom_y);
        ReadBasicSetting(Settings::values.custom_bottom_width);
        ReadBasicSetting(Settings::values.custom_bottom_height);
        ReadBasicSetting(Settings::values.custom_second_layer_opacity);

        ReadBasicSetting(Settings::values.screen_top_stretch);
        ReadBasicSetting(Settings::values.screen_top_leftright_padding);
        ReadBasicSetting(Settings::values.screen_top_topbottom_padding);
        ReadBasicSetting(Settings::values.screen_bottom_stretch);
        ReadBasicSetting(Settings::values.screen_bottom_leftright_padding);
        ReadBasicSetting(Settings::values.screen_bottom_topbottom_padding);
        ReadBasicSetting(Settings::values.portrait_layout_option);
        ReadBasicSetting(Settings::values.custom_portrait_top_x);
        ReadBasicSetting(Settings::values.custom_portrait_top_y);
        ReadBasicSetting(Settings::values.custom_portrait_top_width);
        ReadBasicSetting(Settings::values.custom_portrait_top_height);
        ReadBasicSetting(Settings::values.custom_portrait_bottom_x);
        ReadBasicSetting(Settings::values.custom_portrait_bottom_y);
        ReadBasicSetting(Settings::values.custom_portrait_bottom_width);
        ReadBasicSetting(Settings::values.custom_portrait_bottom_height);
    }

    qt_config->endGroup();
}

void Config::ReadMiscellaneousValues() {
    qt_config->beginGroup(QStringLiteral("Miscellaneous"));

    ReadBasicSetting(Settings::values.log_filter);
    ReadBasicSetting(Settings::values.log_regex_filter);
    ReadBasicSetting(Settings::values.enable_gamemode);

    qt_config->endGroup();
}

void Config::ReadMultiplayerValues() {
    qt_config->beginGroup(QStringLiteral("Multiplayer"));

    UISettings::values.nickname = ReadSetting(QStringLiteral("nickname"), QString{}).toString();
    UISettings::values.ip = ReadSetting(QStringLiteral("ip"), QString{}).toString();
    UISettings::values.port =
        ReadSetting(QStringLiteral("port"), Network::DefaultRoomPort).toString();
    UISettings::values.room_nickname =
        ReadSetting(QStringLiteral("room_nickname"), QString{}).toString();
    UISettings::values.room_name = ReadSetting(QStringLiteral("room_name"), QString{}).toString();
    UISettings::values.room_port =
        ReadSetting(QStringLiteral("room_port"), QStringLiteral("24872")).toString();
    bool ok;
    UISettings::values.host_type = ReadSetting(QStringLiteral("host_type"), 0).toUInt(&ok);
    if (!ok) {
        UISettings::values.host_type = 0;
    }
    UISettings::values.max_player = ReadSetting(QStringLiteral("max_player"), 8).toUInt();
    UISettings::values.game_id = ReadSetting(QStringLiteral("game_id"), 0).toULongLong();
    UISettings::values.room_description =
        ReadSetting(QStringLiteral("room_description"), QString{}).toString();
    UISettings::values.multiplayer_filter_text =
        ReadSetting(QStringLiteral("multiplayer_filter_text"), QString{}).toString();
    UISettings::values.multiplayer_filter_games_owned =
        ReadSetting(QStringLiteral("multiplayer_filter_games_owned"), false).toBool();
    UISettings::values.multiplayer_filter_hide_empty =
        ReadSetting(QStringLiteral("multiplayer_filter_hide_empty"), false).toBool();
    UISettings::values.multiplayer_filter_hide_full =
        ReadSetting(QStringLiteral("multiplayer_filter_hide_full"), false).toBool();

    // Read ban list back
    int size = qt_config->beginReadArray(QStringLiteral("username_ban_list"));
    UISettings::values.ban_list.first.resize(size);
    for (int i = 0; i < size; ++i) {
        qt_config->setArrayIndex(i);
        UISettings::values.ban_list.first[i] =
            ReadSetting(QStringLiteral("username")).toString().toStdString();
    }
    qt_config->endArray();
    size = qt_config->beginReadArray(QStringLiteral("ip_ban_list"));
    UISettings::values.ban_list.second.resize(size);
    for (int i = 0; i < size; ++i) {
        qt_config->setArrayIndex(i);
        UISettings::values.ban_list.second[i] =
            ReadSetting(QStringLiteral("ip")).toString().toStdString();
    }
    qt_config->endArray();

    qt_config->endGroup();
}

void Config::ReadPathValues() {
    qt_config->beginGroup(QStringLiteral("Paths"));

    ReadGlobalSetting(UISettings::values.screenshot_path);

    if (global) {
        UISettings::values.roms_path = ReadSetting(QStringLiteral("romsPath")).toString();
        UISettings::values.symbols_path = ReadSetting(QStringLiteral("symbolsPath")).toString();
        UISettings::values.movie_record_path =
            ReadSetting(QStringLiteral("movieRecordPath")).toString();
        UISettings::values.movie_playback_path =
            ReadSetting(QStringLiteral("moviePlaybackPath")).toString();
        UISettings::values.video_dumping_path =
            ReadSetting(QStringLiteral("videoDumpingPath")).toString();
        UISettings::values.game_dir_deprecated =
            ReadSetting(QStringLiteral("gameListRootDir"), QStringLiteral(".")).toString();
        UISettings::values.game_dir_deprecated_deepscan =
            ReadSetting(QStringLiteral("gameListDeepScan"), false).toBool();
        int size = qt_config->beginReadArray(QStringLiteral("gamedirs"));
        for (int i = 0; i < size; ++i) {
            qt_config->setArrayIndex(i);
            UISettings::GameDir game_dir;
            game_dir.path = ReadSetting(QStringLiteral("path")).toString();
            game_dir.deep_scan = ReadSetting(QStringLiteral("deep_scan"), false).toBool();
            game_dir.expanded = ReadSetting(QStringLiteral("expanded"), true).toBool();
            UISettings::values.game_dirs.append(game_dir);
        }
        qt_config->endArray();
        // create NAND and SD card directories if empty, these are not removable through the UI,
        // also carries over old game list settings if present
        if (UISettings::values.game_dirs.isEmpty()) {
            UISettings::GameDir game_dir;
            game_dir.path = QStringLiteral("INSTALLED");
            game_dir.expanded = true;
            UISettings::values.game_dirs.append(game_dir);
            game_dir.path = QStringLiteral("SYSTEM");
            UISettings::values.game_dirs.append(game_dir);
            if (UISettings::values.game_dir_deprecated != QStringLiteral(".")) {
                game_dir.path = UISettings::values.game_dir_deprecated;
                game_dir.deep_scan = UISettings::values.game_dir_deprecated_deepscan;
                UISettings::values.game_dirs.append(game_dir);
            }
        }
        UISettings::values.last_artic_base_addr =
            ReadSetting(QStringLiteral("last_artic_base_addr"), QString{}).toString();
        UISettings::values.recent_files = ReadSetting(QStringLiteral("recentFiles")).toStringList();
        UISettings::values.language = ReadSetting(QStringLiteral("language"), QString{}).toString();
    }

    qt_config->endGroup();
}

void Config::ReadRendererValues() {
    qt_config->beginGroup(QStringLiteral("Renderer"));

    ReadGlobalSetting(Settings::values.graphics_api);
    ReadGlobalSetting(Settings::values.physical_device);
    ReadGlobalSetting(Settings::values.use_gles);
    ReadGlobalSetting(Settings::values.spirv_shader_gen);
    ReadGlobalSetting(Settings::values.geometry_shader);
    ReadGlobalSetting(Settings::values.use_sample_shading);
    ReadGlobalSetting(Settings::values.optimize_spirv_output);
    ReadGlobalSetting(Settings::values.spirv_output_validation);
    ReadGlobalSetting(Settings::values.spirv_output_legalization);
    ReadGlobalSetting(Settings::values.async_shader_compilation);
    ReadGlobalSetting(Settings::values.async_presentation);
    ReadGlobalSetting(Settings::values.skip_slow_draw);
    ReadGlobalSetting(Settings::values.skip_texture_copy);
    ReadGlobalSetting(Settings::values.skip_cpu_write);
    ReadGlobalSetting(Settings::values.upscaling_hack);
    ReadGlobalSetting(Settings::values.use_hw_shader);
    ReadGlobalSetting(Settings::values.shaders_accurate_mul);
    ReadGlobalSetting(Settings::values.use_disk_shader_cache);
    ReadGlobalSetting(Settings::values.use_vsync_new);
    ReadGlobalSetting(Settings::values.resolution_factor);
    ReadGlobalSetting(Settings::values.frame_limit);
    ReadGlobalSetting(Settings::values.frame_skip);

    ReadGlobalSetting(Settings::values.bg_red);
    ReadGlobalSetting(Settings::values.bg_green);
    ReadGlobalSetting(Settings::values.bg_blue);

    ReadGlobalSetting(Settings::values.texture_filter);
    ReadGlobalSetting(Settings::values.texture_sampling);

    ReadGlobalSetting(Settings::values.delay_game_render_thread_us);
    ReadGlobalSetting(Settings::values.disable_right_eye_render);

    if (global) {
        ReadBasicSetting(Settings::values.use_shader_jit);
    }

    qt_config->endGroup();
}

void Config::ReadShortcutValues() {
    qt_config->beginGroup(QStringLiteral("Shortcuts"));

    for (const auto& [name, group, shortcut] : default_hotkeys) {
        qt_config->beginGroup(group);
        qt_config->beginGroup(name);
        // No longer using ReadSetting for shortcut.second as it innacurately returns a value of 1
        // for WidgetWithChildrenShortcut which is a value of 3. Needed to fix shortcuts the open
        // a file dialog in windowed mode
        UISettings::values.shortcuts.push_back(
            {name,
             group,
             {ReadSetting(QStringLiteral("KeySeq"), shortcut.keyseq).toString(),
              shortcut.context}});
        qt_config->endGroup();
        qt_config->endGroup();
    }

    qt_config->endGroup();
}

void Config::ReadSystemValues() {
    qt_config->beginGroup(QStringLiteral("System"));

    ReadGlobalSetting(Settings::values.is_new_3ds);
    ReadGlobalSetting(Settings::values.lle_applets);
    ReadGlobalSetting(Settings::values.region_value);

    if (global) {
        ReadBasicSetting(Settings::values.init_clock);
        ReadBasicSetting(Settings::values.init_time);
        ReadBasicSetting(Settings::values.init_time_offset);
        ReadBasicSetting(Settings::values.init_ticks_type);
        ReadBasicSetting(Settings::values.init_ticks_override);
        ReadBasicSetting(Settings::values.steps_per_hour);
        ReadBasicSetting(Settings::values.plugin_loader_enabled);
        ReadBasicSetting(Settings::values.allow_plugin_loader);
    }

    qt_config->endGroup();
}

// Options for variable bit rate live streaming taken from here:
// https://developers.google.com/media/vp9/live-encoding
const QString DEFAULT_VIDEO_ENCODER_OPTIONS =
    QStringLiteral("quality:realtime,speed:6,tile-columns:4,frame-parallel:1,threads:8,row-mt:1");
const QString DEFAULT_AUDIO_ENCODER_OPTIONS = QStringLiteral("");

void Config::ReadVideoDumpingValues() {
    qt_config->beginGroup(QStringLiteral("VideoDumping"));

    Settings::values.output_format =
        ReadSetting(QStringLiteral("output_format"), QStringLiteral("webm"))
            .toString()
            .toStdString();
    Settings::values.format_options =
        ReadSetting(QStringLiteral("format_options")).toString().toStdString();

    Settings::values.video_encoder =
        ReadSetting(QStringLiteral("video_encoder"), QStringLiteral("libvpx-vp9"))
            .toString()
            .toStdString();

    Settings::values.video_encoder_options =
        ReadSetting(QStringLiteral("video_encoder_options"), DEFAULT_VIDEO_ENCODER_OPTIONS)
            .toString()
            .toStdString();

    Settings::values.video_bitrate =
        ReadSetting(QStringLiteral("video_bitrate"), 2500000).toULongLong();

    Settings::values.audio_encoder =
        ReadSetting(QStringLiteral("audio_encoder"), QStringLiteral("libvorbis"))
            .toString()
            .toStdString();
    Settings::values.audio_encoder_options =
        ReadSetting(QStringLiteral("audio_encoder_options"), DEFAULT_AUDIO_ENCODER_OPTIONS)
            .toString()
            .toStdString();
    Settings::values.audio_bitrate =
        ReadSetting(QStringLiteral("audio_bitrate"), 64000).toULongLong();

    qt_config->endGroup();
}

void Config::ReadUIValues() {
    qt_config->beginGroup(QStringLiteral("UI"));

    ReadPathValues();

    if (global) {
        UISettings::values.theme =
            ReadSetting(QStringLiteral("theme"), QString::fromUtf8(UISettings::themes[0].second))
                .toString();
        ReadBasicSetting(UISettings::values.enable_discord_presence);
        ReadBasicSetting(UISettings::values.screenshot_resolution_factor);

        ReadUpdaterValues();
        ReadUILayoutValues();
        ReadUIGameListValues();
        ReadShortcutValues();
        ReadMultiplayerValues();

        ReadBasicSetting(UISettings::values.single_window_mode);
        ReadBasicSetting(UISettings::values.fullscreen);
        ReadBasicSetting(UISettings::values.display_titlebar);
        ReadBasicSetting(UISettings::values.show_filter_bar);
        ReadBasicSetting(UISettings::values.show_status_bar);
        ReadBasicSetting(Settings::values.turbo_speed);
        ReadBasicSetting(UISettings::values.confirm_before_closing);
        ReadBasicSetting(UISettings::values.save_state_warning);
        ReadBasicSetting(UISettings::values.first_start);
        ReadBasicSetting(UISettings::values.callout_flags);
        ReadBasicSetting(UISettings::values.show_console);
        ReadBasicSetting(UISettings::values.pause_when_in_background);
        ReadBasicSetting(UISettings::values.mute_when_in_background);
        ReadBasicSetting(UISettings::values.hide_mouse);
    }

    qt_config->endGroup();
}

void Config::ReadUIGameListValues() {
    qt_config->beginGroup(QStringLiteral("GameList"));

    ReadBasicSetting(UISettings::values.game_list_icon_size);
    ReadBasicSetting(UISettings::values.game_list_row_1);
    ReadBasicSetting(UISettings::values.game_list_row_2);
    ReadBasicSetting(UISettings::values.game_list_hide_no_icon);
    ReadBasicSetting(UISettings::values.game_list_single_line_mode);

    ReadBasicSetting(UISettings::values.show_compat_column);
    ReadBasicSetting(UISettings::values.show_region_column);
    ReadBasicSetting(UISettings::values.show_type_column);
    ReadBasicSetting(UISettings::values.show_size_column);
    ReadBasicSetting(UISettings::values.show_play_time_column);

    const int favorites_size = qt_config->beginReadArray(QStringLiteral("favorites"));
    for (int i = 0; i < favorites_size; i++) {
        qt_config->setArrayIndex(i);
        UISettings::values.favorited_ids.append(
            ReadSetting(QStringLiteral("program_id")).toULongLong());
    }
    qt_config->endArray();

    qt_config->endGroup();
}

void Config::ReadUILayoutValues() {
    qt_config->beginGroup(QStringLiteral("UILayout"));

    UISettings::values.geometry = ReadSetting(QStringLiteral("geometry")).toByteArray();
    UISettings::values.state = ReadSetting(QStringLiteral("state")).toByteArray();
    UISettings::values.renderwindow_geometry =
        ReadSetting(QStringLiteral("geometryRenderWindow")).toByteArray();
    UISettings::values.gamelist_header_state =
        ReadSetting(QStringLiteral("gameListHeaderState")).toByteArray();

    qt_config->endGroup();
}

void Config::ReadUpdaterValues() {
    qt_config->beginGroup(QStringLiteral("Updater"));

    ReadBasicSetting(UISettings::values.check_for_update_on_start);
    ReadBasicSetting(UISettings::values.update_on_close);

    qt_config->endGroup();
}

void Config::ReadWebServiceValues() {
    qt_config->beginGroup(QStringLiteral("WebService"));

    NetSettings::values.web_api_url =
        ReadSetting(QStringLiteral("web_api_url"), QStringLiteral("https://api.borked3ds-emu.org"))
            .toString()
            .toStdString();
    NetSettings::values.borked3ds_username =
        ReadSetting(QStringLiteral("borked3ds_username")).toString().toStdString();
    NetSettings::values.borked3ds_token =
        ReadSetting(QStringLiteral("borked3ds_token")).toString().toStdString();

    qt_config->endGroup();
}

void Config::SaveValues() {
    if (global) {
        SaveControlValues();
        SaveCameraValues();
        SaveDataStorageValues();
        SaveMiscellaneousValues();
        SaveDebuggingValues();
        SaveWebServiceValues();
        SaveVideoDumpingValues();
    }

    SaveUIValues();
    SaveCoreValues();
    SaveRendererValues();
    SaveLayoutValues();
    SaveAudioValues();
    SaveSystemValues();
    SaveUtilityValues();
    qt_config->sync();
}

void Config::SaveAudioValues() {
    qt_config->beginGroup(QStringLiteral("Audio"));

    WriteGlobalSetting(Settings::values.audio_emulation);
    WriteGlobalSetting(Settings::values.enable_audio_stretching);
    WriteGlobalSetting(Settings::values.enable_realtime_audio);
    WriteGlobalSetting(Settings::values.volume);

    if (global) {
        WriteBasicSetting(Settings::values.output_type);
        WriteBasicSetting(Settings::values.output_device);
        WriteBasicSetting(Settings::values.input_type);
        WriteBasicSetting(Settings::values.input_device);
    }

    qt_config->endGroup();
}

void Config::SaveCameraValues() {
    using namespace Service::CAM;
    qt_config->beginGroup(QStringLiteral("Camera"));

    WriteSetting(QStringLiteral("camera_outer_right_name"),
                 QString::fromStdString(Settings::values.camera_name[OuterRightCamera]),
                 QStringLiteral("blank"));
    WriteSetting(QStringLiteral("camera_outer_right_config"),
                 QString::fromStdString(Settings::values.camera_config[OuterRightCamera]),
                 QString{});
    WriteSetting(QStringLiteral("camera_outer_right_flip"),
                 Settings::values.camera_flip[OuterRightCamera], 0);
    WriteSetting(QStringLiteral("camera_inner_name"),
                 QString::fromStdString(Settings::values.camera_name[InnerCamera]),
                 QStringLiteral("blank"));
    WriteSetting(QStringLiteral("camera_inner_config"),
                 QString::fromStdString(Settings::values.camera_config[InnerCamera]), QString{});
    WriteSetting(QStringLiteral("camera_inner_flip"), Settings::values.camera_flip[InnerCamera], 0);
    WriteSetting(QStringLiteral("camera_outer_left_name"),
                 QString::fromStdString(Settings::values.camera_name[OuterLeftCamera]),
                 QStringLiteral("blank"));
    WriteSetting(QStringLiteral("camera_outer_left_config"),
                 QString::fromStdString(Settings::values.camera_config[OuterLeftCamera]),
                 QString{});
    WriteSetting(QStringLiteral("camera_outer_left_flip"),
                 Settings::values.camera_flip[OuterLeftCamera], 0);

    qt_config->endGroup();
}

void Config::SaveControlValues() {
    qt_config->beginGroup(QStringLiteral("Controls"));

    WriteBasicSetting(Settings::values.use_artic_base_controller);

    WriteSetting(QStringLiteral("profile"), Settings::values.current_input_profile_index, 0);
    qt_config->beginWriteArray(QStringLiteral("profiles"));
    for (std::size_t p = 0; p < Settings::values.input_profiles.size(); ++p) {
        qt_config->setArrayIndex(static_cast<int>(p));
        const auto& profile = Settings::values.input_profiles[p];
        WriteSetting(QStringLiteral("name"), QString::fromStdString(profile.name),
                     QStringLiteral("default"));
        for (int i = 0; i < Settings::NativeButton::NumButtons; ++i) {
            std::string default_param = InputCommon::GenerateKeyboardParam(default_buttons[i]);
            WriteSetting(QString::fromStdString(Settings::NativeButton::mapping[i]),
                         QString::fromStdString(profile.buttons[i]),
                         QString::fromStdString(default_param));
        }
        for (int i = 0; i < Settings::NativeAnalog::NumAnalogs; ++i) {
            std::string default_param = InputCommon::GenerateAnalogParamFromKeys(
                default_analogs[i][0], default_analogs[i][1], default_analogs[i][2],
                default_analogs[i][3], default_analogs[i][4], 0.5f);
            WriteSetting(QString::fromStdString(Settings::NativeAnalog::mapping[i]),
                         QString::fromStdString(profile.analogs[i]),
                         QString::fromStdString(default_param));
        }
        WriteSetting(
            QStringLiteral("motion_device"), QString::fromStdString(profile.motion_device),
            QStringLiteral("engine:motion_emu,update_period:100,sensitivity:0.01,tilt_clamp:90.0"));
        WriteSetting(QStringLiteral("touch_device"), QString::fromStdString(profile.touch_device),
                     QStringLiteral("engine:emu_window"));
        WriteSetting(QStringLiteral("use_touch_from_button"), profile.use_touch_from_button, false);
        WriteSetting(QStringLiteral("touch_from_button_map"), profile.touch_from_button_map_index,
                     0);
        WriteSetting(QStringLiteral("udp_input_address"),
                     QString::fromStdString(profile.udp_input_address),
                     QString::fromUtf8(InputCommon::CemuhookUDP::DEFAULT_ADDR));
        WriteSetting(QStringLiteral("udp_input_port"), profile.udp_input_port,
                     InputCommon::CemuhookUDP::DEFAULT_PORT);
        WriteSetting(QStringLiteral("udp_pad_index"), profile.udp_pad_index, 0);
    }
    qt_config->endArray();

    qt_config->beginWriteArray(QStringLiteral("touch_from_button_maps"));
    for (std::size_t p = 0; p < Settings::values.touch_from_button_maps.size(); ++p) {
        qt_config->setArrayIndex(static_cast<int>(p));
        const auto& map = Settings::values.touch_from_button_maps[p];
        WriteSetting(QStringLiteral("name"), QString::fromStdString(map.name),
                     QStringLiteral("default"));
        qt_config->beginWriteArray(QStringLiteral("entries"));
        for (std::size_t q = 0; q < map.buttons.size(); ++q) {
            qt_config->setArrayIndex(static_cast<int>(q));
            WriteSetting(QStringLiteral("bind"), QString::fromStdString(map.buttons[q]));
        }
        qt_config->endArray();
    }
    qt_config->endArray();

    qt_config->endGroup();
}

void Config::SaveUtilityValues() {
    qt_config->beginGroup(QStringLiteral("Utility"));

    WriteGlobalSetting(Settings::values.dump_textures);
    WriteGlobalSetting(Settings::values.custom_textures);
    WriteGlobalSetting(Settings::values.preload_textures);
    WriteGlobalSetting(Settings::values.async_custom_loading);

    qt_config->endGroup();
}

void Config::SaveCoreValues() {
    qt_config->beginGroup(QStringLiteral("Core"));

    WriteGlobalSetting(Settings::values.cpu_clock_percentage);
    WriteGlobalSetting(Settings::values.enable_custom_cpu_ticks);
    WriteGlobalSetting(Settings::values.custom_cpu_ticks);
    WriteGlobalSetting(Settings::values.core_downcount_hack);
    WriteGlobalSetting(Settings::values.priority_boost);

    if (global) {
        WriteBasicSetting(Settings::values.use_cpu_jit);
        WriteBasicSetting(Settings::values.delay_start_for_lle_modules);
    }

    qt_config->endGroup();
}

void Config::SaveDataStorageValues() {
    qt_config->beginGroup(QStringLiteral("Data Storage"));

    WriteBasicSetting(Settings::values.use_virtual_sd);
    WriteBasicSetting(Settings::values.use_custom_storage);
    WriteSetting(QStringLiteral("nand_directory"),
                 QString::fromStdString(FileUtil::GetUserPath(FileUtil::UserPath::NANDDir)),
                 QStringLiteral(""));
    WriteSetting(QStringLiteral("sdmc_directory"),
                 QString::fromStdString(FileUtil::GetUserPath(FileUtil::UserPath::SDMCDir)),
                 QStringLiteral(""));

    qt_config->endGroup();
}

void Config::SaveDebuggingValues() {
    qt_config->beginGroup(QStringLiteral("Debugging"));

    WriteBasicSetting(Settings::values.record_frame_times);
    WriteBasicSetting(Settings::values.use_gdbstub);
    WriteBasicSetting(Settings::values.gdbstub_port);
    WriteBasicSetting(Settings::values.renderer_debug);
    WriteBasicSetting(Settings::values.instant_debug_log);

    qt_config->beginGroup(QStringLiteral("LLE"));
    for (const auto& service_module : Settings::values.lle_modules) {
        WriteSetting(QString::fromStdString(service_module.first), service_module.second, false);
    }
    qt_config->endGroup();

    qt_config->endGroup();
}

void Config::SaveLayoutValues() {
    qt_config->beginGroup(QStringLiteral("Layout"));

    WriteGlobalSetting(Settings::values.render_3d);
    WriteGlobalSetting(Settings::values.factor_3d);
    WriteGlobalSetting(Settings::values.swap_eyes_3d);
    WriteGlobalSetting(Settings::values.filter_mode);
    WriteGlobalSetting(Settings::values.pp_shader_name);
    WriteGlobalSetting(Settings::values.anaglyph_shader_name);
    WriteGlobalSetting(Settings::values.layout_option);
    WriteGlobalSetting(Settings::values.swap_screen);
    WriteGlobalSetting(Settings::values.upright_screen);
    WriteGlobalSetting(Settings::values.large_screen_proportion);
    WriteGlobalSetting(Settings::values.screen_gap);
    WriteGlobalSetting(Settings::values.small_screen_position);
    if (global) {
        WriteBasicSetting(Settings::values.mono_render_option);
        WriteBasicSetting(Settings::values.custom_top_x);
        WriteBasicSetting(Settings::values.custom_top_y);
        WriteBasicSetting(Settings::values.custom_top_width);
        WriteBasicSetting(Settings::values.custom_top_height);
        WriteBasicSetting(Settings::values.custom_bottom_x);
        WriteBasicSetting(Settings::values.custom_bottom_y);
        WriteBasicSetting(Settings::values.custom_bottom_width);
        WriteBasicSetting(Settings::values.custom_bottom_height);
        WriteBasicSetting(Settings::values.custom_second_layer_opacity);

        WriteBasicSetting(Settings::values.screen_top_stretch);
        WriteBasicSetting(Settings::values.screen_top_leftright_padding);
        WriteBasicSetting(Settings::values.screen_top_topbottom_padding);
        WriteBasicSetting(Settings::values.screen_bottom_stretch);
        WriteBasicSetting(Settings::values.screen_bottom_leftright_padding);
        WriteBasicSetting(Settings::values.screen_bottom_topbottom_padding);
        WriteBasicSetting(Settings::values.custom_portrait_top_x);
        WriteBasicSetting(Settings::values.custom_portrait_top_y);
        WriteBasicSetting(Settings::values.custom_portrait_top_width);
        WriteBasicSetting(Settings::values.custom_portrait_top_height);
        WriteBasicSetting(Settings::values.custom_portrait_bottom_x);
        WriteBasicSetting(Settings::values.custom_portrait_bottom_y);
        WriteBasicSetting(Settings::values.custom_portrait_bottom_width);
        WriteBasicSetting(Settings::values.custom_portrait_bottom_height);
    }

    qt_config->endGroup();
}

void Config::SaveMiscellaneousValues() {
    qt_config->beginGroup(QStringLiteral("Miscellaneous"));

    WriteBasicSetting(Settings::values.log_filter);
    WriteBasicSetting(Settings::values.log_regex_filter);
    WriteBasicSetting(Settings::values.enable_gamemode);

    qt_config->endGroup();
}

void Config::SaveMultiplayerValues() {
    qt_config->beginGroup(QStringLiteral("Multiplayer"));

    WriteSetting(QStringLiteral("nickname"), UISettings::values.nickname, QString{});
    WriteSetting(QStringLiteral("ip"), UISettings::values.ip, QString{});
    WriteSetting(QStringLiteral("port"), UISettings::values.port, Network::DefaultRoomPort);
    WriteSetting(QStringLiteral("room_nickname"), UISettings::values.room_nickname, QString{});
    WriteSetting(QStringLiteral("room_name"), UISettings::values.room_name, QString{});
    WriteSetting(QStringLiteral("room_port"), UISettings::values.room_port,
                 QStringLiteral("24872"));
    WriteSetting(QStringLiteral("host_type"), UISettings::values.host_type, 0);
    WriteSetting(QStringLiteral("max_player"), UISettings::values.max_player, 8);
    WriteSetting(QStringLiteral("game_id"), UISettings::values.game_id, 0);
    WriteSetting(QStringLiteral("room_description"), UISettings::values.room_description,
                 QString{});
    WriteSetting(QStringLiteral("multiplayer_filter_text"),
                 UISettings::values.multiplayer_filter_text, QString{});
    WriteSetting(QStringLiteral("multiplayer_filter_games_owned"),
                 UISettings::values.multiplayer_filter_games_owned, false);
    WriteSetting(QStringLiteral("multiplayer_filter_hide_empty"),
                 UISettings::values.multiplayer_filter_hide_empty, false);
    WriteSetting(QStringLiteral("multiplayer_filter_hide_full"),
                 UISettings::values.multiplayer_filter_hide_full, false);

    // Write ban list
    qt_config->beginWriteArray(QStringLiteral("username_ban_list"));
    for (std::size_t i = 0; i < UISettings::values.ban_list.first.size(); ++i) {
        qt_config->setArrayIndex(static_cast<int>(i));
        WriteSetting(QStringLiteral("username"),
                     QString::fromStdString(UISettings::values.ban_list.first[i]));
    }
    qt_config->endArray();
    qt_config->beginWriteArray(QStringLiteral("ip_ban_list"));
    for (std::size_t i = 0; i < UISettings::values.ban_list.second.size(); ++i) {
        qt_config->setArrayIndex(static_cast<int>(i));
        WriteSetting(QStringLiteral("ip"),
                     QString::fromStdString(UISettings::values.ban_list.second[i]));
    }
    qt_config->endArray();

    qt_config->endGroup();
}

void Config::SavePathValues() {
    qt_config->beginGroup(QStringLiteral("Paths"));

    WriteGlobalSetting(UISettings::values.screenshot_path);
    if (global) {
        WriteSetting(QStringLiteral("romsPath"), UISettings::values.roms_path);
        WriteSetting(QStringLiteral("symbolsPath"), UISettings::values.symbols_path);
        WriteSetting(QStringLiteral("movieRecordPath"), UISettings::values.movie_record_path);
        WriteSetting(QStringLiteral("moviePlaybackPath"), UISettings::values.movie_playback_path);
        WriteSetting(QStringLiteral("videoDumpingPath"), UISettings::values.video_dumping_path);
        qt_config->beginWriteArray(QStringLiteral("gamedirs"));
        for (int i = 0; i < UISettings::values.game_dirs.size(); ++i) {
            qt_config->setArrayIndex(i);
            const auto& game_dir = UISettings::values.game_dirs[i];
            WriteSetting(QStringLiteral("path"), game_dir.path);
            WriteSetting(QStringLiteral("deep_scan"), game_dir.deep_scan, false);
            WriteSetting(QStringLiteral("expanded"), game_dir.expanded, true);
        }
        qt_config->endArray();
        WriteSetting(QStringLiteral("last_artic_base_addr"),
                     UISettings::values.last_artic_base_addr, QString{});
        WriteSetting(QStringLiteral("recentFiles"), UISettings::values.recent_files);
        WriteSetting(QStringLiteral("language"), UISettings::values.language, QString{});
    }

    qt_config->endGroup();
}

void Config::SaveRendererValues() {
    qt_config->beginGroup(QStringLiteral("Renderer"));

    WriteGlobalSetting(Settings::values.graphics_api);
    WriteGlobalSetting(Settings::values.physical_device);
    WriteGlobalSetting(Settings::values.use_gles);
    WriteGlobalSetting(Settings::values.spirv_shader_gen);
    WriteGlobalSetting(Settings::values.geometry_shader);
    WriteGlobalSetting(Settings::values.use_sample_shading);
    WriteGlobalSetting(Settings::values.optimize_spirv_output);
    WriteGlobalSetting(Settings::values.spirv_output_validation);
    WriteGlobalSetting(Settings::values.spirv_output_legalization);
    WriteGlobalSetting(Settings::values.async_shader_compilation);
    WriteGlobalSetting(Settings::values.async_presentation);
    WriteGlobalSetting(Settings::values.skip_slow_draw);
    WriteGlobalSetting(Settings::values.skip_texture_copy);
    WriteGlobalSetting(Settings::values.skip_cpu_write);
    WriteGlobalSetting(Settings::values.upscaling_hack);
    WriteGlobalSetting(Settings::values.use_hw_shader);
    WriteGlobalSetting(Settings::values.shaders_accurate_mul);
    WriteGlobalSetting(Settings::values.use_disk_shader_cache);
    WriteGlobalSetting(Settings::values.use_vsync_new);
    WriteGlobalSetting(Settings::values.resolution_factor);
    WriteGlobalSetting(Settings::values.frame_limit);
    WriteGlobalSetting(Settings::values.frame_skip);

    WriteGlobalSetting(Settings::values.bg_red);
    WriteGlobalSetting(Settings::values.bg_green);
    WriteGlobalSetting(Settings::values.bg_blue);

    WriteGlobalSetting(Settings::values.texture_filter);
    WriteGlobalSetting(Settings::values.texture_sampling);

    WriteGlobalSetting(Settings::values.delay_game_render_thread_us);
    WriteGlobalSetting(Settings::values.disable_right_eye_render);

    if (global) {
        WriteSetting(QStringLiteral("use_shader_jit"), Settings::values.use_shader_jit.GetValue(),
                     true);
    }

    qt_config->endGroup();
}

void Config::SaveShortcutValues() {
    qt_config->beginGroup(QStringLiteral("Shortcuts"));

    // Lengths of UISettings::values.shortcuts & default_hotkeys are same.
    // However, their ordering must also be the same.
    for (std::size_t i = 0; i < default_hotkeys.size(); i++) {
        const auto& [name, group, shortcut] = UISettings::values.shortcuts[i];
        const auto& default_hotkey = default_hotkeys[i].shortcut;

        qt_config->beginGroup(group);
        qt_config->beginGroup(name);
        WriteSetting(QStringLiteral("KeySeq"), shortcut.keyseq, default_hotkey.keyseq);
        WriteSetting(QStringLiteral("Context"), shortcut.context, default_hotkey.context);
        qt_config->endGroup();
        qt_config->endGroup();
    }

    qt_config->endGroup();
}

void Config::SaveSystemValues() {
    qt_config->beginGroup(QStringLiteral("System"));

    WriteGlobalSetting(Settings::values.is_new_3ds);
    WriteGlobalSetting(Settings::values.lle_applets);
    WriteGlobalSetting(Settings::values.region_value);

    if (global) {
        WriteBasicSetting(Settings::values.init_clock);
        WriteBasicSetting(Settings::values.init_time);
        WriteBasicSetting(Settings::values.init_time_offset);
        WriteBasicSetting(Settings::values.init_ticks_type);
        WriteBasicSetting(Settings::values.init_ticks_override);
        WriteBasicSetting(Settings::values.steps_per_hour);
        WriteBasicSetting(Settings::values.plugin_loader_enabled);
        WriteBasicSetting(Settings::values.allow_plugin_loader);
    }

    qt_config->endGroup();
}

void Config::SaveVideoDumpingValues() {
    qt_config->beginGroup(QStringLiteral("VideoDumping"));

    WriteSetting(QStringLiteral("output_format"),
                 QString::fromStdString(Settings::values.output_format), QStringLiteral("webm"));
    WriteSetting(QStringLiteral("format_options"),
                 QString::fromStdString(Settings::values.format_options));
    WriteSetting(QStringLiteral("video_encoder"),
                 QString::fromStdString(Settings::values.video_encoder),
                 QStringLiteral("libvpx-vp9"));
    WriteSetting(QStringLiteral("video_encoder_options"),
                 QString::fromStdString(Settings::values.video_encoder_options),
                 DEFAULT_VIDEO_ENCODER_OPTIONS);
    WriteSetting(QStringLiteral("video_bitrate"),
                 static_cast<unsigned long long>(Settings::values.video_bitrate), 2500000);
    WriteSetting(QStringLiteral("audio_encoder"),
                 QString::fromStdString(Settings::values.audio_encoder),
                 QStringLiteral("libvorbis"));
    WriteSetting(QStringLiteral("audio_encoder_options"),
                 QString::fromStdString(Settings::values.audio_encoder_options),
                 DEFAULT_AUDIO_ENCODER_OPTIONS);
    WriteSetting(QStringLiteral("audio_bitrate"),
                 static_cast<unsigned long long>(Settings::values.audio_bitrate), 64000);

    qt_config->endGroup();
}

void Config::SaveUIValues() {
    qt_config->beginGroup(QStringLiteral("UI"));

    SavePathValues();

    if (global) {
        WriteSetting(QStringLiteral("theme"), UISettings::values.theme,
                     QString::fromUtf8(UISettings::themes[0].second));
        WriteBasicSetting(UISettings::values.enable_discord_presence);
        WriteBasicSetting(UISettings::values.screenshot_resolution_factor);

        SaveUpdaterValues();
        SaveUILayoutValues();
        SaveUIGameListValues();
        SaveShortcutValues();
        SaveMultiplayerValues();

        WriteBasicSetting(UISettings::values.single_window_mode);
        WriteBasicSetting(UISettings::values.fullscreen);
        WriteBasicSetting(UISettings::values.display_titlebar);
        WriteBasicSetting(UISettings::values.show_filter_bar);
        WriteBasicSetting(UISettings::values.show_status_bar);
        WriteBasicSetting(UISettings::values.confirm_before_closing);
        WriteBasicSetting(Settings::values.turbo_speed);
        WriteBasicSetting(UISettings::values.save_state_warning);
        WriteBasicSetting(UISettings::values.first_start);
        WriteBasicSetting(UISettings::values.callout_flags);
        WriteBasicSetting(UISettings::values.show_console);
        WriteBasicSetting(UISettings::values.pause_when_in_background);
        WriteBasicSetting(UISettings::values.mute_when_in_background);
        WriteBasicSetting(UISettings::values.hide_mouse);
    }

    qt_config->endGroup();
}

void Config::SaveUIGameListValues() {
    qt_config->beginGroup(QStringLiteral("GameList"));

    WriteBasicSetting(UISettings::values.game_list_icon_size);
    WriteBasicSetting(UISettings::values.game_list_row_1);
    WriteBasicSetting(UISettings::values.game_list_row_2);
    WriteBasicSetting(UISettings::values.game_list_hide_no_icon);
    WriteBasicSetting(UISettings::values.game_list_single_line_mode);

    WriteBasicSetting(UISettings::values.show_compat_column);
    WriteBasicSetting(UISettings::values.show_region_column);
    WriteBasicSetting(UISettings::values.show_type_column);
    WriteBasicSetting(UISettings::values.show_size_column);
    WriteBasicSetting(UISettings::values.show_play_time_column);

    qt_config->beginWriteArray(QStringLiteral("favorites"));
    for (int i = 0; i < UISettings::values.favorited_ids.size(); i++) {
        qt_config->setArrayIndex(i);
        WriteSetting(QStringLiteral("program_id"),
                     QVariant::fromValue(UISettings::values.favorited_ids[i]));
    }
    qt_config->endArray();

    qt_config->endGroup();
}

void Config::SaveUILayoutValues() {
    qt_config->beginGroup(QStringLiteral("UILayout"));

    WriteSetting(QStringLiteral("geometry"), UISettings::values.geometry);
    WriteSetting(QStringLiteral("state"), UISettings::values.state);
    WriteSetting(QStringLiteral("geometryRenderWindow"), UISettings::values.renderwindow_geometry);
    WriteSetting(QStringLiteral("gameListHeaderState"), UISettings::values.gamelist_header_state);

    qt_config->endGroup();
}

void Config::SaveUpdaterValues() {
    qt_config->beginGroup(QStringLiteral("Updater"));

    WriteBasicSetting(UISettings::values.check_for_update_on_start);
    WriteBasicSetting(UISettings::values.update_on_close);

    qt_config->endGroup();
}

void Config::SaveWebServiceValues() {
    qt_config->beginGroup(QStringLiteral("WebService"));

    WriteSetting(QStringLiteral("web_api_url"),
                 QString::fromStdString(NetSettings::values.web_api_url),
                 QStringLiteral("https://api.borked3ds-emu.org"));
    WriteSetting(QStringLiteral("borked3ds_username"),
                 QString::fromStdString(NetSettings::values.borked3ds_username));
    WriteSetting(QStringLiteral("borked3ds_token"),
                 QString::fromStdString(NetSettings::values.borked3ds_token));

    qt_config->endGroup();
}

QVariant Config::ReadSetting(const QString& name) const {
    return qt_config->value(name);
}

QVariant Config::ReadSetting(const QString& name, const QVariant& default_value) const {
    QVariant result;
    if (qt_config->value(name + QStringLiteral("/default"), false).toBool()) {
        result = default_value;
    } else {
        result = qt_config->value(name, default_value);
    }
    return result;
}

void Config::WriteSetting(const QString& name, const QVariant& value) {
    qt_config->setValue(name, value);
}

void Config::WriteSetting(const QString& name, const QVariant& value,
                          const QVariant& default_value) {
    qt_config->setValue(name + QStringLiteral("/default"), value == default_value);
    qt_config->setValue(name, value);
}

void Config::Reload() {
    ReadValues();
    // To apply default value changes
    SaveValues();
}

void Config::Save() {
    SaveValues();
}
