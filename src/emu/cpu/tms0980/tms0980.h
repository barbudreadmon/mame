// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/*

  TMS0980/TMS1000-family MCU cores

*/
#ifndef _TMS0980_H_
#define _TMS0980_H_

#include "emu.h"
#include "machine/pla.h"


#define MCFG_TMS1XXX_OUTPUT_PLA(_pla) \
	tms1xxx_cpu_device::set_output_pla(*device, _pla);

#define MCFG_TMS1XXX_READ_K_CB(_devcb) \
	tms1xxx_cpu_device::set_read_k_callback(*device, DEVCB_##_devcb);

#define MCFG_TMS1XXX_WRITE_O_CB(_devcb) \
	tms1xxx_cpu_device::set_write_o_callback(*device, DEVCB_##_devcb);

#define MCFG_TMS1XXX_WRITE_R_CB(_devcb) \
	tms1xxx_cpu_device::set_write_r_callback(*device, DEVCB_##_devcb);

#define MCFG_TMS1XXX_POWER_OFF_CB(_devcb) \
	tms1xxx_cpu_device::set_power_off_callback(*device, DEVCB_##_devcb);


class tms1xxx_cpu_device : public cpu_device
{
public:
	// construction/destruction
	tms1xxx_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock
						, UINT8 o_pins, UINT8 r_pins, UINT8 k_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits
						, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source)
		: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
		, m_program_config("program", ENDIANNESS_BIG, byte_bits > 8 ? 16 : 8, prgwidth, 0, program)
		, m_data_config("data", ENDIANNESS_BIG, 8, datawidth, 0, data)
		, m_mpla(*this, "mpla")
		, m_ipla(*this, "ipla")
		, m_opla(*this, "opla")
		, m_spla(*this, "spla")
		, m_o_pins(o_pins)
		, m_r_pins(r_pins)
		, m_k_pins(k_pins)
		, m_pc_bits(pc_bits)
		, m_byte_bits(byte_bits)
		, m_x_bits(x_bits)
		, c_output_pla(NULL)
		, m_read_k(*this)
		, m_write_o(*this)
		, m_write_r(*this)
		, m_power_off(*this)
	{ }

	// static configuration helpers
	template<class _Object> static devcb_base &set_read_k_callback(device_t &device, _Object object) { return downcast<tms1xxx_cpu_device &>(device).m_read_k.set_callback(object); }
	template<class _Object> static devcb_base &set_write_o_callback(device_t &device, _Object object) { return downcast<tms1xxx_cpu_device &>(device).m_write_o.set_callback(object); }
	template<class _Object> static devcb_base &set_write_r_callback(device_t &device, _Object object) { return downcast<tms1xxx_cpu_device &>(device).m_write_r.set_callback(object); }
	template<class _Object> static devcb_base &set_power_off_callback(device_t &device, _Object object) { return downcast<tms1xxx_cpu_device &>(device).m_power_off.set_callback(object); }
	static void set_output_pla(device_t &device, const UINT16 *output_pla) { downcast<tms1xxx_cpu_device &>(device).c_output_pla = output_pla; }
	
	// driver debugging
	UINT8 debug_peek_o_index() { return m_o_index; }

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_execute_interface overrides
	virtual UINT32 execute_min_cycles() const { return 1; }
	virtual UINT32 execute_max_cycles() const { return 6; }
	virtual UINT32 execute_input_lines() const { return 1; }
	virtual void execute_run();

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const { return(spacenum == AS_PROGRAM) ? &m_program_config :((spacenum == AS_DATA) ? &m_data_config : NULL); }

	// device_disasm_interface overrides
	virtual UINT32 disasm_min_opcode_bytes() const { return 1; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 1; }

	void state_string_export(const device_state_entry &entry, astring &string);

	void next_pc();

	virtual void write_o_output(UINT8 index);
	virtual UINT8 read_k_input();
	virtual void set_cki_bus();
	virtual void dynamic_output() { ; } // not used by default
	virtual void read_opcode();

	virtual void op_sbit();
	virtual void op_rbit();
	virtual void op_setr();
	virtual void op_rstr();
	virtual void op_tdo();
	virtual void op_clo();
	virtual void op_ldx();
	virtual void op_comx();
	virtual void op_comx8();
	virtual void op_ldp();

	virtual void op_comc();
	virtual void op_xda();
	virtual void op_off();
	virtual void op_seac();
	virtual void op_reac();
	virtual void op_sal();
	virtual void op_sbl();

	address_space_config m_program_config;
	address_space_config m_data_config;

	optional_device<pla_device> m_mpla;
	optional_device<pla_device> m_ipla;
	optional_device<pla_device> m_opla;
	optional_device<pla_device> m_spla;

	UINT8   m_pc;        // 6 or 7-bit program counter
	UINT8   m_sr;        // 6 or 7-bit subroutine return register
	UINT8   m_pa;        // 4-bit page address register
	UINT8   m_pb;        // 4-bit page buffer register
	UINT8   m_a;         // 4-bit accumulator
	UINT8   m_x;         // 2,3,or 4-bit RAM X register
	UINT8   m_y;         // 4-bit RAM Y register
	UINT8   m_ca;        // chapter address bit
	UINT8   m_cb;        // chapter buffer bit
	UINT8   m_cs;        // chapter subroutine bit
	UINT16  m_r;
	UINT16  m_o;
	UINT8   m_o_index;
	UINT8   m_cki_bus;
	UINT8   m_c4;
	UINT8   m_p;         // 4-bit adder p(lus)-input
	UINT8   m_n;         // 4-bit adder n(egative)-input
	UINT8   m_adder_out; // adder result
	UINT8   m_carry_in;  // adder carry-in bit
	UINT8   m_carry_out; // adder carry-out bit
	UINT8   m_status;
	UINT8   m_status_latch;
	UINT8   m_eac;       // end around carry bit
	UINT8   m_clatch;    // call latch bit
	UINT8   m_add;       // add latch bit
	UINT8   m_bl;        // branch latch bit

	UINT8   m_ram_in;
	UINT8   m_dam_in;
	int     m_ram_out; // signed!
	UINT8   m_ram_address;
	UINT16  m_rom_address;
	UINT16  m_opcode;
	UINT32  m_fixed;
	UINT32  m_micro;
	int     m_subcycle;
	int     m_icount;

	UINT8   m_o_pins;    // how many O pins
	UINT8   m_r_pins;    // how many R pins
	UINT8   m_k_pins;    // how many K pins
	UINT8   m_pc_bits;   // how many program counter bits
	UINT8   m_byte_bits; // how many bits per 'byte'
	UINT8   m_x_bits;    // how many X register bits

	address_space *m_program;
	address_space *m_data;

	const UINT16 *c_output_pla;
	devcb_read8 m_read_k;
	devcb_write16 m_write_o;
	devcb_write16 m_write_r;
	devcb_write_line m_power_off;
	
	UINT32 m_o_mask;
	UINT32 m_r_mask;
	UINT32 m_k_mask;
	UINT32 m_pc_mask;
	UINT32 m_x_mask;

	// lookup tables
	dynamic_array<UINT32> m_fixed_decode;
	dynamic_array<UINT32> m_micro_decode;
	dynamic_array<UINT32> m_micro_direct;
};



class tms1000_cpu_device : public tms1xxx_cpu_device
{
public:
	tms1000_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms1000_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 k_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source);

protected:
	// overrides
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;


	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
};

class tms1070_cpu_device : public tms1000_cpu_device
{
public:
	tms1070_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tms1200_cpu_device : public tms1000_cpu_device
{
public:
	tms1200_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tms1100_cpu_device : public tms1000_cpu_device
{
public:
	tms1100_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms1100_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 k_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source);

protected:
	// overrides
	virtual void device_reset();

	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	
	virtual void op_setr();
	virtual void op_rstr();
};

class tms1300_cpu_device : public tms1100_cpu_device
{
public:
	tms1300_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


class tms0970_cpu_device : public tms1000_cpu_device
{
public:
	tms0970_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms0970_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 k_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source);

protected:
	// overrides
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void write_o_output(UINT8 index);
	
	virtual void op_setr();
	virtual void op_tdo();
};


class tms0980_cpu_device : public tms0970_cpu_device
{
public:
	tms0980_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	tms0980_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT8 o_pins, UINT8 r_pins, UINT8 k_pins, UINT8 pc_bits, UINT8 byte_bits, UINT8 x_bits, int prgwidth, address_map_constructor program, int datawidth, address_map_constructor data, const char *shortname, const char *source);

protected:
	// overrides
	virtual void device_reset();

	virtual machine_config_constructor device_mconfig_additions() const;

	virtual UINT32 disasm_min_opcode_bytes() const { return 2; }
	virtual UINT32 disasm_max_opcode_bytes() const { return 2; }
	virtual offs_t disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options);
	
	virtual void set_cki_bus();
	virtual void read_opcode();
	
	virtual void op_comx();

	UINT32 decode_micro(UINT8 sel);
};


class tmc0270_cpu_device : public tms0980_cpu_device
{
public:
	tmc0270_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// overrides
	virtual void device_start();
	virtual void device_reset();

	virtual machine_config_constructor device_mconfig_additions() const;

	virtual void write_o_output(UINT8 index) { tms1xxx_cpu_device::write_o_output(index); }
	virtual UINT8 read_k_input();
	virtual void dynamic_output();
	virtual void read_opcode();
	
	virtual void op_setr();
	virtual void op_rstr();
	virtual void op_tdo();

private:
	UINT8   m_a_prev;
	UINT16  m_r_prev;

	UINT8   m_o_latch_low;
	UINT8   m_o_latch;
	UINT8   m_o_latch_prev;
};



extern const device_type TMS1000;
extern const device_type TMS1070;
extern const device_type TMS1200;
extern const device_type TMS1100;
extern const device_type TMS1300;
extern const device_type TMS0970;
extern const device_type TMS0980;
extern const device_type TMC0270;


#endif /* _TMS0980_H_ */
