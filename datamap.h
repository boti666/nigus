#pragma once

// pre-declare.
class datamap_t;

// prototype.
using inputfunc_t = void(__cdecl*)(void* data);

enum fieldtype_t {
	FIELD_VOID = 0,			// No type or value
	FIELD_FLOAT,			// Any floating point value
	FIELD_STRING,			// A string ID (return from ALLOC_STRING)
	FIELD_VECTOR,			// Any vector, QAngle, or AngularImpulse
	FIELD_QUATERNION,		// A quaternion
	FIELD_INTEGER,			// Any integer or enum
	FIELD_BOOLEAN,			// boolean, implemented as an int, I may use this as a hint for compression
	FIELD_SHORT,			// 2 byte integer
	FIELD_CHARACTER,		// a byte
	FIELD_COLOR32,			// 8-bit per channel r,g,b,a (32bit color)
	FIELD_EMBEDDED,			// an embedded object with a datadesc, recursively traverse and embedded class/structure based on an additional typedescription
	FIELD_CUSTOM,			// special type that contains function pointers to it's read/write/parse functions
	FIELD_CLASSPTR,			// CBaseEntity *
	FIELD_EHANDLE,			// Entity handle
	FIELD_EDICT,			// edict_t *
	FIELD_POSITION_VECTOR,	// A world coordinate (these are fixed up across level transitions automagically)
	FIELD_TIME,				// a floating point time (these are fixed up automatically too!)
	FIELD_TICK,				// an integer tick count( fixed up similarly to time)
	FIELD_MODELNAME,		// Engine string that is a model name (needs precache)
	FIELD_SOUNDNAME,		// Engine string that is a sound name (needs precache)
	FIELD_INPUT,			// a list of inputed data fields (all derived from CMultiInputVar)
	FIELD_FUNCTION,			// A class function pointer (Think, Use, etc)
	FIELD_VMATRIX,			// a vmatrix (output coords are NOT worldspace)
	FIELD_VMATRIX_WORLDSPACE,// A VMatrix that maps some local space to world space (translation is fixed up on level transitions)
	FIELD_MATRIX3X4_WORLDSPACE,	// matrix3x4_t that maps some local space to world space (translation is fixed up on level transitions)
	FIELD_INTERVAL,			// a start and range floating point interval ( e.g., 3.2->3.6 == 3.2 and 0.4 )
	FIELD_MODELINDEX,		// a model index
	FIELD_MATERIALINDEX,	// a material index (using the material precache string table)
	FIELD_VECTOR2D,			// 2 floats
	FIELD_TYPECOUNT,		// MUST BE LAST
};

enum {
	TD_OFFSET_NORMAL = 0,
	TD_OFFSET_PACKED = 1,
	TD_OFFSET_COUNT,
};

class typedescription_t {
public:
	fieldtype_t				m_type;
	const char* m_name;
	int					m_offset;
	unsigned short			m_size;
	short					m_flags;
	const char* m_ext_name;
	void* m_save_restore_ops;
	inputfunc_t				m_input_func;
	datamap_t* m_td;
	int						m_bytes;
	typedescription_t* m_override_field;
	int						m_override_count;
	float					m_tolerance;
	int					m_flat_offset[TD_OFFSET_COUNT];
	unsigned short		m_flat_group;
	typedescription_t() { };
	typedescription_t(fieldtype_t type, const char* name,
		int						offset,
		unsigned short			size,
		short					flags,
		const char* ext_name,
		int						bytes,
		float					tolerance) : m_type(type), m_name(name), m_size(size), m_flags(flags),
		m_ext_name(ext_name), m_save_restore_ops(nullptr),
		m_input_func(nullptr),
		m_bytes(bytes),
		m_override_field(nullptr),
		m_override_count(0),
		m_tolerance(tolerance) {
		m_offset = (offset);
	}
};

struct optimized_datamap_t;
class datamap_t {
public:
	typedescription_t* m_desc;
	int					m_num_fields;
	char const* m_name;
	datamap_t* m_base;

	int					m_packed_size;
	optimized_datamap_t* m_optimized_map;
};

typedef void (*FN_FIELD_COMPARE)(const char* classname, const char* fieldname, const char* fieldtype,
	bool networked, bool noterrorchecked, bool differs, bool withintolerance, const char* value);

class CPredictionCopy {
public:
	typedef enum {
		DIFFERS = 0,
		IDENTICAL,
		WITHINTOLERANCE,
	} difftype_t;

	typedef enum {
		TRANSFERDATA_COPYONLY = 0,  // Data copying only (uses runs)
		TRANSFERDATA_ERRORCHECK_NOSPEW, // Checks for errors, returns after first error found
		TRANSFERDATA_ERRORCHECK_SPEW,   // checks for errors, reports all errors to console
		TRANSFERDATA_ERRORCHECK_DESCRIBE, // used by hud_pdump, dumps values, etc, for all fields
	} optype_t;

	CPredictionCopy(int type, byte* dest, bool dest_packed, const byte* src, bool src_packed,
		optype_t opType, FN_FIELD_COMPARE func = nullptr);

	int		TransferData(const char* operation, int entindex, datamap_t* dmap);

	static bool PrepareDataMap(datamap_t* dmap);

private:

	optype_t		m_OpType;
	int				m_nType;
	byte* m_pDest;
	const byte* m_pSrc;
	int				m_nDestOffsetIndex;
	int				m_nSrcOffsetIndex;
	int				m_nErrorCount;
	int				m_nEntIndex;

	FN_FIELD_COMPARE	m_FieldCompareFunc;

	const typedescription_t* m_pWatchField;
	char const* m_pOperation;

	const typedescription_t* m_FieldStack;
};