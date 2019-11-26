#pragma once

#include <cstdint>

#define EVT_BEGIN(name) \
	const int32_t name[] = {
#define EVT_END() \
	0x1 };

#define EVT_HELPER_OP(op) \
	reinterpret_cast<int32_t>((op))

// Expression types
#define EVT_HELPER_EXPR(base, offset) \
	EVT_HELPER_OP((base) + (offset))

#define EVT_HELPER_LW_BASE -30000000
#define EVT_HELPER_GW_BASE -50000000
#define EVT_HELPER_LF_BASE -70000000
#define EVT_HELPER_GF_BASE -90000000
#define EVT_HELPER_LSWF_BASE -110000000
#define EVT_HELPER_GSWF_BASE -130000000
#define EVT_HELPER_LSW_BASE -150000000
#define EVT_HELPER_GSW_BASE -170000000
#define EVT_HELPER_UW_BASE -190000000
#define EVT_HELPER_UF_BASE -210000000
#define EVT_HELPER_FLOAT_BASE -230000000
#define EVT_HELPER_POINTER_BASE -250000000

#define LW(id) \
	EVT_HELPER_EXPR(EVT_HELPER_LW_BASE, id)
#define GW(id) \
	EVT_HELPER_EXPR(EVT_HELPER_GW_BASE, id)
#define LF(id) \
	EVT_HELPER_EXPR(EVT_HELPER_LF_BASE, id)
#define GF(id) \
	EVT_HELPER_EXPR(EVT_HELPER_GF_BASE, id)
#define LSWF(id) \
	EVT_HELPER_EXPR(EVT_HELPER_LSWF_BASE, id)
#define GSWF(id) \
	EVT_HELPER_EXPR(EVT_HELPER_GSWF_BASE, id)
#define LSW(id) \
	EVT_HELPER_EXPR(EVT_HELPER_LSW_BASE, id)
#define GSW(id) \
	EVT_HELPER_EXPR(EVT_HELPER_GSW_BASE, id)
#define UW(id) \
	EVT_HELPER_EXPR(EVT_HELPER_UW_BASE, id)
#define UF(id) \
	EVT_HELPER_EXPR(EVT_HELPER_UF_BASE, id)

#define FLOAT(value) \
	EVT_HELPER_EXPR( \
		EVT_HELPER_FLOAT_BASE, static_cast<int32_t>((value) * 1024.f) \
	)
#define PTR(value) \
	reinterpret_cast<int32_t>(value)

// Commands
#define EVT_HELPER_CMD(parameter_count, opcode) \
	static_cast<int32_t>( \
		static_cast<uint32_t>((parameter_count) << 16 | (opcode)) \
	)

#define RETURN() \
	EVT_HELPER_CMD(0, 2),

#define LBL(id) \
	EVT_HELPER_CMD(1, 3), EVT_HELPER_OP(id),
#define GOTO(id) \
	EVT_HELPER_CMD(1, 4), EVT_HELPER_OP(id),

#define DO(iteration_count) \
	EVT_HELPER_CMD(1, 5), EVT_HELPER_OP(iteration_count),
#define WHILE() \
	EVT_HELPER_CMD(0, 6),
#define DO_BREAK() \
	EVT_HELPER_CMD(0, 7),
#define DO_CONTINUE() \
	EVT_HELPER_CMD(0, 8),

#define WAIT_FRM() \
	EVT_HELPER_CMD(1, 9),
#define WAIT_MSEC() \
	EVT_HELPER_CMD(1, 10),
#define HALT(until) \
	EVT_HELPER_CMD(1, 11), EVT_HELPER_OP(until),

#define IF_STR_EQUAL(lhs, rhs) \
	EVT_HELPER_CMD(2, 12), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),
#define IF_STR_NOT_EQUAL(lhs, rhs) \
	EVT_HELPER_CMD(2, 13), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),
#define IF_STR_SMALL(lhs, rhs) \
	EVT_HELPER_CMD(2, 14), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),
#define IF_STR_LARGE(lhs, rhs) \
	EVT_HELPER_CMD(2, 15), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),
#define IF_STR_SMALL_EQUAL(lhs, rhs) \
	EVT_HELPER_CMD(2, 16), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),
#define IF_STR_LARGE_EQUAL(lhs, rhs) \
	EVT_HELPER_CMD(2, 17), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),

#define IFF_EQUAL(lhs, rhs) \
	EVT_HELPER_CMD(2, 18), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),
#define IFF_NOT_EQUAL(lhs, rhs) \
	EVT_HELPER_CMD(2, 19), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),
#define IFF_SMALL(lhs, rhs) \
	EVT_HELPER_CMD(2, 20), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),
#define IFF_LARGE(lhs, rhs) \
	EVT_HELPER_CMD(2, 21), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),
#define IFF_SMALL_EQUAL(lhs, rhs) \
	EVT_HELPER_CMD(2, 22), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),
#define IFF_LARGE_EQUAL(lhs, rhs) \
	EVT_HELPER_CMD(2, 23), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),

#define IF_EQUAL(lhs, rhs) \
	EVT_HELPER_CMD(2, 24), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),
#define IF_NOT_EQUAL(lhs, rhs) \
	EVT_HELPER_CMD(2, 25), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),
#define IF_SMALL(lhs, rhs) \
	EVT_HELPER_CMD(2, 26), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),
#define IF_LARGE(lhs, rhs) \
	EVT_HELPER_CMD(2, 27), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),
#define IF_SMALL_EQUAL(lhs, rhs) \
	EVT_HELPER_CMD(2, 28), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),
#define IF_LARGE_EQUAL(lhs, rhs) \
	EVT_HELPER_CMD(2, 29), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),

#define IF_FLAG(val, mask) \
	EVT_HELPER_CMD(2, 30), EVT_HELPER_OP(val), reinterpret_cast<int32_t>(mask),
#define IF_NOT_FLAG(val, mask) \
	EVT_HELPER_CMD(2, 31), EVT_HELPER_OP(val), reinterpret_cast<int32_t>(mask),

#define ELSE() \
	EVT_HELPER_CMD(0, 32),
#define END_IF() \
	EVT_HELPER_CMD(0, 33),

#define SWITCH(val) \
	EVT_HELPER_CMD(1, 34), EVT_HELPER_OP(val)
#define SWITCHI(val) \
	EVT_HELPER_CMD(1, 35), EVT_HELPER_OP(val)

#define CASE_EQUAL(val) \
	EVT_HELPER_CMD(1, 36), EVT_HELPER_OP(val),
#define CASE_NOT_EQUAL(val) \
	EVT_HELPER_CMD(1, 37), EVT_HELPER_OP(val),
#define CASE_SMALL(val) \
	EVT_HELPER_CMD(1, 38), EVT_HELPER_OP(val),
#define CASE_LARGE(val) \
	EVT_HELPER_CMD(1, 39), EVT_HELPER_OP(val),
#define CASE_SMALL_EQUAL(val) \
	EVT_HELPER_CMD(1, 40), EVT_HELPER_OP(val),
#define CASE_LARGE_EQUAL(val) \
	EVT_HELPER_CMD(1, 41), EVT_HELPER_OP(val),

#define CASE_ETC() \
	EVT_HELPER_CMD(0, 42),
#define CASE_OR(val) \
	EVT_HELPER_CMD(1, 43), EVT_HELPER_OP(val),
#define CASE_AND(val) \
	EVT_HELPER_CMD(1, 44), EVT_HELPER_OP(val),
#define CASE_FLAG(mask) \
	EVT_HELPER_CMD(1, 45), reinterpret_cast<int32_t>(mask),
#define CASE_END() \
	EVT_HELPER_CMD(0, 46),
#define CASE_BETWEEN(low, high) \
	EVT_HELPER_CMD(2, 47), EVT_HELPER_OP(low), EVT_HELPER_OP(high),

#define SWITCH_BREAK() \
	EVT_HELPER_CMD(0, 48),
#define END_SWITCH() \
	EVT_HELPER_CMD(0, 49),

#define SET(lhs, rhs) \
	EVT_HELPER_CMD(2, 50), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),
#define SETI(lhs, rhs) \
	EVT_HELPER_CMD(2, 51), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),
#define SETF(lhs, rhs) \
	EVT_HELPER_CMD(2, 52), EVT_HELPER_OP(lhs), EVT_HELPER_OP(rhs),

#define ADD(out, lhs, rhs) \
	EVT_HELPER_CMD(3, 53), EVT_HELPER_OP(out), EVT_HELPER_OP(lhs), \
	EVT_HELPER_OP(rhs),
#define SUB(out, lhs, rhs) \
	EVT_HELPER_CMD(3, 54), EVT_HELPER_OP(out), EVT_HELPER_OP(lhs), \
	EVT_HELPER_OP(rhs),
#define MUL(out, lhs, rhs) \
	EVT_HELPER_CMD(3, 55), EVT_HELPER_OP(out), EVT_HELPER_OP(lhs), \
	EVT_HELPER_OP(rhs),
#define DIV(out, lhs, rhs) \
	EVT_HELPER_CMD(3, 56), EVT_HELPER_OP(out), EVT_HELPER_OP(lhs), \
	EVT_HELPER_OP(rhs),
#define MOD(out, lhs, rhs) \
	EVT_HELPER_CMD(3, 57), EVT_HELPER_OP(out), EVT_HELPER_OP(lhs), \
	EVT_HELPER_OP(rhs),

#define ADDF(out, lhs, rhs) \
	EVT_HELPER_CMD(3, 58), EVT_HELPER_OP(out), EVT_HELPER_OP(lhs), \
	EVT_HELPER_OP(rhs),
#define SUBF(out, lhs, rhs) \
	EVT_HELPER_CMD(3, 59), EVT_HELPER_OP(out), EVT_HELPER_OP(lhs), \
	EVT_HELPER_OP(rhs),
#define MULF(out, lhs, rhs) \
	EVT_HELPER_CMD(3, 60), EVT_HELPER_OP(out), EVT_HELPER_OP(lhs), \
	EVT_HELPER_OP(rhs),
#define DIVF(out, lhs, rhs) \
	EVT_HELPER_CMD(3, 61), EVT_HELPER_OP(out), EVT_HELPER_OP(lhs), \
	EVT_HELPER_OP(rhs),

#define SET_READ(val) \
	EVT_HELPER_CMD(1, 62), EVT_HELPER_OP(val),
#define READ(out) \
	EVT_HELPER_CMD(1, 63), EVT_HELPER_OP(out),
#define READ2(out1, out2) \
	EVT_HELPER_CMD(2, 64), EVT_HELPER_OP(out1), EVT_HELPER_OP(out2),
#define READ3(out1, out2, out3) \
	EVT_HELPER_CMD(3, 65), EVT_HELPER_OP(out1), EVT_HELPER_OP(out2), \
	EVT_HELPER_OP(out3),
#define READ4(out1, out2, out3, out4) \
	EVT_HELPER_CMD(4, 66), EVT_HELPER_OP(out1), EVT_HELPER_OP(out2), \
	EVT_HELPER_OP(out3), EVT_HELPER_OP(out4),
#define READ_N(out, index) \
	EVT_HELPER_CMD(2, 67), EVT_HELPER_OP(out), EVT_HELPER_OP(index),

#define SET_READF(val) \
	EVT_HELPER_CMD(1, 68), EVT_HELPER_OP(val),
#define READF(out) \
	EVT_HELPER_CMD(1, 69), EVT_HELPER_OP(out),
#define READF2(out1, out2) \
	EVT_HELPER_CMD(2, 70), EVT_HELPER_OP(out1), EVT_HELPER_OP(out2),
#define READF3(out1, out2, out3) \
	EVT_HELPER_CMD(3, 71), EVT_HELPER_OP(out1), EVT_HELPER_OP(out2), \
	EVT_HELPER_OP(out3),
#define READF4(out1, out2, out3, out4) \
	EVT_HELPER_CMD(4, 72), EVT_HELPER_OP(out1), EVT_HELPER_OP(out2), \
	EVT_HELPER_OP(out3), EVT_HELPER_OP(out4),
#define READF_N(out, index) \
	EVT_HELPER_CMD(2, 73), EVT_HELPER_OP(out), EVT_HELPER_OP(index),

#define SET_USER_WRK(val) \
	EVT_HELPER_CMD(1, 74), EVT_HELPER_OP(val),
#define SET_USER_FLG(val) \
	EVT_HELPER_CMD(1, 75), EVT_HELPER_OP(val),
#define ALLOC_USER_WRK(count) \
	EVT_HELPER_CMD(1, 76), EVT_HELPER_OP(count),

#define AND(out, lhs, rhs) \
	EVT_HELPER_CMD(3, 77), EVT_HELPER_OP(out), EVT_HELPER_OP(lhs), \
	EVT_HELPER_OP(rhs),
#define ANDI(out, lhs, rhs) \
	EVT_HELPER_CMD(3, 78), EVT_HELPER_OP(out), EVT_HELPER_OP(lhs), \
	EVT_HELPER_OP(rhs),
#define OR(out, lhs, rhs) \
	EVT_HELPER_CMD(3, 79), EVT_HELPER_OP(out), EVT_HELPER_OP(lhs), \
	EVT_HELPER_OP(rhs),
#define ORI(out, lhs, rhs) \
	EVT_HELPER_CMD(3, 80), EVT_HELPER_OP(out), EVT_HELPER_OP(lhs), \
	EVT_HELPER_OP(rhs),

#define SET_FRAME_FROM_MSEC(out, in) \
	EVT_HELPER_CMD(2, 81), EVT_HELPER_OP(out), EVT_HELPER_OP(in),
#define SET_MSEC_FROM_FRAME(out, in) \
	EVT_HELPER_CMD(2, 82), EVT_HELPER_OP(out), EVT_HELPER_OP(in),
#define SET_RAM(val, ptr) \
	EVT_HELPER_CMD(2, 83), EVT_HELPER_OP(val), EVT_HELPER_OP(ptr),
#define SET_RAMF(val, ptr) \
	EVT_HELPER_CMD(2, 84), EVT_HELPER_OP(val), EVT_HELPER_OP(ptr),
#define GET_RAM(val, ptr) \
	EVT_HELPER_CMD(2, 85), EVT_HELPER_OP(out), EVT_HELPER_OP(ptr),
#define GET_RAMF(val, ptr) \
	EVT_HELPER_CMD(2, 86), EVT_HELPER_OP(out), EVT_HELPER_OP(ptr),

// R is short for Reg
#define SETR(indirect, val) \
	EVT_HELPER_CMD(2, 87), EVT_HELPER_OP(indirect), EVT_HELPER_OP(val),
#define SETRF(indirect, val) \
	EVT_HELPER_CMD(2, 88), EVT_HELPER_OP(indirect), EVT_HELPER_OP(val),
#define GETR(indirect, out) \
	EVT_HELPER_CMD(2, 89), EVT_HELPER_OP(indirect), EVT_HELPER_OP(out),
#define GETRF(indirect, out) \
	EVT_HELPER_CMD(2, 90), EVT_HELPER_OP(indirect), EVT_HELPER_OP(out),

// User function calls with validated parameter counts
template<bool expression>
class expression_assert
{
	static_assert(expression);
};
using evt_helper_int_array = int32_t[];
#define EVT_HELPER_NUM_ARGS(...) \
	(sizeof(evt_helper_int_array{ __VA_ARGS__ }) / sizeof(int32_t))
#define USER_FUNC(function, ...) \
	( \
		expression_assert< \
			function##_parameter_count == -1 \
			|| function##_parameter_count == EVT_HELPER_NUM_ARGS(__VA_ARGS__) \
		>(), \
		EVT_HELPER_CMD(1 + EVT_HELPER_NUM_ARGS(__VA_ARGS__), 91) \
	), \
	reinterpret_cast<int32_t>(function), \
	##__VA_ARGS__ ,

#define RUN_EVT(evt) \
	EVT_HELPER_CMD(1, 92), EVT_HELPER_OP(evt),
#define RUN_EVT_ID(evt, out_id) \
	EVT_HELPER_CMD(2, 93), EVT_HELPER_OP(evt), EVT_HELPER_OP(out_id),
#define RUN_CHILD_EVT(evt) \
	EVT_HELPER_CMD(1, 94), EVT_HELPER_OP(evt),
#define DELETE_EVT(evt_id) \
	EVT_HELPER_CMD(1, 95), EVT_HELPER_OP(evt_id),
#define RESTART_EVT(evt) \
	EVT_HELPER_CMD(1, 96), EVT_HELPER_OP(evt),

#define SET_PRI(pri) \
	EVT_HELPER_CMD(1, 97), EVT_HELPER_OP(pri),
#define SET_SPD(spd) \
	EVT_HELPER_CMD(1, 98), EVT_HELPER_OP(spd),
#define SET_TYPE(type_mask) \
	EVT_HELPER_CMD(1, 99), EVT_HELPER_OP(type_mask),

#define STOP_ALL(type_mask) \
	EVT_HELPER_CMD(1, 100), EVT_HELPER_OP(type_mask),
#define START_ALL(type_mask) \
	EVT_HELPER_CMD(1, 101), EVT_HELPER_OP(type_mask),
#define STOP_OTHER(type_mask) \
	EVT_HELPER_CMD(1, 102), EVT_HELPER_OP(type_mask),
#define START_OTHER(type_mask) \
	EVT_HELPER_CMD(1, 103), EVT_HELPER_OP(type_mask),
#define STOP_ID(evt_id) \
	EVT_HELPER_CMD(1, 104), EVT_HELPER_OP(evt_id),
#define START_ID(evt_id) \
	EVT_HELPER_CMD(1, 105), EVT_HELPER_OP(evt_id),
#define CHK_EVT(evt_id) \
	EVT_HELPER_CMD(1, 106), EVT_HELPER_OP(evt_id),

#define INLINE_EVT() \
	EVT_HELPER_CMD(0, 107),
#define INLINE_EVT_ID(out_id) \
	EVT_HELPER_CMD(1, 108), EVT_HELPER_OP(out_id),
#define END_INLINE() \
	EVT_HELPER_CMD(0, 109),

#define BROTHER_EVT() \
	EVT_HELPER_CMD(0, 110),
#define BROTHER_EVT_ID(out_id) \
	EVT_HELPER_CMD(1, 111), EVT_HELPER_OP(out_id),
#define END_BROTHER() \
	EVT_HELPER_CMD(0, 112),

#define DEBUG_PUT_MSG(msg) \
	EVT_HELPER_CMD(1, 113), EVT_HELPER_OP(msg)
#define DEBUG_MSG_CLEAR(msg) \
	EVT_HELPER_CMD(0, 114),
#define DEBUG_PUT_REG(reg) \
	EVT_HELPER_CMD(0, 115), EVT_HELPER_OP(reg),
#define DEBUG_NAME(name) \
	EVT_HELPER_CMD(1, 116), EVT_HELPER_OP(name),
#define DEBUG_REM(text) \
	EVT_HELPER_CMD(1, 117), EVT_HELPER_OP(text),
#define DEBUG_BP(text) \
	EVT_HELPER_CMD(0, 118),
