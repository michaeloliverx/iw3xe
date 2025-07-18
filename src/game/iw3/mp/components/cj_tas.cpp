#include "cj_tas.h"
#include "common.h"
#include <algorithm>

#define ANGLE2SHORT(x) ((int)((x) * 65536 / 360) & 65535)
#define SHORT2ANGLE(x) ((x) * (360.0 / 65536))

namespace iw3
{
    namespace mp
    {
        struct RecordedCmd
        {
            int serverTime;
            int buttons;
            short angles[3];
            unsigned __int8 weapon;
            unsigned __int8 offHandIndex;
            char forwardmove;
            char rightmove;
        };
        size_t play_frame = 0;
        bool is_recording = false;
        bool is_playing = false;
        int playback_start_time = 0;
        int recording_start_time = 0;
        std::vector<RecordedCmd> current_recording;

        static cmd_function_s Cmd_Startrecord_VAR;
        static cmd_function_s Cmd_Stoprecord_VAR;
        static cmd_function_s Cmd_Togglerecord_VAR;
        static cmd_function_s Cmd_Startplayback_VAR;
        static cmd_function_s Cmd_Stopplayback_VAR;

        dvar_s *cj_tas_playback_ignore_weapon = nullptr;

        unsigned int rpg_mp_index = 0;

        void cj_tas::On_CG_Init()
        {
            // Weapon indexes change every game
            rpg_mp_index = BG_FindWeaponIndexForName("rpg_mp");
        }

        void Cmd_Startrecord_f()
        {
            if (is_recording)
            {
                CG_GameMessage(0, "^3Already recording");
                return;
            }

            is_recording = true;
            current_recording.clear();
            CG_GameMessage(0, "Recording ^2started");
        }

        void Cmd_Stoprecord_f()
        {
            if (!is_recording)
            {
                CG_GameMessage(0, "^1Not currently recording.");
                return;
            }

            is_recording = false;
            CG_GameMessage(0, "Recording ^1stopped");
        }

        void Cmd_Togglerecord_f()
        {
            if (is_recording)
            {
                Cmd_Stoprecord_f();
            }
            else
            {
                Cmd_Startrecord_f();
            }
        }

        void Cmd_Startplayback_f()
        {
            if (is_recording)
            {
                CG_GameMessage(0, "^1Stop recording before starting playback.\n");
                return;
            }

            if (is_playing)
            {
                CG_GameMessage(0, "^3Already playing.\n");
                return;
            }

            if (current_recording.empty())
            {
                CG_GameMessage(0, "^1No recording available to play.\n");
                return;
            }

            play_frame = 0;
            is_playing = true;
            playback_start_time = 0; // Will be set on first UpdateCommand
            recording_start_time = current_recording[0].serverTime;
            CG_GameMessage(0, "Playback ^2started\n");
        }

        void Cmd_Stopplayback_f()
        {
            if (!is_playing)
            {
                CG_GameMessage(0, "^1Not currently playing.\n");
                return;
            }

            play_frame = 0;
            is_playing = false;
            CG_GameMessage(0, "Playback ^1stopped\n");
        }

        bool IsPlayback()
        {
            return is_playing;
        }

        void CaptureCommand(usercmd_s *const cmd)
        {
            const auto cg = &(*cgArray)[0];
            const auto ps = &cg->predictedPlayerState;

            RecordedCmd recorded_cmd;
            recorded_cmd.serverTime = cmd->serverTime;
            recorded_cmd.buttons = cmd->buttons;
            recorded_cmd.angles[PITCH] = static_cast<short>(cmd->angles[PITCH]) + ANGLE2SHORT(ps->delta_angles[PITCH]);
            recorded_cmd.angles[YAW] = static_cast<short>(cmd->angles[YAW]) + ANGLE2SHORT(ps->delta_angles[YAW]);
            recorded_cmd.angles[ROLL] = static_cast<short>(cmd->angles[ROLL]) + ANGLE2SHORT(ps->delta_angles[ROLL]);
            recorded_cmd.weapon = cmd->weapon;
            recorded_cmd.offHandIndex = cmd->offHandIndex;
            recorded_cmd.forwardmove = cmd->forwardmove;
            recorded_cmd.rightmove = cmd->rightmove;

            current_recording.push_back(recorded_cmd);
        }

        void UpdateCommand(usercmd_s *const cmd)
        {
            if (current_recording.empty())
                return;

            if (play_frame >= current_recording.size())
            {
                Cmd_Stopplayback_f();
                return;
            }

            const auto cg = &(*cgArray)[0];
            const auto ps = &cg->predictedPlayerState;
            auto ca = &(*clients)[0];
            const auto &data = current_recording[play_frame];

            // Initialize playback start time on first frame
            if (playback_start_time == 0)
            {
                playback_start_time = cmd->serverTime;
            }

            // Calculate the relative time offset from the start of the recording
            int recording_time_offset = data.serverTime - recording_start_time;

            // Apply this offset to the current playback time
            cmd->serverTime = playback_start_time + recording_time_offset;

            const auto pitch = data.angles[PITCH] - ANGLE2SHORT(ps->delta_angles[PITCH]);
            const auto yaw = data.angles[YAW] - ANGLE2SHORT(ps->delta_angles[YAW]);
            const auto roll = data.angles[ROLL] - ANGLE2SHORT(ps->delta_angles[ROLL]);

            const int movementThreshold = 75;

            if (std::abs(cmd->forwardmove) >= movementThreshold ||
                std::abs(cmd->rightmove) >= movementThreshold)
            {
                Cmd_Stopplayback_f();
                // Set client viewangles to match the recorded angles
                ca->viewangles[PITCH] = static_cast<float>(SHORT2ANGLE(pitch));
                ca->viewangles[YAW] = static_cast<float>(SHORT2ANGLE(yaw));
                ca->viewangles[ROLL] = static_cast<float>(SHORT2ANGLE(roll));
                return;
            }

            cmd->buttons |= data.buttons;
            // Set the command angles to the recorded angles
            cmd->angles[PITCH] = pitch;
            cmd->angles[YAW] = yaw;
            cmd->angles[ROLL] = roll;

            // Set client viewangles to match the recorded angles
            ca->viewangles[PITCH] = static_cast<float>(SHORT2ANGLE(pitch));
            ca->viewangles[YAW] = static_cast<float>(SHORT2ANGLE(yaw));
            ca->viewangles[ROLL] = static_cast<float>(SHORT2ANGLE(roll));

            if (!cj_tas_playback_ignore_weapon->current.enabled)
            {
                cmd->weapon = data.weapon;
            }

            cmd->offHandIndex = data.offHandIndex;
            cmd->forwardmove = data.forwardmove;
            cmd->rightmove = data.rightmove;

            play_frame++;
        }

        dvar_s *cj_tas_bhop_auto = nullptr;

        dvar_s *cj_tas_jump_at_edge = nullptr;

        dvar_s *cj_tas_jump_on_rpg_fire = nullptr;

        dvar_s *cj_tas_crouch_on_jump = nullptr;

        dvar_s *cj_tas_rpg_lookdown = nullptr;
        dvar_s *cj_tas_rpg_lookdown_yaw = nullptr;
        dvar_s *cj_tas_rpg_lookdown_pitch = nullptr;

        bool cj_tas::TAS_Enabled()
        {
            const bool tas_enabled = (IsPlayback() ||
                                      cj_tas_bhop_auto->current.enabled ||
                                      cj_tas_jump_at_edge->current.enabled ||
                                      cj_tas_jump_on_rpg_fire->current.enabled ||
                                      cj_tas_crouch_on_jump->current.enabled ||
                                      cj_tas_rpg_lookdown->current.enabled);
            return tas_enabled;
        }

        pmove_t *clone_pmove(pmove_t *pmove)
        {
            pmove_t *copy = new pmove_t(*pmove);
            copy->ps = new playerState_s(*pmove->ps);
            return copy;
        }

        void delete_pmove(pmove_t *pmove)
        {
            if (pmove)
            {
                delete pmove->ps;
                delete pmove;
            }
        }

        pmove_t *predict_pmove(int localClientNum, int framesToAdvance = 1)
        {
            // On console this is 85!
            // vsync is 60 FPS, so we predict at 60 FPS
            // TODO: investigate if this is correct
            // static auto com_maxfps = Dvar_FindMalleableVar("com_maxfps");

            pmove_t *pmove_current = &cg_pmove[localClientNum];
            auto pmove_clone = clone_pmove(pmove_current);

            auto ca = &(*clients)[localClientNum];

            int frameTime = 1000 / 60; // assuming 60 FPS
            for (int i = 0; i < framesToAdvance; ++i)
            {
                pmove_clone->oldcmd = pmove_clone->cmd;
                pmove_clone->cmd.serverTime += frameTime;

                pmove_clone->cmd.angles[PITCH] = ANGLE2SHORT(ca->viewangles[PITCH]);
                pmove_clone->cmd.angles[YAW] = ANGLE2SHORT(ca->viewangles[YAW]);

                PmoveSingle(pmove_clone);
            }

            return pmove_clone;
        }

        void TAS_Cycle(int localClientNum)
        {
            if (!cj_tas::TAS_Enabled())
                return;

            pmove_t *pmove_current = &cg_pmove[localClientNum];
            auto pmove_predicted = predict_pmove(localClientNum, 2);

            auto ca = &(*clients)[localClientNum];
            auto cg = &(*cgArray)[localClientNum];
            auto cmd = &ca->cmds[ca->cmdNumber & 0x7F];

            static auto should_reset = false;
            static auto previous_pitch = ca->viewangles[PITCH];
            static auto previous_yaw = ca->viewangles[YAW];

            if (should_reset)
            {
                ca->viewangles[PITCH] = previous_pitch;
                ca->viewangles[YAW] = previous_yaw;
                should_reset = false;
            }

            const bool holding_rpg = pmove_current->ps->holdingWeapon(rpg_mp_index);
            const bool reloading = pmove_current->ps->isReloading();

            bool shot_rpg_next_frame = pmove_predicted->ps->weaponDelay <= 3 && pmove_predicted->ps->weaponDelay != 0;

            if (cj_tas_jump_on_rpg_fire->current.enabled && shot_rpg_next_frame && holding_rpg && !reloading)
            {
                cmd->buttons |= 1024; // JUMP
            }

            if (cj_tas_rpg_lookdown->current.enabled && shot_rpg_next_frame && holding_rpg && !reloading)
            {
                previous_pitch = ca->viewangles[PITCH];
                previous_yaw = ca->viewangles[YAW];
                should_reset = true;

                const auto pitch_offset = cj_tas_rpg_lookdown_pitch->current.integer;
                const auto yaw_offset = cj_tas_rpg_lookdown_yaw->current.integer;

                // Local game
                ca->viewangles[PITCH] = AngleNormalize360(pitch_offset - cg->snap->ps.delta_angles[PITCH]);
                ca->viewangles[YAW] = AngleNormalize360(yaw_offset - cg->snap->ps.delta_angles[YAW]);

                // cmd
                cmd->angles[PITCH] = ANGLE2SHORT(AngleNormalize360(pitch_offset - cg->snap->ps.delta_angles[PITCH]));
                cmd->angles[YAW] = ANGLE2SHORT(AngleNormalize360(yaw_offset - cg->snap->ps.delta_angles[YAW]));

                // // Invert movement direction ?
                // cmd->forwardmove = -cmd->forwardmove;
                // cmd->rightmove = -cmd->rightmove;
            }

            const bool is_on_ground_next_frame = pmove_predicted->ps->isOnGround();
            const bool will_leave_ground_this_frame = pmove_current->ps->isOnGround() && pmove_predicted->ps->isInAir();

            if (cj_tas_jump_at_edge->current.enabled && will_leave_ground_this_frame)
            {
                cmd->buttons |= 1024; // JUMP
            }

            if (cj_tas_bhop_auto->current.enabled && pmove_current->ps->isInAir() && is_on_ground_next_frame)
            {
                cmd->buttons &= ~1024; // Clear JUMP button
            }

            if (cj_tas_crouch_on_jump->current.enabled && cmd->buttons & 1024)
            {
                CL_SetStance(localClientNum, CL_STANCE_CROUCH);
            }

            delete_pmove(pmove_predicted);
        }

        Detour CL_CreateNewCommands_Detour;

        void CL_CreateNewCommands_Hook(int localClientNum)
        {
            CL_CreateNewCommands_Detour.GetOriginal<decltype(CL_CreateNewCommands)>()(localClientNum);
            if (clientUIActives[localClientNum].connectionState == CA_ACTIVE)
            {

                auto ca = &(*clients)[localClientNum];
                auto cmd = &ca->cmds[ca->cmdNumber & 0x7F];

                if (is_recording)
                {
                    CaptureCommand(cmd);
                }
                if (is_playing)
                {
                    UpdateCommand(cmd);
                }

                TAS_Cycle(localClientNum);
            }
        }

        cj_tas::cj_tas()
        {
            CL_CreateNewCommands_Detour = Detour(CL_CreateNewCommands, CL_CreateNewCommands_Hook);
            CL_CreateNewCommands_Detour.Install();

            Cmd_AddCommandInternal("startrecord", Cmd_Startrecord_f, &Cmd_Startrecord_VAR);
            Cmd_AddCommandInternal("stoprecord", Cmd_Stoprecord_f, &Cmd_Stoprecord_VAR);
            Cmd_AddCommandInternal("togglerecord", Cmd_Togglerecord_f, &Cmd_Togglerecord_VAR);
            Cmd_AddCommandInternal("startplayback", Cmd_Startplayback_f, &Cmd_Startplayback_VAR);
            Cmd_AddCommandInternal("stopplayback", Cmd_Stopplayback_f, &Cmd_Stopplayback_VAR);

            cj_tas_playback_ignore_weapon = Dvar_RegisterBool("cj_tas_playback_ignore_weapon", false, 0, "Ignore weapon in playback");

            cj_tas_bhop_auto = Dvar_RegisterBool("cj_tas_bhop_auto", false, 0, "Enable automatic bunny hopping");

            cj_tas_jump_at_edge = Dvar_RegisterBool("cj_tas_jump_at_edge", false, 0, "Enable jump at edge");

            cj_tas_jump_on_rpg_fire = Dvar_RegisterBool("cj_tas_jump_on_rpg_fire", false, 0, "Jump exactly when firing the RPG");

            cj_tas_crouch_on_jump = Dvar_RegisterBool("cj_tas_crouch_on_jump", false, 0, "Enable crouch on jump");

            cj_tas_rpg_lookdown = Dvar_RegisterBool("cj_tas_rpg_lookdown", false, 0, "Enable RPG lookdown");
            cj_tas_rpg_lookdown_yaw = Dvar_RegisterInt("cj_tas_rpg_lookdown_yaw", 0, -180, 180, 0, "RPG lookdown yaw angle");
            cj_tas_rpg_lookdown_pitch = Dvar_RegisterInt("cj_tas_rpg_lookdown_pitch", 70, -70, 70, 0, "RPG lookdown pitch angle");
        }

        cj_tas::~cj_tas()
        {
            CL_CreateNewCommands_Detour.Remove();
        }
    }
}
