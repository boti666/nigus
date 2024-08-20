#pragma once

class CSPlayerResource {

public:
	int GetPlayerPing(int idx);

	int GetPlayerAssists(int idx);

	int GetPlayerKills(int idx);

	int GetPlayerDeaths(int idx);

	vec3_t& m_bombsiteCenterA();

	vec3_t& m_bombsiteCenterB();
};