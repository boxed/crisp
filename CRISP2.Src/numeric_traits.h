#pragma once

template <typename _Tx>
struct numeric_traits
{
	typedef _Tx type;
	typedef type* pointer;

	typedef struct { } promote;
	typedef struct { } real_promote;
	typedef struct { } is_scalar;
	typedef struct { } is_signed;
	typedef struct { } is_integral;
	typedef struct { } is_ordered;
};

template <>
struct numeric_traits<BYTE>
{
	enum { is_integral = true };
	enum { is_scalar = true };
	enum { is_signed = false };
	enum { is_ordered = true };
	static LPCSTR FreeFormat() { return "%%u"; }
};

template <>
struct numeric_traits<CHAR>
{
	enum { is_integral = true };
	enum { is_scalar = true };
	enum { is_signed = true };
	enum { is_ordered = true };
	static LPCSTR FreeFormat() { return "%%d"; }
};

template <>
struct numeric_traits<WORD>
{
	enum { is_integral = true };
	enum { is_scalar = true };
	enum { is_signed = false };
	enum { is_ordered = true };
	static LPCSTR FreeFormat() { return "%%u"; }
};

template <>
struct numeric_traits<SHORT>
{
	static LPCSTR FreeFormat() { return "%%d"; }
	enum { is_integral = true };
	enum { is_scalar = true };
	enum { is_signed = true };
	enum { is_ordered = true };
};

template <>
struct numeric_traits<DWORD>
{
	enum { is_integral = true };
	enum { is_scalar = true };
	enum { is_signed = false };
	enum { is_ordered = true };
	static LPCSTR FreeFormat() { return "%%u"; }
};

template <>
struct numeric_traits<LONG>
{
	enum { is_integral = true };
	enum { is_scalar = true };
	enum { is_signed = true };
	enum { is_ordered = true };
	static LPCSTR FreeFormat() { return "%%d"; }
};

template <>
struct numeric_traits<float>
{
	enum { is_integral = false };
	enum { is_scalar = true };
	enum { is_signed = true };
	enum { is_ordered = true };
	static LPCSTR FreeFormat() { return "%%g"; }
};

template <>
struct numeric_traits<double>
{
	enum { is_integral = false };
	enum { is_scalar = true };
	enum { is_signed = true };
	enum { is_ordered = true };
	static LPCSTR FreeFormat() { return "%%g"; }
};

