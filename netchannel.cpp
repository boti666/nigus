#include "includes.h"

#pragma optimize( "", off )

int Hooks::SendDatagram(void* data) {

    if (!g_cl.m_processing || !g_csgo.m_engine->IsConnected() || !g_csgo.m_engine->IsInGame())
        return g_hooks.m_net_channel.GetOldMethod< SendDatagram_t >(INetChannel::SENDDATAGRAM)(this, data);

    const int backup1 = g_csgo.m_net->m_in_rel_state;
    const int backup2 = g_csgo.m_net->m_in_seq;

    INetChannel* net = g_csgo.m_engine->GetNetChannelInfo();

    if (g_menu.main.misc.ping_spike.get()) {
        float wish_ping = g_aimbot.latency_amount;
        wish_ping /= 1000.f;
        wish_ping -= g_cl.m_lerp;
        wish_ping = std::clamp(wish_ping, g_csgo.m_globals->m_interval, g_csgo.sv_maxunlag->GetFloat());

        const float latency = net->GetLatency(0);
        if (latency < wish_ping) {

            net->m_in_seq -= game::TIME_TO_TICKS(wish_ping - latency);

            for (auto& s : g_cl.m_sequences) {
                if (s.m_seq != net->m_in_seq)
                    continue;

                net->m_in_rel_state = s.m_state;
            }
        }
    }

    const int ret = g_hooks.m_net_channel.GetOldMethod< SendDatagram_t >(INetChannel::SENDDATAGRAM)(this, data);

    g_csgo.m_net->m_in_rel_state = backup1;
    g_csgo.m_net->m_in_seq = backup2;

    return ret;
}

void Hooks::ProcessPacket(void* packet, bool header) {
    g_hooks.m_net_channel.GetOldMethod< ProcessPacket_t >(INetChannel::PROCESSPACKET)(this, packet, header);

    g_cl.UpdateIncomingSequences();
}

bool Hooks::SendNetMsg(INetMessage& msg, bool reliable, bool voice) {
    INetChannel* net_channel = reinterpret_cast<INetChannel*> (this);

    if (net_channel != g_csgo.m_engine->GetNetChannelInfo())
        return g_hooks.m_net_channel.GetOldMethod< SendNetMsg_t >(INetChannel::SENDNETMSG)(this, msg, reliable, voice);

    if (msg.GetType() == INetChannel::TOTAL) // Return and don't send messsage if its FileCRCCheck
        return false;

    if (msg.GetGroup() == INetChannel::MOVE)
        g_inputpred.update();

    return g_hooks.m_net_channel.GetOldMethod< SendNetMsg_t >(INetChannel::SENDNETMSG)(this, msg, reliable, voice);
}
 
#pragma optimize( "", on )