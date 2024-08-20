#include "includes.h"

void Ragebot::Main(Player* local)
{
	if (!local || !local->alive())
		return;

	if (!local->GetActiveWeapon() || !local->GetActiveWeapon()->GetWpnData())
		return;

	this->m_weapon = local->GetActiveWeapon();
	this->m_weapon_info = this->m_weapon->GetWpnData();
	
	// clear them
	this->m_players.clear();

	/* Add our targets from the entity based few things */
	for (int i = 1; i <= g_csgo.m_globals->m_max_clients; i++)
	{
		Entity* ent = g_csgo.m_entlist->GetClientEntity(i);
		if (!ent || !ent->IsPlayer())
			continue;

		Player* player = reinterpret_cast<Player*>(ent);
		if (!player || !player->alive() || player->dormant() || !player->enemy(local))
			continue;

		/* make sure we have atleast this... */
		AimPlayer* data = &g_aimbot.m_players[player->index() - 1];
		if (!data) continue;

		/* add the player */
		this->m_players.push_back(data);
	}

	// we have no possible targets to choose from
	if (this->m_players.empty())
		return;

	/* choose the best one */
	for (auto player : this->m_players)
	{
		AimPlayer* data = &g_aimbot.m_players[player->m_player->index() - 1];
		if (!data)
			continue;


	}
}
