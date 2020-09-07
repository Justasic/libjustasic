#pragma once
#include "flags.h"
#include <map>
#include <string>

// Console Variables (CVars)
// based on id Tech 3's cvar system

enum CVAR_FLAGS
{
	CVAR_ARCHIVE  = 1,  // Will be saved to disk
	CVAR_INIT     = 2,  // Disallow editing via console but can be edited from cmdline
	CVAR_READONLY = 4,  // Only C code can set this value
	CVAR_ROM      = 8,  // Cannot be set by user
	CVAR_TEMP     = 16, // Can be modified but will not be saved to disk
};

class cvar_t : public Flags<CVAR_FLAGS>
{
	// Our values
	float f;
	int i;
	bool b;
	std::string s;

	// Whether or not we were modified
	bool modified;

	// A private list of cvars
	static std::map<std::string, cvar_t> cvars;

public:
	cvar_t(const std::string &name, const std::string &value);
	cvar_t(const std::string &name, int value);
	cvar_t(const std::string &name, float value);
	cvar_t(const std::string &name, bool value);

	// TODO: Get/Set
	// TODO: user Get/Set
	// TODO: cmdline parsing

	static cvar_t FindCVar(const std::string &name);

	// Cast operators for convenience
	inline operator bool() const { return this->b; }
	inline operator int() const { return this->i; }
	inline operator std::string() const { return this->s; }
	inline operator float() const { return this->f; }
	explicit inline operator const char*() const { return this->s.c_str(); }
};
