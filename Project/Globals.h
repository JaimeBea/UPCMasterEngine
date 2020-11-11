#pragma once

enum class UpdateStatus
{
	CONTINUE,
	STOP,
	ERROR
};

// Constants -----------
#define DEGTORAD 0.01745329251f

// Configuration -----------
#define GLSL_VERSION "#version 460"

// Delete helpers
#define RELEASE( x ) { if( x != nullptr ) { delete x; x = nullptr; } }
#define RELEASE_ARRAY( x ) { if( x != nullptr ) { delete[] x; x = nullptr; } }

// Defer calls the given code at the end of the scope. Useful for freeing resources. Usage: DEFER{ statements; };
#ifndef DEFER
struct defer_dummy {};
template <class F> struct deferrer { F f; ~deferrer() { f(); } };
template <class F> deferrer<F> operator*(defer_dummy, F f) { return { f }; }
#define DEFER__(LINE) zz_defer##LINE
#define DEFER_(LINE) DEFER__(LINE)
#define DEFER auto DEFER_(__LINE__) = defer_dummy{} *[&]()
#endif