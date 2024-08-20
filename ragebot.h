#pragma once

class Ragebot
{
private:
	Player* m_target{ nullptr };
	AimPlayer* m_data{ nullptr };
	Weapon* m_weapon{ nullptr };
	WeaponInfo* m_weapon_info{ nullptr };
	std::vector<AimPlayer*> m_players{ nullptr };

public:
	void Main(Player* local);
};