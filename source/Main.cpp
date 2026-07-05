#include <plugin.h>
#include <CPad.h>
#include <CPlayerInfo.h>
#include <CTimer.h>
#include <common.h>
#include <extensions/Config.h>
#include <extensions/ScriptCommands.h>
#include <algorithm>
#include <fstream>
#include <string>

using namespace plugin;

namespace {
constexpr float GTA_HEALTH_BASELINE = 100.0f;
}

struct Main {
    config_file m_config{ true, false };
    int m_keyboardKey = 'I';
    int m_gamepadButton = BUTTON_DPAD_DOWN;
    bool m_dynamicRadar = false;
    bool m_debug = true;
    unsigned int m_healthThreshold = 30;
    unsigned int m_lastLogTime = 0;
    float m_currentHealth = 0.0f;
    float m_maxHealth = 0.0f;
    float m_healthPercent = 0.0f;
    bool m_playerFound = false;
    bool m_radarHidden = false;
    bool m_showLowHealthHint = false;
    bool m_toggleKeyWasDown = false;
    std::string m_logPath;
    std::string m_warningText = "Low Health";

    Main() {
        LoadConfig();

        Events::gameProcessEvent += [this] {
            Process();
        };
    }

    void LoadConfig() {
        auto& keyboardKey = m_config["KeyboardKey"];
        auto& gamepadButton = m_config["GamepadButton"];
        auto& dynamicRadar = m_config["Dynamic"];
        auto& healthThreshold = m_config["HealthThreshold"];
        auto& debug = m_config["Debug"];
        auto& warningText = m_config["WarningText"];
        bool saveDefaults = false;

        if (keyboardKey.isEmpty()) {
            keyboardKey = m_keyboardKey;
            saveDefaults = true;
        }
        if (gamepadButton.isEmpty()) {
            gamepadButton = m_gamepadButton;
            saveDefaults = true;
        }
        if (dynamicRadar.isEmpty()) {
            dynamicRadar = m_dynamicRadar;
            saveDefaults = true;
        }
        if (healthThreshold.isEmpty()) {
            healthThreshold = static_cast<int>(m_healthThreshold);
            saveDefaults = true;
        }
        if (debug.isEmpty()) {
            debug = m_debug;
            saveDefaults = true;
        }
        if (warningText.isEmpty()) {
            warningText = m_warningText;
            saveDefaults = true;
        }

        m_keyboardKey = static_cast<int>(keyboardKey.asInt(m_keyboardKey));
        m_gamepadButton = static_cast<int>(gamepadButton.asInt(m_gamepadButton));
        m_dynamicRadar = dynamicRadar.asInt(m_dynamicRadar ? 1 : 0) != 0;
        m_healthThreshold = healthThreshold.asInt(m_healthThreshold);
        m_debug = debug.asInt(m_debug ? 1 : 0) != 0;
        m_warningText = warningText.asString(m_warningText);

        if (m_keyboardKey < 0 || m_keyboardKey > 255)
            m_keyboardKey = 0;
        if (m_gamepadButton < 0 || m_gamepadButton > BUTTON_STICK_RIGHT)
            m_gamepadButton = -1;
        if (m_healthThreshold > 100)
            m_healthThreshold = 100;
        if (m_warningText.empty())
            m_warningText = "Low Health";

        if (saveDefaults)
            m_config.save();

        m_logPath = m_config._path;
        const auto extension = m_logPath.find_last_of('.');
        if (extension != std::string::npos)
            m_logPath.replace(extension, std::string::npos, ".log");
    }

    bool IsPlayerLowOnHealth() {
        const auto* player = FindPlayerPed(0);
        m_playerFound = player != nullptr;
        m_currentHealth = 0.0f;
        m_maxHealth = 0.0f;
        m_healthPercent = 0.0f;

        if (!player)
            return false;

        m_currentHealth = player->m_fHealth;
        m_maxHealth = player->m_fMaxHealth;
        if (player->m_pPlayerInfo && player->m_pPlayerInfo->MaxHealth > 0)
            m_maxHealth = static_cast<float>(player->m_pPlayerInfo->MaxHealth);

        const float effectiveMaxHealth = m_maxHealth - GTA_HEALTH_BASELINE;
        if (effectiveMaxHealth <= 0.0f)
            return false;

        const float effectiveHealth =
            (std::max)(0.0f, m_currentHealth - GTA_HEALTH_BASELINE);
        m_healthPercent = effectiveHealth * 100.0f / effectiveMaxHealth;
        return m_healthPercent < static_cast<float>(m_healthThreshold);
    }

    void DrawLowHealthWarning() {
        if (!m_showLowHealthHint)
            return;

        Command<void, Commands::SET_TEXT_SCALE>(0.20f, 0.36f);
        Command<void, Commands::SET_TEXT_COLOUR>(150, 30, 30, 235);
        Command<void, Commands::SET_TEXT_FONT>(0);
        Command<void, Commands::SET_TEXT_PROPORTIONAL>(true);
        Command<void, Commands::SET_TEXT_EDGE>(2, 0, 0, 0, 255);
        Command<void, Commands::DISPLAY_TEXT_WITH_LITERAL_STRING>(
            0.015f,
            0.95f,
            "STRING",
            m_warningText.c_str());
    }

    void WriteDebugLog() {
        if (!m_debug || m_logPath.empty())
            return;

        const auto now = CTimer::m_snTimeInMilliseconds;
        if (now - m_lastLogTime < 1000)
            return;

        m_lastLogTime = now;
        std::ofstream log(m_logPath, std::ios::app);
        log << "time=" << now
            << " player=" << m_playerFound
            << " health=" << m_currentHealth
            << " maxHealth=" << m_maxHealth
            << " healthPercent=" << m_healthPercent
            << " dynamic=" << m_dynamicRadar
            << " threshold=" << m_healthThreshold
            << " low=" << m_showLowHealthHint
            << " radarHidden=" << m_radarHidden
            << '\n';
    }

    void Process() {
        const bool toggleKeyIsDown =
            m_keyboardKey != 0 && KeyPressed(m_keyboardKey);
        const bool gamepadButtonWasPressed =
            m_gamepadButton >= 0 &&
            CPad::IsButtonJustPressed(CPad::CurrentPad, m_gamepadButton);

        if ((toggleKeyIsDown && !m_toggleKeyWasDown) ||
            gamepadButtonWasPressed) {
            m_radarHidden = !m_radarHidden;
        }
        m_toggleKeyWasDown = toggleKeyIsDown;

        const bool playerLowOnHealth = IsPlayerLowOnHealth();
        m_showLowHealthHint =
            m_dynamicRadar && m_radarHidden && playerLowOnHealth;
        WriteDebugLog();
        DrawLowHealthWarning();

        // Missions and other scripts can enable the radar again, so enforce the
        // selected state each frame. Dynamic mode only draws the health hint.
        Command<void, Commands::DISPLAY_RADAR>(!m_radarHidden);
    }
} gInstance;
