#pragma once

class C_EconItemView {
public:
};

class CEconItemSchema {
public:
};

class CEconItemDefinition {
public:
	__forceinline const char* GetItemBaseName( ) {
		return util::get_method< const char*( __thiscall* )( void* ) >( this, 2 )( this );
	}
};

typedef UINT LocalizeStringIndex_t;

class ILocalize {
public:
	__forceinline const wchar_t* Find( const char* token ) {
		return util::get_method< const wchar_t*( __thiscall* )( void*, const char* ) >( this, 12 )( this, token );
	}

	virtual LocalizeStringIndex_t	FindIndex(const char* tokenName) = 0;

	virtual void SetValueByIndex(LocalizeStringIndex_t index, wchar_t* newValue) = 0;
};

