`timescale 1ps/1ps
module main();

    initial begin
        $dumpfile("cpu.vcd");
        $dumpvars(0,main);
    end

    // clock
    wire clk;
    clock c0(clk);

    reg halt = 0;

    counter ctr(halt,clk);

    /******
     * PC *
     ******/
    reg [15:0] pc = 16'h0000;

    /*******************
     * Shift Registers *
     *******************/
    reg  f_1 = 1;
    reg  f_2 = 0;
    reg  d_0 = 0;
    reg  e_1 = 0;
    reg  e_2 = 0;
    reg  wb_0 = 0;
    reg  is_flushing = 0;
    always @(posedge clk) begin
        // If stall don't move bits, if flush reset all
        f_1 <= 1;
        f_2 <= (is_flushing && wb_0) ? 0 : (is_stall) ? f_2 : f_1;
        d_0 <= (is_flushing && wb_0) ? 0 : (is_stall) ? d_0 : f_2;
        e_1 <= (is_flushing && wb_0) ? 0 : (is_stall && !is_mis_store) ? 0 : d_0;
        e_2 <= (is_flushing && wb_0) ? 0 : e_1;
        wb_0 <= (is_flushing && wb_0) ? 0 : e_2;
    end

    /***********
     * Fetch 1 *
     ***********/
    // Fetch the instruction pointed to by the current PC
    wire [15:0] mem_rdata0;
    wire [15:0] mem_rdata1;
    wire is_stall;

    /***********
     * Fetch 2 *
     ***********/
    reg [15:0] pc_fetch2;
    always @(posedge clk) begin
         pc_fetch2 <= (is_stall) ? pc_fetch2 : pc;
    end

    /**********
     * Decode *
     **********/
    // Splits up instruction from memory
    reg [15:0] instruction;
    wire [15:0] misaligned_instr;
    wire [15:0] true_instr;
    // Handles misaligned PC
    assign misaligned_instr[15:8] = mem_rdata0[7:0];
    assign misaligned_instr[7:0] = (is_jump_wb_3 && pc[0]) ? mem_rdata1[15:8] : instruction[15:8];
    assign true_instr = (was_stall || was_stall2) ? buffer_dec : (pc[0]) ? misaligned_instr : mem_rdata0;

    wire [3:0] opcode = true_instr[15:12];
    wire [3:0] ra = true_instr[11:8];
    wire [3:0] rb = true_instr[7:4];
    wire [3:0] rt = true_instr[3:0];

    // Values from registers
    wire [15:0] va;
    wire [3:0] read_addr_2 = (opcode == 4'b0000) ? rb : rt;
    wire [15:0] read_data_2;

    // Decode instruction
    wire  is_sub_calc = (opcode == 4'b0000);
    wire  is_movl_calc = (opcode == 4'b1000);
    wire  is_movh_calc = (opcode == 4'b1001);
    wire  is_jump_calc = (opcode == 4'b1110);
    wire  is_loadStore_calc = (opcode == 4'b1111);

    wire  is_zero_calc = (is_jump_calc && (rb == 4'b0000));
    wire  is_nZero_calc = (is_jump_calc && (rb == 4'b0001));
    wire  is_lZero_calc = (is_jump_calc && (rb == 4'b0010));
    wire  is_geZero_calc = (is_jump_calc && (rb == 4'b0011));

    wire  is_load_calc = (is_loadStore_calc && (rb == 4'b0000));
    wire  is_store_calc = (is_loadStore_calc && (rb == 4'b0001));

    wire  [7:0] imm_val_calc = true_instr[11:4];

    wire  w_mem_en_calc = is_store_calc;
    wire  w_reg_en_calc = (is_sub_calc || is_movl_calc || is_movh_calc || is_load_calc);

    // Check if need to halt
    wire  halt_calc = (!(is_sub_calc || is_movl_calc || is_movh_calc || is_zero_calc || 
                is_nZero_calc || is_lZero_calc || is_geZero_calc || is_load_calc || is_store_calc));

    reg  is_sub_dec;
    reg  is_movl_dec;
    reg  is_movh_dec;
    reg  is_jump_dec;
    reg  is_zero_dec;
    reg  is_nZero_dec;
    reg  is_lZero_dec;
    reg  is_geZero_dec;
    reg  is_load_dec;
    reg  is_store_dec;

    reg [7:0] imm_val_dec;
    reg [3:0] opcode_dec;
    reg [3:0] ra_dec;
    reg [3:0] rb_or_rt_dec;
    reg [3:0] rt_dec;

    reg  w_mem_en_dec;
    reg  w_reg_en_dec;

    reg  halt_dec;
    reg [15:0] pc_dec;
    reg [15:0] buffer_dec;

    always @(posedge clk) begin
        // Pass down information through pipeline
        buffer_dec <= (is_stall) ? true_instr : (was_stall) ? mem_rdata0 : buffer_dec;
        instruction <= (is_stall) ? instruction : mem_rdata0;
        is_sub_dec <= (is_stall) ? is_sub_dec : is_sub_calc;
        is_movl_dec <= (is_stall) ? is_movl_dec: is_movl_calc;
        is_movh_dec <= (is_stall) ? is_movh_dec : is_movh_calc;
        is_jump_dec <= (is_stall) ? is_jump_dec : is_jump_calc;
        is_zero_dec <= (is_stall) ? is_zero_dec : is_zero_calc;
        is_nZero_dec <= (is_stall) ? is_nZero_dec : is_nZero_calc;
        is_lZero_dec <= (is_stall) ? is_lZero_dec : is_lZero_calc;
        is_geZero_dec <= (is_stall) ? is_geZero_dec : is_geZero_calc;
        is_load_dec <= (is_stall) ? is_load_dec : is_load_calc;
        is_store_dec <= (is_stall) ? is_store_dec : is_store_calc;

        imm_val_dec <= (is_stall) ? imm_val_dec : imm_val_calc;
        opcode_dec <= (is_stall) ? opcode_dec : opcode;
        ra_dec <= (is_stall) ? ra_dec : ra;
        rb_or_rt_dec <= (is_stall) ? rb_or_rt_dec : read_addr_2;
        rt_dec <= (is_stall) ? rt_dec : rt;

        w_mem_en_dec <= (is_stall) ? w_mem_en_dec : is_store_calc;
        w_reg_en_dec <= (is_stall) ? w_reg_en_dec : w_reg_en_calc;

        pc_dec <= (is_stall) ? pc_dec : pc_fetch2;
        halt_dec <= (is_stall) ? halt_dec : halt_calc;
    end

    /*************
     * Execute 1 *
     *************/

    reg [3:0] rt_exec2;
    wire [15:0] res_wb;

    wire [15:0] vb_or_vt_calc = read_data_2;

    reg  is_sub_exec1;
    reg  is_movl_exec1;
    reg  is_movh_exec1;
    reg  is_jump_exec1;
    reg  is_zero_exec1;
    reg  is_nZero_exec1;
    reg  is_lZero_exec1;
    reg  is_geZero_exec1;
    reg  is_load_exec1;
    reg  is_store_exec1;

    reg [7:0] imm_val_exec1;
    reg [3:0] opcode_reg_exec1;
    reg [3:0] ra_exec1;
    reg [3:0] rb_or_rt_exec1;
    reg [3:0] rt_exec1;
    reg [15:0] va_exec1;
    reg [15:0] vb_or_vt_exec1;

    reg  w_mem_en_exec1;
    reg  w_reg_en_exec1;
    
    reg  halt_exec1;
    reg [15:0] pc_exec1;

    // Calculates forwarded value
    wire [15:0] forward_va_exec1 = (ra_dec == 0) ? 0 : (ra_dec == rt_exec1 && w_reg_en_exec1 && e_2 && !halt && !is_load_exec1) ? res_exec2 : (ra_dec == rt_exec2 && w_reg_en_exec2 && wb_0 && !halt) ? res_wb : va;
    // checks if misaligned load
    wire is_mis_load = (forward_va_exec1[0] && is_load_dec && e_1);
    reg is_mis_load_exec1;
    // checks if misaligned store
    wire is_mis_store = (is_store_dec && forward_va_exec1[0] && e_1 && !is_mis_store_exec1);
    reg is_mis_store_exec1;
    // is stalling logic
    assign is_stall = (d_0 && e_1 && is_load_calc && is_load_dec && ra == rt_dec) || is_mis_load || is_mis_store;
    reg was_stall;
    reg was_stall2;
    // Special forwarding case handling
    wire is_mis_store_then_mis_load_calc = (is_mis_store_exec2 && is_mis_load && wb_0 && va==va_exec2);
    reg is_mis_store_then_mis_load_exec1;
    reg [15:0] mis_store_then_mis_load_val_exec1;

    always @(posedge clk) begin
        is_mis_store_then_mis_load_exec1 <= is_mis_store_then_mis_load_calc;
        mis_store_then_mis_load_val_exec1 <= vb_or_vt_exec2;
        // Opcode Decode
        is_mis_store_exec1 <= is_mis_store;
        is_mis_load_exec1 <= is_mis_load;
        was_stall <= is_stall;
        was_stall2 <= was_stall;
        is_sub_exec1 <= is_sub_dec;
        is_movl_exec1 <= is_movl_dec;
        is_movh_exec1 <= is_movh_dec;
        is_jump_exec1 <= is_jump_dec;
        is_zero_exec1 <= is_zero_dec;
        is_nZero_exec1 <= is_nZero_dec;
        is_lZero_exec1 <= is_lZero_dec;
        is_geZero_exec1 <= is_geZero_dec;
        is_load_exec1 <= is_load_dec;
        is_store_exec1 <= is_store_dec;

        imm_val_exec1 <= imm_val_dec;
        opcode_reg_exec1 <= opcode_dec;
        ra_exec1 <= ra_dec;
        rb_or_rt_exec1 <= rb_or_rt_dec;
        rt_exec1 <= rt_dec;
        va_exec1 <= forward_va_exec1;
        // Forwarding for vb_ort_vt
        vb_or_vt_exec1 <= (rb_or_rt_dec == 0) ? 0 : (rb_or_rt_dec == rt_exec1 && w_reg_en_exec1 && e_2 && !halt && !is_load_exec1) ? res_exec2 : (rb_or_rt_dec == rt_exec2 && w_reg_en_exec2 && wb_0 && !halt) ? res_wb : vb_or_vt_calc;

        w_mem_en_exec1 <= w_mem_en_dec;
        w_reg_en_exec1 <= w_reg_en_dec;
        halt_exec1 <= halt_dec;
        pc_exec1 <= pc_dec;
    end

    /*************
     * Execute 2 *
     *************/

    // misalign store after store case
    wire is_mis_st_aft_st_bottom = (wb_0 && is_store_exec2 && !is_mis_store_exec2) && is_mis_store_exec1 && (va_wire_exec1-1==va_exec2);
    wire is_mis_st_aft_st_upper = (wb_0 && is_store_exec2 && !is_mis_store_exec2) && is_mis_store_exec1 && (va_wire_exec1+1==va_exec2);

    wire [15:0] va_wire_exec1 = (ra_exec1 == 0) ? 0 : (ra_exec1 == rt_exec2 && w_reg_en_exec2 && wb_0 && !halt) ? res_wb : va_exec1;
    wire [15:0] vb_or_vt_wire_exec1 = (rb_or_rt_exec1 == 0) ? 0 : (rb_or_rt_exec1 == rt_exec2 && w_reg_en_exec2 && wb_0 && !halt) ? res_wb : vb_or_vt_exec1;

    wire [15:0] sub_res = va_wire_exec1 - vb_or_vt_wire_exec1;
    wire [15:0] movl_res = { {8{imm_val_exec1[7]}}, imm_val_exec1};
    wire [15:0] movh_res = (vb_or_vt_wire_exec1 & 16'h00FF) | (imm_val_exec1 << 8);
    wire [15:0] res_exec2 = is_sub_exec1 ? sub_res : 
                            is_movl_exec1 ? movl_res : movh_res;
    
    reg  is_jump_exec2;
    reg  is_jumping;
    reg  is_zero_exec2;
    reg  is_nZero_exec2;
    reg  is_lZero_exec2;
    reg  is_geZero_exec2;
    reg  is_load_exec2;
    reg  is_store_exec2;

    reg [3:0] ra_exec2;
    reg [15:0] va_exec2;
    reg [15:0] vb_or_vt_exec2;

    reg  w_mem_en_exec2;
    reg  w_reg_en_exec2;
    reg  halt_exec2;
    reg [15:0] pc_exec2;
    
    reg [15:0] res_reg_exec2;
    reg  is_frwd_mem_exec2;
    reg  is_frwd_mem_mis1_exec2;
    reg  is_frwd_mem_mis2_exec2;
    reg  [15:0] mem_overload_exec2;
    reg is_mis_load_exec2;
    reg is_mis_store_exec2;
    reg is_mis_st_aft_st_bottom_exec2;
    reg is_mis_st_aft_st_upper_exec2;
    wire is_jumping_wire = (is_zero_exec1 && $signed(va_wire_exec1) == 0) ? is_jump_exec1 && 1 : 
                  (is_nZero_exec1 && $signed(va_wire_exec1) != 0) ? is_jump_exec1 && 1 : 
                  (is_lZero_exec1 && $signed(va_wire_exec1) < 0) ? is_jump_exec1 && 1 : 
                  (is_geZero_exec1 && $signed(va_wire_exec1) >= 0) ? is_jump_exec1 && 1 : 0;
    wire[15:0] correct_instr = (is_jumping_wire) ? vb_or_vt_wire_exec1 : pc_exec2 + 2;

    wire do_nothing = was_stall2 || (!pc_exec2[0] && pc_exec1 == correct_instr) || (pc_exec2[0] && !is_jumping_wire);
    // Calculates if flushing
    wire is_flushing_wire = !do_nothing || (!is_mis_store_exec2 && e_2 && ((is_store_exec1 && va_wire_exec1 < pc+2)));
    reg do_nothing_reg;
    reg[15:0] correct_instr_reg;


    always @(posedge clk) begin
        is_mis_st_aft_st_bottom_exec2 <= is_mis_st_aft_st_bottom;
        is_mis_st_aft_st_upper_exec2 <= is_mis_st_aft_st_upper;
        is_mis_load_exec2 <= (is_mis_store_exec2) ? 0 : is_mis_load_exec1;
        is_mis_store_exec2 <= (is_mis_store_exec2) ? 0 : is_mis_store_exec1;
        // If doing misaligned store everything should be a no op and converts it to what instruction should be
        is_jumping <= (is_mis_store_exec2) ? 0 : is_jumping_wire;
        is_jump_exec2 <= (is_mis_store_exec2) ? 0 : is_jump_exec1;
        is_zero_exec2 <= (is_mis_store_exec2) ? 0 : is_zero_exec1;
        is_nZero_exec2 <= (is_mis_store_exec2) ? 0 : is_nZero_exec1;
        is_lZero_exec2 <= (is_mis_store_exec2) ? 0 : is_lZero_exec1;
        is_geZero_exec2 <= (is_mis_store_exec2) ? 0 : is_geZero_exec1;
        is_load_exec2 <= (is_mis_store_exec2) ? 0 : is_load_exec1;
        is_store_exec2 <= (is_mis_store_exec2) ? 1 : is_store_exec1;
        ra_exec2 <= (is_mis_store_exec2) ? 16'h7777 : ra_exec1;
        rt_exec2 <= (is_mis_store_exec2) ? 0 : rt_exec1;
        w_mem_en_exec2 <= (is_mis_store_exec2) ? 1 : w_mem_en_exec1;
        w_reg_en_exec2 <= (is_mis_store_exec2) ? 0 : w_reg_en_exec1;
        halt_exec2 <= (is_mis_store_exec2) ? 0 : halt_exec1;
        res_reg_exec2 <= (is_mis_store_exec2) ? 0 : res_exec2;
        va_exec2 <= (is_mis_store_exec2) ? va_exec2+1 : va_wire_exec1;
        // Checks if forwarding memory
        vb_or_vt_exec2 <= (is_mis_store_exec2) ? upper_mis_st_val : vb_or_vt_wire_exec1;
        is_frwd_mem_exec2 <= (is_mis_store_exec2) ? 0 : (is_mis_store_then_mis_load_exec1 || va_wire_exec1 == va_exec2 && w_mem_en_exec2 && wb_0 && !halt) ? 1 : 0;
        is_frwd_mem_mis1_exec2 <= (is_mis_store_exec2) ? 0 : (va_wire_exec1 == va_exec2-1 && w_mem_en_exec2 && wb_0 && !halt) ? 1 : 0;
        is_frwd_mem_mis2_exec2 <= (is_mis_store_exec2) ? 0 : (va_wire_exec1 == va_exec2+1 && w_mem_en_exec2 && wb_0 && !halt) ? 1 : 0;
        mem_overload_exec2 <= (is_mis_store_exec2) ? 0 : (is_mis_store_then_mis_load_exec1) ? mis_store_then_mis_load_val_exec1 : ((va_wire_exec1 == va_exec2 || (va_wire_exec1 == va_exec2-1 && is_mis_load_exec1) || (va_wire_exec1 == va_exec2+1 && is_mis_load_exec1)) && w_mem_en_exec2 && wb_0 && !halt) ? vb_or_vt_exec2 : 0;
        pc_exec2 <= (is_mis_store_exec2) ? pc_exec2 : pc_exec1;
        is_flushing <= is_flushing_wire;
        do_nothing_reg <= do_nothing;
        correct_instr_reg <= correct_instr;
    end
                           

    /**************
     * Write Back *
     **************/

    wire [15:0] bottom_mis_st_val;
    wire [15:0] upper_mis_st_val;
    assign bottom_mis_st_val[15:8] = vb_or_vt_exec2[7:0];
    assign bottom_mis_st_val[7:0] = (is_mis_st_aft_st_bottom_exec2) ? delay_vb_or_vt[7:0] : mem_rdata1[7:0];
    assign upper_mis_st_val[15:8] = (is_mis_st_aft_st_upper_exec2) ? delay_vb_or_vt[15:8] : mem_rdata0[15:8];
    assign upper_mis_st_val[7:0] = vb_or_vt_exec2[15:8];
    
    wire [15:0] mis_st_val_or_norm = (is_mis_store_exec2) ? bottom_mis_st_val : vb_or_vt_exec2;
    wire [15:0] mis_load_val;
    // More misalign handling
    assign mis_load_val[15:8] = (is_frwd_mem_mis1_exec2) ? mem_overload_exec2[7:0] : mem_rdata0[7:0];
    assign mis_load_val[7:0] = (is_frwd_mem_mis2_exec2) ? mem_overload_exec2[15:8] : mem_rdata1[15:8];
    assign res_wb = (is_load_exec2 && is_frwd_mem_exec2) ? mem_overload_exec2 : (is_mis_load_exec2) ? mis_load_val : (is_load_exec2) ? mem_rdata1 : res_reg_exec2;
    reg  is_jump_wb;
    reg [15:0] delay_vb_or_vt;
    reg [15:0] pc_wb;

    always @(posedge clk) begin
        pc_wb <= pc_exec2;
        delay_vb_or_vt <= mis_st_val_or_norm;
        //is_mis_st_val <= (is_mis_store_exec2); 
        is_jump_wb <= (is_jumping && wb_0);
        if (halt_exec2 && wb_0) begin
            halt <= 1;
            wb_0 <= 0;
        end
        if ((rt_exec2 == 0) && w_reg_en_exec2 && wb_0 && !halt) $write("%c", res_wb[7:0]); 
        pc <= (!do_nothing_reg && wb_0) ? correct_instr_reg : (is_flushing && wb_0) ? pc_exec2 : (is_stall) ? pc : bp_out;
    end

    reg  is_jump_wb_2;

    always @(posedge clk) begin
        is_jump_wb_2 <= is_jump_wb;
    end

    reg  is_jump_wb_3;

    always @(posedge clk) begin
        is_jump_wb_3 <= is_jump_wb_2;
    end


    /**********
     * Memory *
     **********/
    
    // Hijacking for memory reading
    wire[15:0] reg_pc_or_changed = (is_mis_load || is_mis_store) ? forward_va_exec1 + 1 : (pc[0]) ? pc + 1 : pc;
    wire[15:0] hijack_or_reg = (is_jump_wb && pc[0]) ? pc - 1 : (is_mis_load || is_mis_store) ? forward_va_exec1 - 1 : forward_va_exec1;

    mem mem(clk, reg_pc_or_changed[15:1], mem_rdata0, hijack_or_reg[15:1], mem_rdata1, 
        (w_mem_en_exec2 && wb_0 && !halt), va_exec2[15:1], mis_st_val_or_norm);


    /************
     * Register *
     ************/
    
    regs regs(clk, ra, va, read_addr_2, read_data_2, 
        (w_reg_en_exec2 && wb_0 && !halt), rt_exec2, res_wb);

    /******
     * BP *
     ******/
    wire [15:0] bp_out;
    bp bp(clk, pc, bp_out, is_jump_exec2 && wb_0, pc_wb, vb_or_vt_exec2);

    wire a = 2;
    wire b = 1;
    wire x = y + b;
    wire y = x + a;
    wire z;
    assign z = a + x + y + b;

endmodule
