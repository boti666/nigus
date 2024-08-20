#pragma once

class Config {
public:
	void init();
	void LoadHotkeys();
	void SaveHotkeys();
	void load(const Form* form, const std::string& name, bool skins = false);
	void save(const Form* form, const std::string& name, bool skins = false);
	void reset();
	std::string serialize();

private:
	bool m_init;
	std::string m_path;
};

extern Config g_config;