// Copyright 2013 Dolphin Emulator Project / 2014 Citra Emulator Project
// Copyright 2024 Borked3DS Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

// Directory separators, do we need this?
#define DIR_SEP "/"
#define DIR_SEP_CHR '/'
#define DIR_SEP_CHR_WIN '\\'

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

// The user data dir
#define USERDATA_DIR "user"
#ifdef USER_DIR
#define EMU_DATA_DIR USER_DIR
#else
#ifdef _WIN32
#define EMU_DATA_DIR "Borked3DS"
#define LEGACY_EMU_DATA_DIR "Citra"
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define APPLE_EMU_DATA_DIR "Documents" DIR_SEP "Borked3DS"
#define LEGACY_APPLE_EMU_DATA_DIR "Documents" DIR_SEP "Citra"
#else
#define APPLE_EMU_DATA_DIR "Library" DIR_SEP "Application Support" DIR_SEP "Borked3DS"
#define LEGACY_APPLE_EMU_DATA_DIR "Library" DIR_SEP "Application Support" DIR_SEP "Citra"
#endif
// For compatibility with XDG paths.
#define EMU_DATA_DIR "borked3ds-emu"
#define LEGACY_EMU_DATA_DIR "citra-emu"
#else
#define EMU_DATA_DIR "borked3ds-emu"
#define LEGACY_EMU_DATA_DIR "citra-emu"
#endif
#endif

// Dirs in both User and Sys
#define EUR_DIR "EUR"
#define USA_DIR "USA"
#define JAP_DIR "JAP"

// Subdirs in the User dir returned by GetUserPath(UserPath::UserDir)
#define CONFIG_DIR "config"
#define CACHE_DIR "cache"
#define SDMC_DIR "sdmc"
#define NAND_DIR "nand"
#define SYSDATA_DIR "sysdata"
#define LOG_DIR "log"
#define CHEATS_DIR "cheats"
#define DLL_DIR "external_dlls"
#define SHADER_DIR "shaders"
#define DUMP_DIR "dump"
#define LOAD_DIR "load"
#define STATES_DIR "states"
#define ICONS_DIR "icons"

// Filenames
// Files in the directory returned by GetUserPath(UserPath::LogDir)
#define LOG_FILE "borked3ds_log.txt"

// Files in the directory returned by GetUserPath(UserPath::ConfigDir)
#define EMU_CONFIG "emu.ini"
#define DEBUGGER_CONFIG "debugger.ini"
#define LOGGER_CONFIG "logger.ini"

// Sys files
#define SHARED_FONT "shared_font.bin"
#define AES_KEYS "aes_keys.txt"
#define BOOTROM9 "boot9.bin"
#define SECRET_SECTOR "sector0x96.bin"
