#include "includes.h"

bool Hooks::TempEntities( void *msg ) {
	if (!g_cl.m_processing) {
		return g_hooks.m_client_state.GetOldMethod< TempEntities_t >(CClientState::TEMPENTITIES)(this, msg);
	}

	const bool ret = g_hooks.m_client_state.GetOldMethod< TempEntities_t >(CClientState::TEMPENTITIES)(this, msg);

	for (auto ei = g_csgo.m_cl->m_events; ei; ei = ei->m_next) {
		const auto create_event = ei->m_client_class->m_pCreateEvent;

		if (!create_event || !create_event())
			continue;

		const auto class_id = ei->m_class_id - 1;

		ei->m_fire_delay = 0.0f;
	}

	return ret;
}

int Hooks::PacketStart(int incoming_sequence, int outgoing_acknowledged) {
	for (const int it : g_cl.m_outgoing_cmd_nums) {
		if (it == outgoing_acknowledged) {
			g_hooks.m_client_state.GetOldMethod< PacketStart_t >(CClientState::PACKETSTART)(this, incoming_sequence, outgoing_acknowledged);
			break;
		}
	}

	for (auto it = g_cl.m_outgoing_cmd_nums.begin(); it != g_cl.m_outgoing_cmd_nums.end(); ) {
		if (*it < outgoing_acknowledged)
			it = g_cl.m_outgoing_cmd_nums.erase(it);
		else
			it++;
	}

	return 0;
}
