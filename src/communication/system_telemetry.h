#pragma once
#include <Arduino.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        SYS_FLAG__LORA_OK = (1 << 0), // 0x01
        SYS_FLAG__BT_OK = (1 << 1),   // 0x02
        SYS_FLAG__FAULT = (1 << 2),   // 0x04
        SYS_FLAG__RESERVED = (1 << 3) // 0x08
    } system_status_e;

    typedef enum
    {
        SCENARIO__NONE = 0,
        SCENARIO__MORNING,
        SCENARIO__NIGHT,
        SCENARIO__AWAY,
        SCENARIO__CUSTOM_START = 4
    } scenario_id_e;

    typedef enum
    {
        AUDIO__SILENCE = 0,
        AUDIO__ALARM,
        AUDIO__BEEP,
        AUDIO__VOICE_PROMPT
    } audio_id_e;

    typedef struct __attribute__((packed))
    {
        // בייט 1: דגלי מערכת (ביטים 0-7)
        uint8_t status_flags;

        // בייט 2: סוללות (בייטים מלאים)
        uint8_t remote_batt;
        uint8_t system_batt;

        // בייט 4: דגלי מצב (אור, קול, חום, סנריו פעיל)
        uint8_t light_on : 1;    // ביט 0
        uint8_t sound_on : 1;    // ביט 1
        uint8_t heat_on : 1;     // ביט 2
        uint8_t scn_active : 1;  // ביט 3
        uint8_t scenario_id : 4; // ביטים 4-7 (חצי בייט לסנריו)

        // בייט 5: אודיו (אפשר להוסיף עוד שדה כזה אם צריך)
        uint8_t audio_id : 4;
        uint8_t reserved : 4;

    } system_telemetry_t;

    // בדיקת בטיחות בזמן קומפילציה - מוודא שה-struct לא חורג מהגודל שרצינו
    static_assert(sizeof(system_telemetry_t) == 5, "Telemetry struct size mismatch!");