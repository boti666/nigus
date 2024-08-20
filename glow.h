#pragma once

#define END_OF_FREE_LIST	-1
#define ENTRY_IN_USE		-2
#define GLOW_FOR_ALL_SPLIT_SCREEN_SLOTS -1

#define STENCILCOMPARISONFUNCTION_NEVER			1 //Never passes.
#define STENCILCOMPARISONFUNCTION_LESS			2 //Passes where the reference value is less than the stencil value.
#define STENCILCOMPARISONFUNCTION_EQUAL			3 //Passes where the reference value is equal to the stencil value.
#define STENCILCOMPARISONFUNCTION_LESSEQUAL		4 //Passes where the reference value is less than or equal to the stencil value.
#define STENCILCOMPARISONFUNCTION_GREATER		5 //Passes where the reference value is greater than the stencil value.
#define STENCILCOMPARISONFUNCTION_NOTEQUAL		6 //Passes where the reference value is not equal to the stencil value.
#define STENCILCOMPARISONFUNCTION_GREATEREQUAL	7 //Passes where the reference value is greater than or equal to the stencil value.
#define STENCILCOMPARISONFUNCTION_ALWAYS		8 //Always passes.

#define STENCILOPERATION_KEEP	 1	//Preserves the existing stencil buffer value.
#define STENCILOPERATION_ZERO	 2	//Sets the value in the stencil buffer to 0.
#define STENCILOPERATION_REPLACE 3	//Sets the value in the stencil buffer to the reference value, set using render.SetStencilReferenceValue.
#define STENCILOPERATION_INCRSAT 4	//Increments the value in the stencil buffer by 1, clamping the result.
#define STENCILOPERATION_DECRSAT 5	//Decrements the value in the stencil buffer by 1, clamping the result.
#define STENCILOPERATION_INVERT	 6	//Inverts the value in the stencil buffer.
#define STENCILOPERATION_INCR	 7	//Increments the value in the stencil buffer by 1, wrapping around on overflow.
#define STENCILOPERATION_DECR	 8  //Decrements the value in the stencil buffer by 1, wrapping around on overflow.

enum GlowRenderStyle_t {
	GLOWRENDERSTYLE_DEFAULT = 0,
	GLOWRENDERSTYLE_RIMGLOW3D,
	GLOWRENDERSTYLE_EDGE_HIGHLIGHT,
	GLOWRENDERSTYLE_EDGE_HIGHLIGHT_PULSE,
	GLOWRENDERSTYLE_COUNT,
};

struct ShaderStencilState_t {
	bool m_bEnable;
	int m_FailOp;
	int m_ZFailOp;
	int m_PassOp;
	int m_CompareFunc;
	int m_nReferenceValue;
	unsigned int m_nTestMask;
	unsigned int m_nWriteMask;

	ShaderStencilState_t() {
		m_bEnable = false;
		m_PassOp = m_FailOp = m_ZFailOp = STENCILOPERATION_KEEP;
		m_CompareFunc = STENCILCOMPARISONFUNCTION_ALWAYS;
		m_nReferenceValue = 0;
		m_nTestMask = m_nWriteMask = 0xFFFFFFFF;
	}
};

struct GlowObjectDefinition_t {

	Entity* m_pEntity;
	vec3_t m_vGlowColor;
	float m_flGlowAlpha;

	bool m_bGlowAlphaCappedByRenderAlpha;
	float m_flGlowAlphaFunctionOfMaxVelocity;
	float m_flGlowAlphaMax;
	float m_flGlowPulseOverdrive;
	bool m_bRenderWhenOccluded;
	bool m_bRenderWhenUnoccluded;
	bool m_bFullBloomRender;
	int m_nFullBloomStencilTestValue; // only render full bloom objects if stencil is equal to this value (value of -1 implies no stencil test)
	int m_nRenderStyle;
	int m_nSplitScreenSlot;

	// Linked list of free slots
	int m_nNextFreeSlot;
};

class CGlowObjectManager {
public:
	CUtlVector< GlowObjectDefinition_t >	m_object_definitions;
	int										m_first_free_slot;
};