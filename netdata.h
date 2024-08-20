#pragma once
#include <deque>
#include "Windows.h"
#define	EQUAL_EPSILON	0.001 

#define ADDVAR(type, prop, tol, name) m_vars.push_back(std::make_unique<type>(prop, tol, name))
#define ADDDATAMAPVAR(type, prop, tol, name) m_vars.push_back(std::make_unique<type>(prop, tol, name))
#define ADDVARCOORD(type, prop, tol, name, coord) m_vars.push_back(std::make_unique<type>(prop, tol, name, coord))

class datamap_t;
class Player;

static float get_new(float old, float new_, float tol, bool& error) {
	const float delta = new_ - old;
	if (fabsf(delta) <= tol) {
		error = true;
		return old;
	}
	return new_;
}

__forceinline bool CloseEnough(float a, float b, float epsilon = EQUAL_EPSILON) {
	return fabs(a - b) <= epsilon;
}

static float AssignRangeMultiplier(int nBits, double range) {
	unsigned long iHighValue;
	if (nBits == 32)
		iHighValue = 0xFFFFFFFE;
	else
		iHighValue = ((1 << (unsigned long)nBits) - 1);

	float fHighLowMul = iHighValue / range;
	if (CloseEnough(range, 0))
		fHighLowMul = iHighValue;

	// If the precision is messing us up, then adjust it so it won't.
	if ((unsigned long)(fHighLowMul * range) > iHighValue ||
		(fHighLowMul * range) > (double)iHighValue) {
		// Squeeze it down smaller and smaller until it's going to produce an integer
		// in the valid range when given the highest value.
		float multipliers[] = { 0.9999f, 0.99f, 0.9f, 0.8f, 0.7f };
		int i;
		for (i = 0; i < ARRAYSIZE(multipliers); i++) {
			fHighLowMul = (float)(iHighValue / range) * multipliers[i];
			if ((unsigned long)(fHighLowMul * range) > iHighValue ||
				(fHighLowMul * range) > (double)iHighValue) {
			}
			else {
				break;
			}
		}

		if (i == ARRAYSIZE(multipliers)) {
			// Doh! We seem to be unable to represent this range.
			return 0;
		}
	}

	return fHighLowMul;
}

class ManagedNetvar {
public:
	uintptr_t m_offset = 0;
	virtual void PreUpdate(Player* player) = 0;
	virtual void PostUpdate(Player* player) = 0;
};

class SharedNetvar : public ManagedNetvar {
public:
	float m_value;
	float m_old_value;
	float m_tolerance;
	char* m_name;
	SharedNetvar(uintptr_t offset, float tolerance, const char* name) : m_tolerance(tolerance) {
		m_offset = offset;
		m_name = _strdup(name);
	}

	void PreUpdate(Player* player) override;
	void PostUpdate(Player* player) override;
};

class ManagedVec : public ManagedNetvar {
public:
	vec3_t m_value;
	vec3_t m_old_value;
	float m_tolerance = 0;
	char* m_name;
	bool m_coord;
	ManagedVec(uintptr_t offset, float tolerance, const char* name, bool coord = false) : m_tolerance(tolerance), m_coord(coord) {
		m_offset = offset;
		m_name = _strdup(name);
	}

	void PreUpdate(Player* player) override;
	void PostUpdate(Player* player) override;
};

class PredictionNetvarManager {
public:
	~PredictionNetvarManager();
	bool m_initalized = false;
	bool m_setup_vars = false;
	std::vector<std::unique_ptr<ManagedNetvar>> m_vars;
	std::vector<ManagedNetvar*> m_weapon_vars;
	bool called_once = false;

	__forceinline void reset() {
		g_cl.m_map_setup = false;
		called_once = false;
	}

	void PreUpdate(Player*);
	void PostUpdate(Player*);
	void init(datamap_t* map);
};

extern PredictionNetvarManager g_predmanager;