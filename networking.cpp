#include "includes.h"

networking g_networking{};

float networking::get_latency() {
	return g_csgo.m_engine->GetNetChannelInfo()->GetLatency(INetChannel::FLOW_OUTGOING);
}

float networking::get_server_tick() {
	return g_csgo.m_cl->m_server_tick;
}