----------------------------------------------------------------------------------------------------------
--  Hyperdimensional Computing Unit(s)                                                                  --
--  Author(s): - Rocco Martino   rocco.martino@uniroma1.it                                              --    
--             - Marco Angioli   marco.angioli@uniroma1.it                                              --
--             - Abdallah Cheikh abdallah.cheikh@uniroma1.it (abdallah93.as@gmail.com)                  --
--                                                                                                      --
--  Date Modified: 26-07-2024                                                                           --
----------------------------------------------------------------------------------------------------------       
--                                       FILE DESCRIPTION                                               --
----------------------------------------------------------------------------------------------------------
--  The HDC unit executes on Hypervector fetched from local-low-latency-wide-bus scratchpad memories.   --
--  The HDCU has six functional units, able to perform the fundamental operation of the Hyperdimensional--
--  Computing paradigm such as bundling, binding and permutation together other frequently operation    --
--  like clipping, similarity and associative search. All these FU support binary HV only               --
--  The data parallelism of the HDCU is defined by the SIMD parameter in the PKG file. Increasing the   --
--  data level parallelism increasess the number of banks per SPM as well, as the number of functional  --
--  units. To increase the instruction level parallelism, the replicated_accl_en parameter must be      --
--  set. Setting it will provide a dedicated HDCU for each hart,                                        --
--  Custom CSRs are implemented for the accelerator unit                                                --
----------------------------------------------------------------------------------------------------------                                                    --
--                                           LICENSE                                                    --
----------------------------------------------------------------------------------------------------------
--  Copyright 2024 Sapienza University of Rome                                                          --
--  SPDX-License-Identifier: Apache-2.0 WITH SHL-2.1                                                    --
--                                                                                                      --
--  Licensed under the Solderpad Hardware License v 2.1 (the “License”);                                --
--  you may not use this file except in compliance with the License, or, at your                        --
--  option, the Apache License version 2.0. You may obtain a copy of the License at                     --
--                                                                                                      --
--  https://solderpad.org/licenses/SHL-2.1/                                                             --
--                                                                                                      --
--  Unless required by applicable law or agreed to in writing, any work                                 --
--  distributed under the License is distributed on an "AS IS" BASIS,                                   --
--  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.                            --
--  See the License for the specific language governing permissions and                                 --
--  limitations under the License.                                                                      --
----------------------------------------------------------------------------------------------------------


-- ieee packages ------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_misc.all;
use ieee.numeric_std.all;
use ieee.math_real.all;
use std.textio.all;

-- local packages -----------------
use work.riscv_klessydra.all;
--use work.klessydra_parameters.all;

-- HDCU  pinout --------------------
entity HDC_Unit is

  generic(
    THREAD_POOL_SIZE      : natural;
    accl_en               : natural;
    replicate_accl_en     : natural;
    multithreaded_accl_en : natural;
    SPM_NUM               : natural; 
    Addr_Width            : natural;
    SIMD                  : natural;
    --------------------------------
    ACCL_NUM              : natural;
    FU_NUM                : natural;
    TPS_CEIL              : natural;
    TPS_BUF_CEIL          : natural;
    SPM_ADDR_WID          : natural;
    SIMD_BITS             : natural;
    Data_Width            : natural;
    SIMD_Width            : natural
    --------------------------------
  );
  port (
  -- Core Signals
    clk_i, rst_ni              : in std_logic;
  -- Processing Pipeline Signals
    rs1_to_sc                  : in  std_logic_vector(SPM_ADDR_WID-1 downto 0);
    rs2_to_sc                  : in  std_logic_vector(SPM_ADDR_WID-1 downto 0);
    rd_to_sc                   : in  std_logic_vector(SPM_ADDR_WID-1 downto 0);
  -- CSR Signals
    HVSIZE                     : in  array_2d(THREAD_POOL_SIZE-1 downto 0)(Addr_Width downto 0);
    MVTYPE                     : in  array_2d(THREAD_POOL_SIZE-1 downto 0)(3 downto 0);
    MPSCLFAC                   : in  array_2d(THREAD_POOL_SIZE-1 downto 0)(4 downto 0);
    hdc_except_data            : out array_2d(ACCL_NUM-1 downto 0)(31 downto 0);
  -- Program Counter Signals
    hdc_taken_branch           : out std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_except_condition       : out std_logic_vector(ACCL_NUM-1 downto 0);
  -- ID_Stage Signals
    decoded_instruction_HDC    : in  std_logic_vector(HDC_UNIT_INSTR_SET_SIZE-1 downto 0);
    harc_EXEC                  : in  natural range THREAD_POOL_SIZE-1 downto 0;
    pc_IE                      : in  std_logic_vector(31 downto 0);
    RS1_Data_IE                : in  std_logic_vector(31 downto 0);
    RS2_Data_IE                : in  std_logic_vector(31 downto 0);
    RD_Data_IE                 : in  std_logic_vector(Addr_Width -1 downto 0);
    hdc_instr_req              : in  std_logic_vector(ACCL_NUM-1 downto 0);
    spm_rs1                    : in  std_logic;
    spm_rs2                    : in  std_logic;
    vec_read_rs1_ID            : in  std_logic;
    vec_read_rs2_ID            : in  std_logic;
    vec_write_rd_ID            : in  std_logic;
    busy_hdc                   : out std_logic_vector(ACCL_NUM-1 downto 0);
  -- Scratchpad Interface Signals
    hdc_data_gnt_i             : in  std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_sci_wr_gnt             : in  std_logic_vector(ACCL_NUM-1 downto 0);
    hdc_sc_data_read           : in  array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(SIMD_Width-1 downto 0);
    hdc_we_word                : out array_2d(ACCL_NUM-1 downto 0)(SIMD-1 downto 0);
    hdc_sc_read_addr           : out array_3d(ACCL_NUM-1 downto 0)(1 downto 0)(Addr_Width-1 downto 0);
    hdc_to_sc                  : out array_3d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0)(1 downto 0);
    hdc_sc_data_write_wire     : out array_2d(ACCL_NUM-1 downto 0)(SIMD_Width-1 downto 0);
    hdc_sc_write_addr          : out array_2d(ACCL_NUM-1 downto 0)(Addr_Width-1 downto 0);
    hdc_sci_we                 : out array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    hdc_sci_req                : out array_2d(ACCL_NUM-1 downto 0)(SPM_NUM-1 downto 0);
    -- Tracer signals
    state_HDC                  : out array_2d(ACCL_NUM-1 downto 0)(1 downto 0)
  );

end entity;  

------------------------------------------

architecture HDC of HDC_Unit is

  subtype harc_range is natural range THREAD_POOL_SIZE-1 downto 0;
  subtype accl_range is integer range ACCL_NUM-1 downto 0;
  subtype fu_range   is integer range FU_NUM-1 downto 0;  

  signal nextstate_HDC : array_2d(accl_range)(1 downto 0);

  -- Virtual Parallelism Signals
  signal halt_hart                       : std_logic_vector(accl_range); -- halts the thread when the requested functional unit is in use
  signal fu_req                          : array_2D(accl_range)(8 downto 0); -- Each threa has request bits equal to the total number of FUs
  signal fu_gnt                          : array_2D(accl_range)(8 downto 0); -- Each threa has grant bits equal to the total number of FUs
  signal fu_gnt_wire                     : array_2D(accl_range)(8 downto 0); -- Each threa has grant bits equal to the total number of FUs
  signal fu_gnt_en                       : array_2D(accl_range)(8 downto 0); -- Enable the giving of the grant to the thread pointed at by the issue buffer
  signal fu_rd_ptr                       : array_2D(8 downto 0)(TPS_BUF_CEIL-1 downto 0); -- five rd pointers each has a number of bits equal to ceil(log2(THREAD_POOL_SIZE-1))
  signal fu_wr_ptr                       : array_2D(8 downto 0)(TPS_BUF_CEIL-1 downto 0); -- five rd pointers each has a number of bits equal to ceil(log2(THREAD_POOL_SIZE-1))
  signal fu_issue_buffer                 : array_3D(8 downto 0)(THREAD_POOL_SIZE-2 downto 0)(TPS_CEIL-1 downto 0);
  signal hdc_sc_data_write_wire_int      : array_2d(accl_range)(SIMD_Width-1 downto 0);
  signal hdc_sc_data_write_int           : array_2d(accl_range)(SIMD_Width-1 downto 0);
  signal vec_write_rd_HDC                : std_logic_vector(accl_range);  -- Indicates whether the result being written is a vector or a scalar
  signal vec_read_rs1_HDC                : std_logic_vector(accl_range);  -- Indicates whether the operand being read is a vector or a scalar
  signal vec_read_rs2_HDC                : std_logic_vector(accl_range);  -- Indicates whether the operand being read is a vector or a scalar
  signal wb_ready                        : std_logic_vector(accl_range);
  signal halt_hdc                        : std_logic_vector(accl_range);
  signal halt_hdc_lat                    : std_logic_vector(accl_range);
  signal recover_state                   : std_logic_vector(accl_range);
  signal recover_state_wires             : std_logic_vector(accl_range);
  signal hdc_data_gnt_i_lat              : std_logic_vector(accl_range);
  signal hdc_except_data_wire            : array_2d(accl_range)(31 downto 0);
  signal decoded_instruction_HDC_lat     : array_2d(accl_range)(HDC_UNIT_INSTR_SET_SIZE -1 downto 0);
  signal overflow_rs1_sc                 : array_2d(accl_range)(Addr_Width downto 0);
  signal overflow_rs2_sc                 : array_2d(accl_range)(Addr_Width downto 0);
  signal overflow_rd_sc                  : array_2d(accl_range)(Addr_Width downto 0);
  signal hdc_rs1_to_sc                   : array_2d(accl_range)(SPM_ADDR_WID-1 downto 0);
  signal hdc_rs2_to_sc                   : array_2d(accl_range)(SPM_ADDR_WID-1 downto 0);
  signal hdc_rd_to_sc                    : array_2d(accl_range)(SPM_ADDR_WID-1 downto 0);
  signal hdc_sc_data_read_mask           : array_2d(accl_range)(SIMD_Width-1 downto 0);
  signal RS1_Data_IE_lat                 : array_2d(accl_range)(31 downto 0);
  signal RS2_Data_IE_lat                 : array_2d(accl_range)(31 downto 0);
  signal RD_Data_IE_lat                  : array_2d(accl_range)(Addr_Width -1 downto 0);
  signal HVSIZE_READ                     : array_2d(accl_range)(Addr_Width downto 0);  -- Bytes remaining to read
  signal HVSIZE_READ_lat                 : array_2d(accl_range)(Addr_Width downto 0);  -- Bytes remaining to read
  signal HVSIZE_READ_MASK                : array_2d(accl_range)(Addr_Width downto 0);  -- Bytes remaining to read
  signal HVSIZE_WRITE                    : array_2d(accl_range)(Addr_Width downto 0);  -- Bytes remaining to write
  signal CLASS_NUM                       : array_2d(accl_range)(4 downto 0);
  signal busy_hdc_internal               : std_logic_vector(accl_range);
  signal busy_hdc_internal_lat           : std_logic_vector(accl_range);
  signal rf_rs2                          : std_logic_vector(accl_range);
  signal SIMD_RD_BYTES_wire              : array_2d_int(accl_range);
  signal SIMD_RD_BYTES                   : array_2d_int(accl_range);
  
  ------------------ BUNDLING-- ------------------
  constant COUNTERS_NUMBER                  : natural := Data_Width/COUNTER_BITS; -- number of counters in the array

  signal busy_bundle                        : std_logic;                     -- busy signal active only when the FU is shared and currently in use 
  signal busy_bundle_wire                   : std_logic;                     -- busy signal active only when the FU is shared and currently in use 
  
  signal bundle_en                          : std_logic_vector(accl_range);  -- enables the use of the adders
  signal bundle_en_wire                     : std_logic_vector(accl_range);  -- enables the use of the adders
  signal bundle_en_pending                  : std_logic_vector(accl_range);  -- signal to preserve the request to access the adder "multhithreaded mode" only
  signal bundle_en_pending_wire             : std_logic_vector(accl_range);  -- signal to preserve the request to access the adder "multhithreaded mode" only
  signal bundle_stage_1_en                  : std_logic_vector(accl_range);
  signal bundle_stage_2_en                  : std_logic_vector(accl_range);

  signal counters                           : CountArray3d(fu_range)(SIMD-1 downto 0)(COUNTERS_NUMBER-1 downto 0)(COUNTER_BITS-1 downto 0); -- array di contatori
  signal counters_wire                      : CountArray3d(fu_range)(SIMD-1 downto 0)(COUNTERS_NUMBER-1 downto 0)(COUNTER_BITS-1 downto 0);
  signal bundle_offset                      : array_3d_int(accl_range)(SIMD-1 downto 0);
  signal bundle_processed_bytes             : array_2d_int(accl_range);

  signal hdcu_in_bundle_operands            : array_3d(fu_range)(1 downto 0)(SIMD_Width - 1 downto 0);
  signal hdcu_out_bundle_results            : array_2d(fu_range)(SIMD_Width-1 downto 0);
  ------------------------------------------------

  ------------------ BINDING ---------------------
  signal busy_bind                        : std_logic;                     -- busy signal active only when the FU is shared and currently in use 
  signal busy_bind_wire                   : std_logic;                     -- busy signal active only when the FU is shared and currently in use 
  
  signal bind_en                          : std_logic_vector(accl_range);  -- enables the use of the multipliers
  signal bind_en_wire                     : std_logic_vector(accl_range);  -- enables the use of the multipliers
  signal bind_en_pending                  : std_logic_vector(accl_range);  -- signal to preserve the request to access the multiplier "multhithreaded mode" only
  signal bind_en_pending_wire             : std_logic_vector(accl_range);  -- signal to preserve the request to access the multiplier "multhithreaded mode" only
  signal bind_stage_1_en                  : std_logic_vector(accl_range);
  signal bind_stage_2_en                  : std_logic_vector(accl_range);

  signal hdcu_in_bind_operands            : array_3d(fu_range)(1 downto 0)(SIMD_Width-1 downto 0);
  signal hdcu_out_bind_results            : array_2d(fu_range)(SIMD_Width-1 downto 0);
  ------------------------------------------------

  ------------------ SIMILARITY ------------------
  constant SIMILARITY_BITS               : natural := 13; -- Number of bits used to represent the similarity: max 2^13 = 8192;
  
  signal busy_sim                        : std_logic; 
  signal busy_sim_wire                   : std_logic;
  
  signal sim_en                          : std_logic_vector(accl_range); 
  signal sim_en_wire                     : std_logic_vector(accl_range);
  signal sim_en_pending                  : std_logic_vector(accl_range); 
  signal sim_en_pending_wire             : std_logic_vector(accl_range);
  signal sim_stage_1_en                  : std_logic_vector(accl_range);
  signal sim_stage_2_en                  : std_logic_vector(accl_range);
  
  signal xor_out                         : array_2d(fu_range)(SIMD_Width - 1 downto 0);
  signal partial_sim_measure             : array_2d(fu_range)(SIMD_Width - 1 downto 0);

  signal sim_measure                     : array_2d(fu_range)(SIMD_Width - 1 downto 0);
  signal sim_measure_reg                 : array_2d(fu_range)(SIMD_Width - 1 downto 0);

  signal hamming_distance_wire           : array_2d(accl_range)(SIMILARITY_BITS - 1 downto 0);

  signal hdcu_in_sim_operands            : array_3d(fu_range)(1 downto 0)(SIMD_Width - 1 downto 0);
  signal hdcu_out_sim_results            : array_2d(fu_range)(SIMILARITY_BITS - 1 downto 0);


  function log2(x : integer) return integer is
    begin
        return integer(ceil(log2(real(x))));
  end function;
  
  function popcount(x : std_logic_vector) return integer is
    variable count : integer := 0;
    begin
      for i in x'range loop
        if x(i) = '1' then
          count := count + 1;
        end if;
      end loop;
      return count;
  end function;
  ------------------------------------------------
  
  ----------------- CLIPPING ---------------------
  signal busy_clip                       : std_logic;
  signal busy_clip_wire                  : std_logic;
  
  signal clip_en                         : std_logic_vector(accl_range);
  signal clip_en_wire                    : std_logic_vector(accl_range);
  signal clip_en_pending                 : std_logic_vector(accl_range);
  signal clip_en_pending_wire            : std_logic_vector(accl_range);
  signal clip_stage_1_en                 : std_logic_vector(accl_range);
  signal clip_stage_2_en                 : std_logic_vector(accl_range);

  signal clip_processed_bytes            : array_2d_int(accl_range);

  signal hdcu_in_clip_operand_0          : array_2d(fu_range)(SIMD_Width - 1 downto 0);
  signal hdcu_in_clip_operand_1          : array_2d(fu_range)(Data_Width - 1 downto 0); 
  signal hdcu_out_clip_results           : array_2d(fu_range)(SIMD_Width - 1 downto 0);

  signal harc_f                          : array_2d_int(accl_range);
  ------------------------------------------------

  ----------------- PERMUTATION ------------------
  signal busy_perm                       : std_logic;
  signal busy_perm_wire                  : std_logic;

  signal perm_en                         : std_logic_vector(accl_range);
  signal perm_en_wire                    : std_logic_vector(accl_range);
  signal perm_en_pending                 : std_logic_vector(accl_range);
  signal perm_en_pending_wire            : std_logic_vector(accl_range);
  signal perm_stage_1_en                 : std_logic_vector(accl_range);
  signal perm_stage_2_en                 : std_logic_vector(accl_range);
  
  signal hdcu_in_perm_operand_0          : array_2d(fu_range)(SIMD_Width - 1 downto 0); -- Contiene ipervettore
  signal hdcu_in_perm_operand_1          : array_2d(fu_range)(Data_Width - 1 downto 0); -- Contiene immediato
  signal hdcu_out_perm_results           : array_2d(fu_range)(SIMD_Width - 1 downto 0); -- Contiene risultato
  signal buffer_reg                      : array_2d(fu_range)(Data_Width - 1 downto 0); -- Buffer per permutazione, contiene i bit shiftati fuori

  signal first_perm_source_addr          : array_2d(ACCL_NUM-1 downto 0)(Addr_Width-1 downto 0);
  signal first_perm_dest_addr            : array_2d(ACCL_NUM-1 downto 0)(Addr_Width-1 downto 0);
  signal new_perm_offset                 : array_2d_int(accl_range);
  signal shift_amount                    : array_2d_shift_int(fu_range);
  ------------------------------------------------

  ---------------ASSOCIATIVE SEARCH --------------
  signal busy_as                         : std_logic;
  signal busy_as_wire                    : std_logic;

  signal as_en                           : std_logic_vector(accl_range);
  signal as_en_wire                      : std_logic_vector(accl_range);
  signal as_en_pending                   : std_logic_vector(accl_range);
  signal as_en_pending_wire              : std_logic_vector(accl_range);
  signal as_stage_1_en                   : std_logic_vector(accl_range);
  signal as_stage_2_en                   : std_logic_vector(accl_range);
  signal as_stage_3_en                   : std_logic_vector(accl_range);

  signal sim_class                       : array_2d_int_sim(accl_range); -- Similarity per classe
  signal temp_best_sim_class             : array_2d_int_sim(accl_range); -- Migliore similarità per classe temporanea

  signal class_index                     : array_2d_shift_int(accl_range); -- Indice della classe che sto processando
  signal temp_best_class_index           : array_2d_shift_int(accl_range); -- Indice della classe con migliore similarità

  signal AMSIZE_READ                     : array_2d(accl_range)(Addr_Width downto 0);   -- Bytes remaining to read in the associative memory
  signal head_ptr_encoded_hv             : array_2d(accl_range)(31 downto 0); -- Puntatore alla testa dell'encoded vector. Serve per ricaricare lo stesso encoded vector.
  -------------------------------------------------------------------------

---------------------------------- HDCU ARCHITECTURE BEGIN -------------------------------------------

begin
  busy_hdc <= busy_hdc_internal;

  HDC_replicated : for h in accl_range generate
    harc_f(h) <= 0 when multithreaded_accl_en = 1 else h;
  
  ----------------------------- Sequential Stage of HDC Unit ----------------------------------------

  HDC_Exec_Unit : process(clk_i, rst_ni)  -- single cycle unit, fully synchronous 
  
  begin
    if rst_ni = '0' then
      rf_rs2(h)     <= '0';
      recover_state(h) <= '0';
    elsif rising_edge(clk_i) then
      
      HVSIZE_READ_lat(h) <= HVSIZE_READ(h);

      if hdc_instr_req(h) = '1' or busy_hdc_internal_lat(h) = '1' then  

        case state_HDC(h) is

          when hdc_init =>

            -------------------------------------------------------------
            -- ██╗███╗   ██╗██╗████████╗    ██╗  ██╗██████╗  ██████╗   --
            -- ██║████╗  ██║██║╚══██╔══╝    ██║  ██║██╔══██╗██╔════╝   --
            -- ██║██╔██╗ ██║██║   ██║       ███████║██║  ██║██║        --
            -- ██║██║╚██╗██║██║   ██║       ██╔══██║██║  ██║██║        --
            -- ██║██║ ╚████║██║   ██║       ██║  ██║██████╔╝╚██████╗   --
            -- ╚═╝╚═╝  ╚═══╝╚═╝   ╚═╝       ╚═╝  ╚═╝╚═════╝  ╚═════╝   --
            -------------------------------------------------------------

            if (decoded_instruction_HDC(HVCLIP_bit_position  ) = '1' or 
               decoded_instruction_HDC(HVPERM_bit_position) = '1' )  then
              rf_rs2(h) <= '1';
            else
              rf_rs2(h) <= '0';  
            end if;

            -- We backup data from decode stage since they will get updated
            HVSIZE_READ_MASK(h) <= HVSIZE(harc_EXEC); 
            CLASS_NUM(h) <= MPSCLFAC(harc_EXEC); -- Number of classes we retrieve from the CSR
            class_index(h) <= 0; 
            
            if decoded_instruction_HDC(HVSEARCH_bit_position) = '1' then
              AMSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)), HVSIZE_READ(h)'length));
            end if;

            -- When the decoded instruction is a bundle, we need to multiply the HVSIZE_WRITE by (Data_Width/COUNTERS_NUMBER)
            if decoded_instruction_HDC(HVBUNDLE_bit_position)    = '1' then
              HVSIZE_WRITE(h) <= std_logic_vector(resize(unsigned(HVSIZE(h)) * Data_Width/COUNTERS_NUMBER, HVSIZE_WRITE(h)'length));
            elsif (decoded_instruction_HDC(HVSIM_bit_position)    = '1' or
                   decoded_instruction_HDC(HVSEARCH_bit_position)    = '1') then
              HVSIZE_WRITE(h) <= std_logic_vector(to_unsigned(4, HVSIZE_WRITE(h)'length));
            else
              HVSIZE_WRITE(h) <= HVSIZE(harc_EXEC);
            end if;

            decoded_instruction_HDC_lat(h)  <= decoded_instruction_HDC;
            
            vec_write_rd_HDC(h) <= vec_write_rd_ID;
            vec_read_rs1_HDC(h) <= vec_read_rs1_ID;
            vec_read_rs2_HDC(h) <= vec_read_rs2_ID;
          
            hdc_rs1_to_sc(h) <= rs1_to_sc;
            hdc_rs2_to_sc(h) <= rs2_to_sc;
            hdc_rd_to_sc(h)  <= rd_to_sc;
            RD_Data_IE_lat(h) <= RD_Data_IE;
            
            -- We need to keep the addresses the same when the decoded instruction is a bundle or a clipping
            if (decoded_instruction_HDC(HVPERM_bit_position) = '1') then
              first_perm_source_addr(h) <= RS1_Data_IE(Addr_Width - 1 downto 0);
              first_perm_dest_addr(h) <= RD_Data_IE;
            end if;

            -- Increment the read addresses if there is a data grant
            if hdc_data_gnt_i(h) = '1' then
              
              ------------------ Source Register 1 ------------------
              if vec_read_rs1_ID = '1'  then

                if decoded_instruction_HDC(HVSEARCH_bit_position) = '1' and to_integer(unsigned(HVSIZE(h))) <= SIMD_RD_BYTES_wire(h) then
                  RS1_Data_IE_lat(h) <= RS1_Data_IE;
                else
                  RS1_Data_IE_lat(h) <= std_logic_vector(unsigned(RS1_Data_IE) + SIMD_RD_BYTES_wire(h));  -- source 1 address increment
                end if;

              else
                RS1_Data_IE_lat(h) <= RS1_Data_IE;
              end if;
              -------------------------------------------------------

              ------------------ Source Register 2 ------------------
              if vec_read_rs2_ID = '1' and decoded_instruction_HDC(HVBUNDLE_bit_position) = '0' then
                if decoded_instruction_HDC(HVSEARCH_bit_position) = '1' and to_integer(unsigned(HVSIZE(h))) < SIMD_RD_BYTES_wire(h) then
                  RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE) + unsigned(HVSIZE(h)));
                else
                  RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE) + SIMD_RD_BYTES_wire(h)); 
                end if;
              else
                RS2_Data_IE_lat(h) <= RS2_Data_IE;
              end if;
              -------------------------------------------------------

              -- Decrement the vector elements that have already been operated on
              if  (decoded_instruction_HDC(HVBUNDLE_bit_position)  = '1' or        
                   decoded_instruction_HDC(HVCLIP_bit_position)    = '1') then
                if (unsigned(HVSIZE(harc_EXEC)) * to_unsigned(Data_Width/COUNTERS_NUMBER, HVSIZE(harc_EXEC)'length)) >= SIMD_RD_BYTES_wire(h) then
                  HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) * Data_Width/COUNTERS_NUMBER, HVSIZE_READ(h)'length)); -- decrement by SIMD_BYTE Execution Capability
                else
                  HVSIZE_READ(h) <= (others => '0');                                                             -- decrement the remaining bytes
                end if;

              elsif decoded_instruction_HDC(HVPERM_bit_position)  = '1'  then
                if unsigned(HVSIZE(harc_EXEC)) >= SIMD_RD_BYTES_wire(h) then
                  HVSIZE_READ(h) <= (HVSIZE(harc_EXEC));       -- decrement by SIMD_BYTE Execution Capability            
                else
                  HVSIZE_READ(h) <= std_logic_vector(to_unsigned(SIMD_RD_BYTES_wire(h), HVSIZE_READ(h)'length));
                  HVSIZE_READ_lat(h) <= std_logic_vector(to_unsigned(SIMD_RD_BYTES_wire(h), HVSIZE_READ(h)'length));
                end if;
              
              elsif decoded_instruction_HDC(HVSEARCH_bit_position)  = '1' then
                -- The number of classes is contained in the CSR in the MPSCLFAC signal, so the size of the associative memory is MPSCLFAC * HVSIZE
                HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) * unsigned(MPSCLFAC(harc_EXEC)), HVSIZE_READ(h)'length)); 
                -- Save the address of the encoded vector because it must be presented every time I finish calculating the similarity with a class vector
                head_ptr_encoded_hv(h) <= RS1_Data_IE;

              else
                if unsigned(HVSIZE(harc_EXEC)) >= SIMD_RD_BYTES_wire(h) then
                  HVSIZE_READ(h) <= std_logic_vector(unsigned(HVSIZE(harc_EXEC)) - SIMD_RD_BYTES_wire(h));       -- decrement by SIMD_BYTE Execution Capability            
                else
                  HVSIZE_READ(h) <= (others => '0');                                                             -- decrement the remaining bytes
                end if;
              end if;

            -- If there is no data grant, we keep the addresses the same
            else

              RS1_Data_IE_lat(h) <= RS1_Data_IE;
              RS2_Data_IE_lat(h) <= RS2_Data_IE;

              -- When the decoded instruction is a bundle/clipping, we need to multiply the HVSIZE_READ by Data_Width/COUNTERS_NUMBER
              if decoded_instruction_HDC(HVBUNDLE_bit_position)  = '1' or
                 decoded_instruction_HDC(HVCLIP_bit_position  )    = '1' then
                HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) * Data_Width/COUNTERS_NUMBER, HVSIZE_READ(h)'length));
              
              elsif decoded_instruction_HDC(HVSEARCH_bit_position)  = '1' then
                HVSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(harc_EXEC)) * unsigned(MPSCLFAC(harc_EXEC)), HVSIZE_READ(h)'length)); 
                head_ptr_encoded_hv(h) <= RS1_Data_IE;

              else
                HVSIZE_READ(h) <= HVSIZE(harc_EXEC);
              end if;  

            end if;

           ---------------------------------------------------------------------------

          when hdc_exec =>
            recover_state(h) <= recover_state_wires(h);
            if halt_hdc(h) = '1' and halt_hdc_lat(h) = '0' then
              hdc_sc_data_write_int(h) <= hdc_sc_data_write_wire_int(h);
            end if;

            --------------------------------------------------------------------------
            --  ██╗  ██╗██╗    ██╗      ██╗      ██████╗  ██████╗ ██████╗ ███████╗  --
            --  ██║  ██║██║    ██║      ██║     ██╔═══██╗██╔═══██╗██╔══██╗██╔════╝  --
            --  ███████║██║ █╗ ██║█████╗██║     ██║   ██║██║   ██║██████╔╝███████╗  --
            --  ██╔══██║██║███╗██║╚════╝██║     ██║   ██║██║   ██║██╔═══╝ ╚════██║  --
            --  ██║  ██║╚███╔███╔╝      ███████╗╚██████╔╝╚██████╔╝██║     ███████║  --
            --  ╚═╝  ╚═╝ ╚══╝╚══╝       ╚══════╝ ╚═════╝  ╚═════╝ ╚═╝     ╚══════╝  --            
            --------------------------------------------------------------------------

            if halt_hdc(h) = '0' then

              ------------------------------- Increment the write address when we have a result as a vector -------------------------------

              if vec_write_rd_HDC(h) = '1' and wb_ready(h) = '1' then
                RD_Data_IE_lat(h)  <= std_logic_vector(unsigned(RD_Data_IE_lat(h)) + SIMD_RD_BYTES_wire(h)); -- destination address increment
              end if;
              
              ------------------------------- Decrement the number of bytes to write -------------------------------
              if wb_ready(h) = '1' then
                
                if to_integer(unsigned(HVSIZE_WRITE(h))) >= SIMD_RD_BYTES_wire(h) then
                  HVSIZE_WRITE(h) <= std_logic_vector(unsigned(HVSIZE_WRITE(h)) - SIMD_RD_BYTES_wire(h));    -- decrement by SIMD_BYTE Execution Capability 
                else
                  HVSIZE_WRITE(h) <= (others => '0');                                                        -- decrement the remaining bytes
                end if;
                
              end if;
              
              ------------------------------- Increment the read addresses -------------------------------

              if to_integer(unsigned(HVSIZE_READ(h))) >= SIMD_RD_BYTES_wire(h) and hdc_data_gnt_i(h) = '1' then -- Increment the addresses untill all the vector elements are operated fetched
                
                ----------------------------------------- Source Register 1 -----------------------------------------
                if vec_read_rs1_HDC(h) = '1' then
                  
                  if decoded_instruction_HDC(HVSEARCH_bit_position) = '1' then

                  -- Incremento l'indice della classe ogni volta che ho finito di leggere un class vector
                    if to_integer(unsigned(AMSIZE_READ(h))) = 0 and as_stage_3_en(h)='0' then 
                      class_index(h) <= (class_index(h) + 1);
                    end if;

                    -- Se ho quasi finito di leggere un class vector (AMSIZE_READ = 2*SIMD_RD_BYTES_wire) e 
                    -- ho ancora piu di un class vector da leggere (HVSIZE_READ > HVSIZE) o
                    -- sto leggendo un class vector intero a ciclo di clock (HVSIZE = SIMD_RD_BYTES_wire) allora assegno a RS1 l'indirizzo dell'encoded vector
                    if (to_integer(unsigned(AMSIZE_READ(h))) = 2*SIMD_RD_BYTES_wire(h)  and 
                        to_integer(unsigned(HVSIZE_READ(h))) > to_integer(unsigned(HVSIZE(h)))) or
                        to_integer(unsigned(HVSIZE(h))) = SIMD_RD_BYTES_wire(h) then 
                      RS1_Data_IE_lat(h) <= head_ptr_encoded_hv(h);
                    
                    -- Se il valore a cui si resetta AMSIZE_READ è minore = SIMD_RD_BYTES_wire(h) allora resetto RS1 a head_ptr_encoded_hv(h)

                    elsif to_integer(unsigned(HVSIZE(h)) - SIMD_RD_BYTES_wire(h)) = SIMD_RD_BYTES_wire(h) and
                          to_integer(unsigned(AMSIZE_READ(h))) = 0 then
                      RS1_Data_IE_lat(h) <= head_ptr_encoded_hv(h);
                    
                    -- La condizione di default è incrementare RS1 di SIMD_RD_BYTES_wire(h)
                    else
                      RS1_Data_IE_lat(h) <= std_logic_vector(unsigned(RS1_Data_IE_lat(h)) + SIMD_RD_BYTES_wire(h)); -- source 1 address increment
                    end if;
                  
                  else 

                    RS1_Data_IE_lat(h) <= std_logic_vector(unsigned(RS1_Data_IE_lat(h)) + SIMD_RD_BYTES_wire(h)); -- source 1 address increment

                  end if; 
                end if; 
                
                ----------------------------------------- Source Register 2 -----------------------------------------
                if vec_read_rs2_HDC(h) = '1' then
                  
                  if decoded_instruction_HDC(HVBUNDLE_bit_position) = '1' then
                    
                    if bundle_processed_bytes(h) = 1 then
                    
                      RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE_lat(h)) + SIMD_RD_BYTES_wire(h)); -- source 2 address increment

                    end if;

                  elsif decoded_instruction_HDC(HVSEARCH_bit_position) = '1'  and to_integer(unsigned(HVSIZE(h))) < SIMD_RD_BYTES_wire(h) then
                  -- Se ho una search e HVSIZE_READ è minore di SIMD_RD_BYTES_wire(h) incremento RS2 di HVSIZE e non di SIMD_RD_BYTES_wire(h)
                  -- cosi leggo un class vector ogni ciclo di clock
                    RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE_lat(h)) + unsigned(HVSIZE(h))); -- source 2 address increment
                    
                  else

                    RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE_lat(h)) + SIMD_RD_BYTES_wire(h)); -- source 2 address increment

                  end if;

                end if;
              
              else -- se HVSIZE_READ è minore di SIMD_RD_BYTES_wire(h) incremento RS2 di HVSIZE e non di SIMD_RD_BYTES_wire(h)

                if decoded_instruction_HDC(HVSEARCH_bit_position) = '1'  and to_integer(unsigned(HVSIZE(h))) < SIMD_RD_BYTES_wire(h) then

                  -- Incremento l'indice della classe ogni volta che ho finito di leggere un class vector
                  if to_integer(unsigned(AMSIZE_READ(h))) = 0 and as_stage_3_en(h)='0' then 
                    class_index(h) <= (class_index(h) + 1);
                  end if;

                  RS2_Data_IE_lat(h) <= std_logic_vector(unsigned(RS2_Data_IE_lat(h)) + unsigned(HVSIZE(h))); -- source 2 address increment

                end if;

              end if;

              -------------------------- Decrement the vector elements that have already been operated on ---------------------------------------
              if hdc_data_gnt_i(h) = '1' then
                
                if to_integer(unsigned(HVSIZE_READ(h))) >= SIMD_RD_BYTES_wire(h) then
                  
                  HVSIZE_READ(h) <= std_logic_vector(unsigned(HVSIZE_READ(h)) - SIMD_RD_BYTES_wire(h)); -- decrement by SIMD_BYTE Execution Capability
                  
                  if decoded_instruction_HDC(HVSEARCH_bit_position) = '1' then

                    -- Se la dimensione dell'ipervettore è maggiore della capacità di lettura 
                    if to_integer(unsigned(AMSIZE_READ(h))) > SIMD_RD_BYTES_wire(h) then

                      -- Decremento di SIMD_RD_BYTES_wire(h) 
                      AMSIZE_READ(h) <= std_logic_vector(unsigned(AMSIZE_READ(h)) - SIMD_RD_BYTES_wire(h));

                    -- Se ho finito di leggere un class vector allora lo resetto
                    elsif to_integer(unsigned(AMSIZE_READ(h))) = 0 then
                      AMSIZE_READ(h) <= std_logic_vector(resize(unsigned(HVSIZE(h)) - SIMD_RD_BYTES_wire(h), HVSIZE_READ(h)'length));
                    
                    -- Di default lo lascio a 0
                    else
                      AMSIZE_READ(h) <= (others => '0');
                      --HVSIZE_READ(h) <= std_logic_vector(unsigned(HVSIZE_READ(h)) - unsigned(HVSIZE(h))); 
                    end if;

                  end if;

                else
                  -- Qui vado a coprire il caso in cui la memoria associativa è minore della capacità di lettura
                  -- Quindi significa che potrei leggere piu di un class vector a ciclo di clock
                  -- Quando questo accade dobbiamo tenere AMSIZE_READ(h) a 0 per leggere sempre l'indirizzo dell'encoded vector
                  -- e decrementare HVSIZE_READ(h) di HVSIZE(h) .
                  if decoded_instruction_HDC(HVSEARCH_bit_position) = '1' then
                    HVSIZE_READ(h) <= std_logic_vector(unsigned(HVSIZE_READ(h)) - unsigned(HVSIZE(h)));
                    AMSIZE_READ(h) <= (others => '0'); 
                  else
                    HVSIZE_READ(h) <= (others => '0');                                                    -- decrement the remaining bytes
                  end if;

                end if;
                if SIMD_RD_BYTES_wire(h) > to_integer(unsigned(HVSIZE(h))) then
                  HVSIZE_READ(h) <= std_logic_vector(unsigned(HVSIZE_READ(h)) - unsigned(HVSIZE(h)));
                end if;
              end if;
              
              hdc_sc_data_read_mask(h) <= (others => '0');
              
              if hdc_data_gnt_i_lat(h) = '1' then
                if to_integer(unsigned(HVSIZE_READ_MASK(h))) >= SIMD_RD_BYTES_wire(h) then
                  hdc_sc_data_read_mask(h) <= (others => '1');
                  HVSIZE_READ_MASK(h) <= std_logic_vector(unsigned(HVSIZE_READ_MASK(h)) - SIMD_RD_BYTES_wire(h)); -- decrement by SIMD_BYTE Execution Capability 
                else
                  HVSIZE_READ_MASK(h) <= (others => '0');
                  hdc_sc_data_read_mask(h)(to_integer(unsigned(HVSIZE_READ_MASK(h)))*8 - 1 downto 0) <= (others => '1');
                end if;
              end if;
            end if;

          when others =>
            null;
        end case;
      end if;
    end if;
  end process;

  ------------ Combinational Stage of HDC Unit ------------
  HDC_Excpt_Cntrl_Unit_comb : process(all)
  
  variable busy_hdc_internal_wires : std_logic;
  variable hdc_except_condition_wires : std_logic_vector(harc_range);
  variable hdc_taken_branch_wires : std_logic_vector(harc_range);  
      
  begin

    busy_hdc_internal_wires        := '0';
    hdc_except_condition_wires(h)  := '0';
    hdc_taken_branch_wires(h)      := '0';
    wb_ready(h)                    <= '0';
    halt_hdc(h)                    <= '0';
    nextstate_HDC(h)               <= hdc_init;
    recover_state_wires(h)         <= recover_state(h);
    hdc_except_data_wire(h)        <= hdc_except_data(h);
    overflow_rs1_sc(h)             <= (others => '0');
    overflow_rs2_sc(h)             <= (others => '0');
    overflow_rd_sc(h)              <= (others => '0');
    hdc_we_word(h)                 <= (others => '0');
    hdc_sci_req(h)                 <= (others => '0');
    hdc_sci_we(h)                  <= (others => '0');
    hdc_sc_write_addr(h)           <= (others => '0');
    hdc_sc_read_addr(h)            <= (others => (others => '0'));
    hdc_to_sc(h)                   <= (others => (others => '0'));

    if hdc_instr_req(h) = '1' or busy_hdc_internal_lat(h) = '1' then
      case state_HDC(h) is

        when hdc_init =>

          ---------------------------------------------------------------------------------------------------------------------
          --  ███████╗██╗  ██╗ ██████╗██████╗ ████████╗    ██╗  ██╗ █████╗ ███╗   ██╗██████╗ ██╗     ██╗███╗   ██╗ ██████╗   --
          --  ██╔════╝╚██╗██╔╝██╔════╝██╔══██╗╚══██╔══╝    ██║  ██║██╔══██╗████╗  ██║██╔══██╗██║     ██║████╗  ██║██╔════╝   --
          --  █████╗   ╚███╔╝ ██║     ██████╔╝   ██║       ███████║███████║██╔██╗ ██║██║  ██║██║     ██║██╔██╗ ██║██║  ███╗  --
          --  ██╔══╝   ██╔██╗ ██║     ██╔═══╝    ██║       ██╔══██║██╔══██║██║╚██╗██║██║  ██║██║     ██║██║╚██╗██║██║   ██║  -- 
          --  ███████╗██╔╝ ██╗╚██████╗██║        ██║       ██║  ██║██║  ██║██║ ╚████║██████╔╝███████╗██║██║ ╚████║╚██████╔╝  --
          --  ╚══════╝╚═╝  ╚═╝ ╚═════╝╚═╝        ╚═╝       ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝╚═╝╚═╝  ╚═══╝ ╚═════╝   --
          ---------------------------------------------------------------------------------------------------------------------

          overflow_rs1_sc(h) <= std_logic_vector('0' & unsigned(RS1_Data_IE(Addr_Width -1 downto 0)) + unsigned(HVSIZE(harc_EXEC)) -1);
          overflow_rs2_sc(h) <= std_logic_vector('0' & unsigned(RS2_Data_IE(Addr_Width -1 downto 0)) + unsigned(HVSIZE(harc_EXEC)) -1);
          overflow_rd_sc(h)  <= std_logic_vector('0' & unsigned(RD_Data_IE(Addr_Width  -1 downto 0)) + unsigned(HVSIZE(harc_EXEC)) -1);
          if HVSIZE(harc_EXEC) = (0 to Addr_Width => '0') then
            null;
          elsif HVSIZE(harc_EXEC)(1 downto 0) /= "00" and MVTYPE(harc_EXEC)(3 downto 2) = "10" then  -- Set exception if the number of bytes are not divisible by four
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_wires(h)     := '1';    
            hdc_except_data_wire(h) <= ILLEGAL_VECTOR_SIZE_EXCEPT_CODE;
          elsif HVSIZE(harc_EXEC)(0) /= '0' and MVTYPE(harc_EXEC)(3 downto 2) = "01" then            -- Set exception if the number of bytes are not divisible by two
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_wires(h)     := '1';
            hdc_except_data_wire(h) <= ILLEGAL_VECTOR_SIZE_EXCEPT_CODE;
          elsif (rs1_to_sc  = "100" and vec_read_rs1_ID = '1') or
            (rs2_to_sc  = "100" and vec_read_rs2_ID = '1') or
             rd_to_sc   = "100" then     -- Set exception for non scratchpad access
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_wires(h)     := '1';    
            hdc_except_data_wire(h) <= ILLEGAL_ADDRESS_EXCEPT_CODE;
          elsif rs1_to_sc = rs2_to_sc and vec_read_rs1_ID = '1' and vec_read_rs2_ID = '1' then               -- Set exception for same read access
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_wires(h)     := '1';    
            hdc_except_data_wire(h) <= READ_SAME_SCARTCHPAD_EXCEPT_CODE;    
          elsif (overflow_rs1_sc(h)(Addr_Width) = '1' and vec_read_rs1_ID = '1') or (overflow_rs2_sc(h)(Addr_Width) = '1' and  vec_read_rs2_ID = '1') then -- Set exception if reading overflows the scratchpad's address
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_wires(h)     := '1';    
            hdc_except_data_wire(h) <= SCRATCHPAD_OVERFLOW_EXCEPT_CODE;
          elsif overflow_rd_sc(h)(Addr_Width) = '1'  and vec_write_rd_ID = '1' then           -- Set exception if reading overflows the scratchpad's address, scalar writes are excluded
            hdc_except_condition_wires(h) := '1';
            hdc_taken_branch_wires(h)     := '1';    
            hdc_except_data_wire(h) <= SCRATCHPAD_OVERFLOW_EXCEPT_CODE;
          else
            if halt_hart(h) = '0' then
              nextstate_HDC(h) <= hdc_exec;
            else
              nextstate_HDC(h) <= hdc_halt_hart;
            end if;
            busy_hdc_internal_wires := '1';
          end if;

          if rs1_to_sc /= "100" and spm_rs1 = '1' and halt_hart(h) = '0' then
            hdc_sci_req(h)(to_integer(unsigned(rs1_to_sc))) <= '1';
            hdc_to_sc(h)(to_integer(unsigned(rs1_to_sc)))(0) <= '1';
            hdc_sc_read_addr(h)(0) <= RS1_Data_IE(Addr_Width-1 downto 0);
          end if;
          if rs2_to_sc /= "100" and spm_rs2 = '1' and rs1_to_Sc /= rs2_to_sc and halt_hart(h) = '0' then   -- Do not send a read request if the second operand accesses the same spm as the first, 
            hdc_sci_req(h)(to_integer(unsigned(rs2_to_sc))) <= '1';
            hdc_to_sc(h)(to_integer(unsigned(rs2_to_sc)))(1) <= '1';
            hdc_sc_read_addr(h)(1) <= RS2_Data_IE(Addr_Width-1 downto 0);
          end if;
        
         when hdc_halt_hart =>

           if halt_hart(h) = '0' then
             nextstate_HDC(h) <= hdc_exec;
           else
             nextstate_HDC(h) <= hdc_halt_hart;
           end if;
           busy_hdc_internal_wires := '1';

         when hdc_exec =>

           -----------------------------------------------------------------------------------------------------------------------
           --   ██████╗███╗   ██╗████████╗██████╗ ██╗         ██╗  ██╗ █████╗ ███╗   ██╗██████╗ ██╗     ██╗███╗   ██╗ ██████╗   --
           --  ██╔════╝████╗  ██║╚══██╔══╝██╔══██╗██║         ██║  ██║██╔══██╗████╗  ██║██╔══██╗██║     ██║████╗  ██║██╔════╝   --
           --  ██║     ██╔██╗ ██║   ██║   ██████╔╝██║         ███████║███████║██╔██╗ ██║██║  ██║██║     ██║██╔██╗ ██║██║  ███╗  --
           --  ██║     ██║╚██╗██║   ██║   ██╔══██╗██║         ██╔══██║██╔══██║██║╚██╗██║██║  ██║██║     ██║██║╚██╗██║██║   ██║  --
           --  ╚██████╗██║ ╚████║   ██║   ██║  ██║███████╗    ██║  ██║██║  ██║██║ ╚████║██████╔╝███████╗██║██║ ╚████║╚██████╔╝  --
           --   ╚═════╝╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝╚══════╝    ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝╚═╝╚═╝  ╚═══╝ ╚═════╝   --
           -----------------------------------------------------------------------------------------------------------------------

           ------ SMP BANK ENABLER --------------------------------------------------------------------------------------------------
           -- the following enables the appropriate banks to write the SIMD output, depending whether the result is a vector or a  --
           -- scalar, and adjusts the enabler appropriately based on the SIMD size. If the bytes to write are greater than SIMD*4  --
           -- then all banks are enabaled, else we perform the selective bank enabling as shown below under the 'elsif' clause     --
           --------------------------------------------------------------------------------------------------------------------------

           if (hdc_sci_wr_gnt(h) = '0' and wb_ready(h) = '1') then
             halt_hdc(h) <= '1';
             recover_state_wires(h) <= '1';
           elsif unsigned(HVSIZE_WRITE(h)) <= SIMD_RD_BYTES(h) then
             recover_state_wires(h) <= '0';
           end if;

           if vec_write_rd_HDC(h) = '1' and  hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h)))) = '1' then
             if ((unsigned(HVSIZE_WRITE(h)) >= (SIMD)*4+1) or 
                (perm_stage_1_en(h)='0' and perm_stage_2_en(h)='1')) then  -- 
               hdc_we_word(h) <= (others => '1');
             elsif  unsigned(HVSIZE_WRITE(h)) >= 1 then
               for i in 0 to SIMD-1 loop
                 if i <= to_integer(unsigned(HVSIZE_WRITE(h))-1)/4 then -- Four because of the number of bytes per word
                   if to_integer(unsigned(hdc_sc_write_addr(h)(SIMD_BITS+1 downto 0))/4 + i) < SIMD then
                     hdc_we_word(h)(to_integer(unsigned(hdc_sc_write_addr(h)(SIMD_BITS+1 downto 0))/4 + i)) <= '1';
                   elsif to_integer(unsigned(hdc_sc_write_addr(h)(SIMD_BITS+1 downto 0))/4 + i) >= SIMD then
                     hdc_we_word(h)(to_integer(unsigned(hdc_sc_write_addr(h)(SIMD_BITS+1 downto 0))/4 + i - SIMD)) <= '1';
                   end if;
                 end if;
               end loop;
             end if;
           elsif vec_write_rd_HDC(h) = '0' and  hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h)))) = '1' then
             hdc_we_word(h)(to_integer(unsigned(hdc_sc_write_addr(h)(SIMD_BITS+1 downto 0))/4)) <= '1';
           end if;
           -------------------------------------------------------------------------------------------------------------------------

           --------------------------- BUNDLING ---------------------------------
           if decoded_instruction_HDC_lat(h)(HVBUNDLE_bit_position)  = '1' then

            if bundle_stage_2_en(h) = '1' then 
              wb_ready(h) <= '1';
            elsif recover_state(h) = '1' then
              wb_ready(h) <= '1';  
            end if;

            if HVSIZE_READ(h) > (0 to Addr_Width => '0') then
              hdc_to_sc(h)(to_integer(unsigned(hdc_rs1_to_sc(h))))(0) <= '1';
              hdc_to_sc(h)(to_integer(unsigned(hdc_rs2_to_sc(h))))(1) <= '1';
              hdc_sci_req(h)(to_integer(unsigned(hdc_rs1_to_sc(h))))  <= '1';
              hdc_sci_req(h)(to_integer(unsigned(hdc_rs2_to_sc(h))))  <= '1';
              hdc_sc_read_addr(h)(0)  <= RS1_Data_IE_lat(h)(Addr_Width - 1 downto 0);
              hdc_sc_read_addr(h)(1)  <= RS2_Data_IE_lat(h)(Addr_Width - 1 downto 0);
              nextstate_HDC(h) <= hdc_exec;
              busy_hdc_internal_wires := '1';

            elsif HVSIZE_WRITE(h) = (0 to Addr_Width => '0') then
              nextstate_HDC(h) <= hdc_init;
            else
              nextstate_HDC(h) <= hdc_exec;
              busy_hdc_internal_wires := '1';
            end if;

            if wb_ready(h) = '1' then
              hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h)))) <= '1';
              hdc_sc_write_addr(h) <= RD_Data_IE_lat(h);
            end if;
            
           end if;
           ----------------------------------------------------------------------

           ----------------------------- BINDING --------------------------------
           if decoded_instruction_HDC_lat(h)(HVBIND_bit_position)    = '1' then 
             if bind_stage_2_en(h) = '1' then 
               wb_ready(h) <= '1';
             elsif recover_state(h) = '1' then
               wb_ready(h) <= '1';
             end if;
             if HVSIZE_READ(h) > (0 to Addr_Width => '0') then
               hdc_to_sc(h)(to_integer(unsigned(hdc_rs1_to_sc(h))))(0) <= '1';
               hdc_to_sc(h)(to_integer(unsigned(hdc_rs2_to_sc(h))))(1) <= '1';
               hdc_sci_req(h)(to_integer(unsigned(hdc_rs1_to_sc(h)))) <= '1';
               hdc_sci_req(h)(to_integer(unsigned(hdc_rs2_to_sc(h)))) <= '1';
               hdc_sc_read_addr(h)(1)  <= RS2_Data_IE_lat(h)(Addr_Width - 1 downto 0); 
               hdc_sc_read_addr(h)(0)  <= RS1_Data_IE_lat(h)(Addr_Width - 1 downto 0);
               nextstate_HDC(h) <= hdc_exec;
               busy_hdc_internal_wires := '1';
             elsif HVSIZE_WRITE(h) = (0 to Addr_Width => '0') then
              nextstate_HDC(h) <= hdc_init;
             else
              nextstate_HDC(h) <= hdc_exec;
              busy_hdc_internal_wires := '1';
             end if;
             if wb_ready(h) = '1' then
               hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h)))) <= '1';
               hdc_sc_write_addr(h) <= RD_Data_IE_lat(h);
             end if;
           end if;
           ----------------------------------------------------------------------------
          
           ----------------------------- SIMILARITY -----------------------------------
           if decoded_instruction_HDC_lat(h)(HVSIM_bit_position)    = '1' then 
             if sim_stage_2_en(h) = '1' then 
               wb_ready(h) <= '1';
             elsif recover_state(h) = '1' then
               wb_ready(h) <= '1';
             end if;
             if HVSIZE_READ(h) > (0 to Addr_Width => '0') then
              hdc_to_sc(h)(to_integer(unsigned(hdc_rs1_to_sc(h))))(0) <= '1';
              hdc_to_sc(h)(to_integer(unsigned(hdc_rs2_to_sc(h))))(1) <= '1';
              hdc_sci_req(h)(to_integer(unsigned(hdc_rs1_to_sc(h)))) <= '1';
              hdc_sci_req(h)(to_integer(unsigned(hdc_rs2_to_sc(h)))) <= '1';
              hdc_sc_read_addr(h)(0)  <= RS1_Data_IE_lat(h)(Addr_Width - 1 downto 0);
              hdc_sc_read_addr(h)(1)  <= RS2_Data_IE_lat(h)(Addr_Width - 1 downto 0);
              nextstate_HDC(h) <= hdc_exec;
              busy_hdc_internal_wires := '1';
            elsif HVSIZE_WRITE(h) = (0 to Addr_Width => '0') then
             nextstate_HDC(h) <= hdc_init;
            else
             nextstate_HDC(h) <= hdc_exec;
             busy_hdc_internal_wires := '1';
            end if;
            if wb_ready(h) = '1' then
              hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h)))) <= '1';
              hdc_sc_write_addr(h) <= RD_Data_IE_lat(h);
            end if;
          end if;
           -------------------------------------------------------------------------

           ---------------------------- CLIPPING -----------------------------------
           if decoded_instruction_HDC_lat(h)(HVCLIP_bit_position)  = '1' then
            if clip_stage_2_en(h) = '1' then 
              wb_ready(h) <= '1';
            elsif recover_state(h) = '1' then
              wb_ready(h) <= '1';  
            end if;
            if HVSIZE_READ(h) > (0 to Addr_Width => '0') then
              hdc_to_sc(h)(to_integer(unsigned(hdc_rs1_to_sc(h))))(0) <= '1';
              hdc_sci_req(h)(to_integer(unsigned(hdc_rs1_to_sc(h))))  <= '1';
              hdc_sc_read_addr(h)(0)  <= RS1_Data_IE_lat(h)(Addr_Width - 1 downto 0);
            end if;
            if HVSIZE_WRITE(h) > (0 to Addr_Width => '0') then
              nextstate_HDC(h) <= hdc_exec;
              busy_hdc_internal_wires := '1';
            end if;
            if wb_ready(h) = '1' then
              hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h))))    <= '1';
              hdc_sc_write_addr(h) <= RD_Data_IE_lat(h);
            end if;
          end if;
          ------------------------------------------------------------------------

          ------------------------------ PERMUTATION -----------------------------
          if decoded_instruction_HDC_lat(h)(HVPERM_bit_position)  = '1' then
            if perm_stage_2_en(h) = '1' then 
              wb_ready(h) <= '1';
            elsif recover_state(h) = '1' then
              wb_ready(h) <= '1';  
            end if;
            if HVSIZE_READ(h) > (0 to Addr_Width => '0') then
              
              hdc_to_sc(h)(to_integer(unsigned(hdc_rs1_to_sc(h))))(0) <= '1';
              hdc_sci_req(h)(to_integer(unsigned(hdc_rs1_to_sc(h))))  <= '1';

              if to_integer(unsigned(HVSIZE_READ(h))) <= SIMD_RD_Bytes(h) then
                hdc_sc_read_addr(h)(0)  <= first_perm_source_addr(h);
              else
                hdc_sc_read_addr(h)(0)  <= RS1_Data_IE_lat(h)(Addr_Width - 1 downto 0);
              end if;

            end if;
            if HVSIZE_WRITE(h) > (0 to Addr_Width => '0') or (perm_stage_1_en(h)='0' and perm_stage_2_en(h)='1') then
              nextstate_HDC(h) <= hdc_exec;
              busy_hdc_internal_wires := '1';
            end if;
            if wb_ready(h) = '1' then
              hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h))))    <= '1';  
              if (perm_stage_1_en(h) = '0' and perm_stage_2_en(h) = '1') then
                hdc_sc_write_addr(h) <= first_perm_dest_addr(h);
              else
                hdc_sc_write_addr(h) <= RD_Data_IE_lat(h);
              end if;
            end if;
          end if;
          -------------------------------------------------------------------------

          ------------------------------ SEARCH -----------------------------------
          if decoded_instruction_HDC_lat(h)(HVSEARCH_bit_position) = '1' then 
             if as_stage_3_en(h) = '1' then 
               wb_ready(h) <= '1';
             elsif recover_state(h) = '1' then
               wb_ready(h) <= '1';
             end if;
             if HVSIZE_READ(h) > (0 to Addr_Width => '0') then
              hdc_to_sc(h)(to_integer(unsigned(hdc_rs1_to_sc(h))))(0) <= '1';
              hdc_to_sc(h)(to_integer(unsigned(hdc_rs2_to_sc(h))))(1) <= '1';
              hdc_sci_req(h)(to_integer(unsigned(hdc_rs1_to_sc(h)))) <= '1';
              hdc_sci_req(h)(to_integer(unsigned(hdc_rs2_to_sc(h)))) <= '1';
              hdc_sc_read_addr(h)(0)  <= RS1_Data_IE_lat(h)(Addr_Width - 1 downto 0);
              hdc_sc_read_addr(h)(1)  <= RS2_Data_IE_lat(h)(Addr_Width - 1 downto 0);
             end if;
             if HVSIZE_WRITE(h) > (0 to Addr_Width => '0') then
              nextstate_HDC(h) <= hdc_exec;
              busy_hdc_internal_wires := '1';
             end if;
             if wb_ready(h) = '1' then
               hdc_sci_we(h)(to_integer(unsigned(hdc_rd_to_sc(h)))) <= '1';
               hdc_sc_write_addr(h) <= RD_Data_IE_lat(h);
             end if;
           end if;
           -------------------------------------------------------------------------

        when others =>
           null;
       end case;
     end if;
      
    busy_hdc_internal(h)    <= busy_hdc_internal_wires;
    hdc_except_condition(h) <= hdc_except_condition_wires(h);
    hdc_taken_branch(h)     <= hdc_taken_branch_wires(h);
      
  end process;

  ---------------------------------------------------------------------------------------------------------------------------------------------------------
  --  ██████╗ ██╗██████╗ ███████╗██╗     ██╗███╗   ██╗███████╗     ██████╗ ██████╗ ███╗   ██╗████████╗██████╗  ██████╗ ██╗     ██╗     ███████╗██████╗   --
  --  ██╔══██╗██║██╔══██╗██╔════╝██║     ██║████╗  ██║██╔════╝    ██╔════╝██╔═══██╗████╗  ██║╚══██╔══╝██╔══██╗██╔═══██╗██║     ██║     ██╔════╝██╔══██╗  --
  --  ██████╔╝██║██████╔╝█████╗  ██║     ██║██╔██╗ ██║█████╗      ██║     ██║   ██║██╔██╗ ██║   ██║   ██████╔╝██║   ██║██║     ██║     █████╗  ██████╔╝  --
  --  ██╔═══╝ ██║██╔═══╝ ██╔══╝  ██║     ██║██║╚██╗██║██╔══╝      ██║     ██║   ██║██║╚██╗██║   ██║   ██╔══██╗██║   ██║██║     ██║     ██╔══╝  ██╔══██╗  --
  --  ██║     ██║██║     ███████╗███████╗██║██║ ╚████║███████╗    ╚██████╗╚██████╔╝██║ ╚████║   ██║   ██║  ██║╚██████╔╝███████╗███████╗███████╗██║  ██║  --
  --  ╚═╝     ╚═╝╚═╝     ╚══════╝╚══════╝╚═╝╚═╝  ╚═══╝╚══════╝     ╚═════╝ ╚═════╝ ╚═╝  ╚═══╝   ╚═╝   ╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚══════╝╚══════╝╚═╝  ╚═╝  --
  ---------------------------------------------------------------------------------------------------------------------------------------------------------

  fsm_HDC_pipeline_controller : process(clk_i, rst_ni)
  begin
    if rst_ni = '0' then
      
      hdc_data_gnt_i_lat(h)    <= '0';

      ------------ BUNDLING ---------------
      bundle_stage_1_en(h)     <= '0';
      bundle_stage_2_en(h)     <= '0';
      -------------------------------------

      ------------ BINDING ----------------
      bind_stage_1_en(h)        <= '0';
      bind_stage_2_en(h)        <= '0';
      -------------------------------------

      ------------ SIMILARITY ---------------
      sim_stage_1_en(h)        <= '0';
      sim_stage_2_en(h)        <= '0';
      ---------------------------------------

      ------------ CLIPPING -----------------
      clip_stage_1_en(h)       <= '0';
      clip_stage_2_en(h)       <= '0';
      ---------------------------------------

      ------------ PERMUTATION ---------------
      perm_stage_1_en(h)       <= '0';
      perm_stage_2_en(h)       <= '0';
      ---------------------------------------

      ------------ SEARCH -------------------
      as_stage_1_en(h)         <= '0';
      as_stage_2_en(h)         <= '0';
      as_stage_3_en(h)         <= '0';
      ---------------------------------------


      state_HDC(h)             <= hdc_init;

    elsif rising_edge(clk_i) then

      hdc_data_gnt_i_lat(h)    <= hdc_data_gnt_i(h);

      ------------ BUNDLING ----------------
      bundle_stage_1_en(h)     <= hdc_data_gnt_i_lat(h) and bundle_en(h);
      bundle_stage_2_en(h)     <= bundle_stage_1_en(h);
      --------------------------------------

      ------------ BINDING -----------------
      bind_stage_1_en(h)       <= hdc_data_gnt_i_lat(h) and bind_en(h);
      bind_stage_2_en(h)       <= bind_stage_1_en(h);
      --------------------------------------
      
      ------------- SIMILARITY ----------------
      sim_stage_1_en(h)      <= hdc_data_gnt_i_lat(h) and sim_en(h);

      if (HVSIZE_READ_lat(h) = (0 to Addr_Width => '0')) then
        sim_stage_2_en(h)      <= sim_stage_1_en(h);
        else
        sim_stage_2_en(h)      <= '0';
      end if;
      -----------------------------------------

      ------------- CLIPPING ------------------
      clip_stage_1_en(h)      <= hdc_data_gnt_i_lat(h) and clip_en(h);
      
      if (clip_processed_bytes(h) = 3) or (HVSIZE_READ(h) = (0 to Addr_Width => '0')) then
        clip_stage_2_en(h)      <= clip_stage_1_en(h);
      else
        clip_stage_2_en(h)      <= '0';
      end if;
      ---------------------------------------

      ------------- PERMUTATION ----------------
      perm_stage_1_en(h)      <= hdc_data_gnt_i_lat(h) and perm_en(h);
      perm_stage_2_en(h)      <= perm_stage_1_en(h);
      ------------------------------------------

      -------------- SEARCH --------------------
      as_stage_1_en(h)        <= hdc_data_gnt_i_lat(h) and sim_en(h);
      as_stage_2_en(h)        <= as_stage_1_en(h);

      if (HVSIZE_READ_lat(h) = (0 to Addr_Width => '0')) then
        as_stage_3_en(h)      <= as_stage_2_en(h);
        else
        as_stage_3_en(h)      <= '0';
      end if;
      -------------------------------------------

      halt_hdc_lat(h)          <= halt_hdc(h);
      state_HDC(h)             <= nextstate_HDC(h);
      busy_hdc_internal_lat(h) <= busy_hdc_internal(h);
      SIMD_RD_BYTES(h)         <= SIMD_RD_BYTES_wire(h);
      hdc_except_data(h)       <= hdc_except_data_wire(h);

    end if;
  end process;

  HDC_FU_ENABLER_SYNC : process(clk_i, rst_ni)
  begin
    if rst_ni = '0' then

      ------- BUNDLING ----------
      bundle_en(h)         <= '0';
      bundle_en_pending(h) <= '0';
      ---------------------------
      
      ------- BINDING -----------
      bind_en(h)           <= '0';
      bind_en_pending(h)   <= '0';
      ---------------------------
      
      ------- SIMILARITY --------
      sim_en(h)           <= '0';
      sim_en_pending(h)   <= '0';  
      ---------------------------

      ------- CLIPPING ----------
      clip_en(h)          <= '0';
      clip_en_pending(h)  <= '0';
      ---------------------------

      ------- PERMUTATION -------
      perm_en(h)          <= '0';
      perm_en_pending(h)  <= '0';
      ---------------------------

      ------- SEARCH -----------
      as_en(h)            <= '0';
      as_en_pending(h)    <= '0';
      --------------------------

    elsif rising_edge(clk_i) then

      ------------ BUNDLING ------------ 
      bundle_en(h)           <= bundle_en_wire(h);
      bundle_en_pending(h)   <= bundle_en_pending_wire(h);
      ----------------------------------

      ------------ BINDING ---------------
      bind_en(h)           <= bind_en_wire(h); 
      bind_en_pending(h)   <= bind_en_pending_wire(h);
      ------------------------------------

      ------------ SIMILARITY ------------
      sim_en(h)           <= sim_en_wire(h); 
      sim_en_pending(h)   <= sim_en_pending_wire(h);  
      ------------------------------------

      ------------ CLIPPING --------------
      clip_en(h)          <= clip_en_wire(h);
      clip_en_pending(h)  <= clip_en_pending_wire(h);
      ------------------------------------

      ------------ PERMUTATION -----------
      perm_en(h)          <= perm_en_wire(h);
      perm_en_pending(h)  <= perm_en_pending_wire(h);
      ------------------------------------

      ------------ SEARCH ---------------
      as_en(h)            <= as_en_wire(h);
      as_en_pending(h)    <= as_en_pending_wire(h);
      -----------------------------------

    end if;

  end process;

end generate HDC_replicated;

  -------------------------------------------------------------------------------------------------------------------------------------------
  --  ███████╗██╗   ██╗     █████╗  ██████╗ ██████╗███████╗███████╗███████╗    ██╗  ██╗ █████╗ ███╗   ██╗██████╗ ██╗     ███████╗██████╗   --
  --  ██╔════╝██║   ██║    ██╔══██╗██╔════╝██╔════╝██╔════╝██╔════╝██╔════╝    ██║  ██║██╔══██╗████╗  ██║██╔══██╗██║     ██╔════╝██╔══██╗  --
  --  █████╗  ██║   ██║    ███████║██║     ██║     █████╗  ███████╗███████╗    ███████║███████║██╔██╗ ██║██║  ██║██║     █████╗  ██████╔╝  --
  --  ██╔══╝  ██║   ██║    ██╔══██║██║     ██║     ██╔══╝  ╚════██║╚════██║    ██╔══██║██╔══██║██║╚██╗██║██║  ██║██║     ██╔══╝  ██╔══██╗  --
  --  ██║     ╚██████╔╝    ██║  ██║╚██████╗╚██████╗███████╗███████║███████║    ██║  ██║██║  ██║██║ ╚████║██████╔╝███████╗███████╗██║  ██║  --
  --  ╚═╝      ╚═════╝     ╚═╝  ╚═╝ ╚═════╝ ╚═════╝╚══════╝╚══════╝╚══════╝    ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚══════╝╚══════╝╚═╝  ╚═╝  --
  -------------------------------------------------------------------------------------------------------------------------------------------

FU_HANDLER_MC : if multithreaded_accl_en = 0 generate
  DSP_FU_ENABLER_comb : process(all)
  begin
    for h in accl_range loop

      ---------- BUNDLING -------------
      bundle_en_wire(h)<= bundle_en(h);
      ---------------------------------

      ---------- BINDING --------------
      bind_en_wire(h)   <= bind_en(h); 
      ---------------------------------

      ---------- SIMILARITY -----------
      sim_en_wire(h)   <= sim_en(h);       
      ---------------------------------

      ---------- CLIPPING -------------
      clip_en_wire(h)  <= clip_en(h);
      ---------------------------------

      ---------- PERMUTATION ----------
      perm_en_wire(h)  <= perm_en(h);
      ---------------------------------
      
      ---------- SEARCH ---------------
      as_en_wire(h)    <= as_en(h);
      ---------------------------------

      halt_hart(h)     <= '0';
    

      ----------------- BUNDLING ---------------------------
      if bundle_en(h) = '1' and busy_hdc_internal(h) = '0' then
        bundle_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      ----------------- BINDING ----------------------------
      if bind_en(h) = '1' and busy_hdc_internal(h) = '0' then
        bind_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      ----------------- SIMILARITY -------------------------
      if sim_en(h) = '1' and busy_hdc_internal(h) = '0' then
        sim_en_wire(h) <= '0';                                  
      end if;
      ------------------------------------------------------

      ----------------- CLIPPING ---------------------------
      if clip_en(h) = '1' and busy_hdc_internal(h) = '0' then
        clip_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------
      
      ----------------- PERMUTATION ------------------------
      if perm_en(h) = '1' and busy_hdc_internal(h) = '0' then
        perm_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      -------------------- SEARCH --------------------------
      if as_en(h) = '1' and busy_hdc_internal(h) = '0' then
        as_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      if hdc_instr_req(h) = '1' or busy_hdc_internal_lat(h) = '1' then

        case state_HDC(h) is

          when hdc_init =>

            -- Set signals to enable correct virtual parallelism operation

            ------------------------ BUNDLING ---------------------------------
            if decoded_instruction_HDC(HVBUNDLE_bit_position)    = '1' then 
              bundle_en_wire(h) <= '1';
            -------------------------------------------------------------------

            ------------------------ BINDING ----------------------------------
            elsif decoded_instruction_HDC(HVBIND_bit_position) = '1' then
              bind_en_wire(h) <= '1';
            -------------------------------------------------------------------

            ---------------------- SIMILARITY ---------------------------------
            elsif decoded_instruction_HDC(HVSIM_bit_position)    = '1' then
              sim_en_wire(h) <= '1';                                         
            -------------------------------------------------------------------

            ---------------------- CLIPPING -----------------------------------
            elsif decoded_instruction_HDC(HVCLIP_bit_position  ) = '1' then
              clip_en_wire(h) <= '1';
            -------------------------------------------------------------------

            ---------------------- PERMUTATION --------------------------------
            elsif decoded_instruction_HDC(HVPERM_bit_position) = '1' then
              perm_en_wire(h) <= '1';
            -------------------------------------------------------------------

            ---------------------- SEARCH -------------------------------------
            elsif decoded_instruction_HDC(HVSEARCH_bit_position) = '1' then
              as_en_wire(h)  <= '1';
              sim_en_wire(h) <= '1';
            -------------------------------------------------------------------
            end if;
          when others =>
            null;
        end case;
      end if;
    end loop;
  end process;
end generate FU_HANDLER_MC;

FU_HANDLER_MT : if multithreaded_accl_en = 1 generate
  DSP_FU_ENABLER_comb : process(all)
  begin

    for h in accl_range loop

      ------------- BUNDLING -----------------------
      bundle_en_wire(h)               <= bundle_en(h);
      bundle_en_pending_wire(h)       <= bundle_en_pending(h);
      ----------------------------------------------

      ------------- BINDING ------------------------
      bind_en_wire(h)                 <= bind_en(h);
      bind_en_pending_wire(h)         <= bind_en_pending(h);
      ----------------------------------------------

      ------------- SIMILARITY ---------------------
      sim_en_wire(h)                 <= sim_en(h);  
      sim_en_pending_wire(h)         <= sim_en_pending(h);        
      ----------------------------------------------

      ------------- CLIPPING -----------------------
      clip_en_wire(h)                <= clip_en(h);
      clip_en_pending_wire(h)        <= clip_en_pending(h);
      ----------------------------------------------

      ------------- PERMUTATION --------------------
      perm_en_wire(h)                <= perm_en(h);
      perm_en_pending_wire(h)        <= perm_en_pending(h);
      ----------------------------------------------

      ------------- SEARCH --------------------------
      as_en_wire(h)                  <= as_en(h);
      as_en_pending_wire(h)          <= as_en_pending(h);
      -----------------------------------------------

      fu_req(h)                      <= (others => '0');
      halt_hart(h)                   <= '0';
      
      ----------------- BUNDLING ---------------------------
      if bundle_en(h) = '1' and busy_hdc_internal(h) = '0' then
        bundle_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      ----------------- BINDING ----------------------------
      if bind_en(h) = '1' and busy_hdc_internal(h) = '0' then
        bind_en_wire(h) <= '0';
      end if;
      ------------------------------------------------------

      ---------------- SIMILARITY --------------------------
      if sim_en(h) = '1' and busy_hdc_internal(h) = '0' then
        sim_en_wire(h) <= '0';   
      end if;                                 
      -----------------------------------------------------

      ----------------- CLIPPING --------------------------
      if clip_en(h) = '1' and busy_hdc_internal(h) = '0' then
        clip_en_wire(h) <= '0';
      end if;
      -----------------------------------------------------

      ----------------- PERMUTATION -----------------------
      if perm_en(h) = '1' and busy_hdc_internal(h) = '0' then
        perm_en_wire(h) <= '0';
      end if;
      -----------------------------------------------------

      ----------------- SEARCH ----------------------------
      if as_en(h) = '1' and busy_hdc_internal(h) = '0' then
        as_en_wire(h) <= '0';
      end if;
      -----------------------------------------------------

      if hdc_instr_req(h) = '1' or busy_hdc_internal_lat(h) = '1' then

        case state_HDC(h) is

          when hdc_init =>

            -- Set signals to enable correct virtual parallelism operation
            ------------------------ BUNDLING --------------------------------
            if decoded_instruction_HDC(HVBUNDLE_bit_position)    = '1' then
              if busy_bundle = '0' and bundle_en_pending = (accl_range => '0') then 
                bundle_en_wire(h) <= '1';
              else
                bundle_en_pending_wire(h) <= '1';
                halt_hart(h) <= '1';
                fu_req(h)(0) <= '1';
              end if;
            ------------------------------------------------------------------

            ---------------------- BINDING -----------------------------------
            elsif decoded_instruction_HDC(HVBIND_bit_position) = '1' then
              if busy_bind = '0' and bind_en_pending = (accl_range => '0') then 
                bind_en_wire(h) <= '1';
              else
                bind_en_pending_wire(h) <= '1';
                halt_hart(h) <= '1';
                fu_req(h)(2) <= '1';
              end if;
            ------------------------------------------------------------------

            ----------------------- SIMILARITY -------------------------------
            elsif decoded_instruction_HDC(HVSIM_bit_position)    = '1' then
              if busy_sim = '0' and sim_en_pending = (accl_range => '0') then 
                sim_en_wire(h) <= '1';                                        
              else
                sim_en_pending_wire(h) <= '1';                                  
                halt_hart(h) <= '1';
                fu_req(h)(5) <= '1';
              end if;
            -------------------------------------------------------------------

            ----------------------- CLIPPING -----------------------------------
            elsif decoded_instruction_HDC(HVCLIP_bit_position  ) = '1' then
              if busy_clip = '0' and clip_en_pending = (accl_range => '0') then 
                clip_en_wire(h) <= '1';
              else
                clip_en_pending_wire(h) <= '1';
                halt_hart(h) <= '1';
                fu_req(h)(6) <= '1';
              end if;
            -------------------------------------------------------------------

            ----------------------- PERMUTATION --------------------------------
            elsif decoded_instruction_HDC(HVPERM_bit_position) = '1' then
              if busy_perm = '0' and perm_en_pending = (accl_range => '0') then 
                perm_en_wire(h) <= '1';
              else
                perm_en_pending_wire(h) <= '1';
                halt_hart(h) <= '1';
                fu_req(h)(7) <= '1';
              end if;
            -------------------------------------------------------------------

            ----------------------- SEARCH -------------------------------------
            elsif decoded_instruction_HDC(HVSEARCH_bit_position) = '1' then
              if busy_as = '0' and busy_sim = '0' and as_en_pending = (accl_range => '0') and sim_en_pending = (accl_range => '0') then 
                as_en_wire(h) <= '1';
                sim_en_wire(h) <= '1';
              else
                as_en_pending_wire(h) <= '1';
                sim_en_pending_wire(h) <= '1'; 
                halt_hart(h) <= '1';
                fu_req(h)(8) <= '1';
                fu_req(h)(5) <= '1';
              end if;
            -------------------------------------------------------------------

            end if;

          when hdc_halt_hart =>
  
            if fu_gnt(h)(0) = '1' then
              bundle_en_wire(h) <= '1';
              bundle_en_pending_wire(h) <= '0';
            elsif bundle_en_pending(h) = '1' and fu_gnt(h)(0) = '0'  then
              halt_hart(h) <= '1';
            end if;

            if fu_gnt(h)(2) = '1' then
              bind_en_wire(h) <= '1';
              bind_en_pending_wire(h) <= '0';
            elsif bind_en_pending(h) = '1' and fu_gnt(h)(2) = '0'  then
              halt_hart(h) <= '1';
            end if;

            if fu_gnt(h)(5) = '1' then
              sim_en_wire(h) <= '1';
              sim_en_pending_wire(h) <= '0';
            elsif sim_en_pending(h) = '1' and fu_gnt(h)(5) = '0'  then
              halt_hart(h) <= '1';
            end if;

            if fu_gnt(h)(6) = '1' then
              clip_en_wire(h) <= '1';
              clip_en_pending_wire(h) <= '0';
            elsif clip_en_pending(h) = '1' and fu_gnt(h)(6) = '0'  then
              halt_hart(h) <= '1';
            end if;
            
            if fu_gnt(h)(7) = '1' then
              perm_en_wire(h) <= '1';
              perm_en_pending_wire(h) <= '0';
            elsif perm_en_pending(h) = '1' and fu_gnt(h)(7) = '0'  then
              halt_hart(h) <= '1';
            end if;

            if fu_gnt(h)(8) = '1' then
              as_en_wire(h) <= '1';
              as_en_pending_wire(h) <= '0';
            elsif as_en_pending(h) = '1' and fu_gnt(h)(8) = '0'  then
              halt_hart(h) <= '1';
            end if;

          when others =>
            null;
        end case;
      end if;
    end loop;
  end process;

  FU_Issue_Buffer_sync : process(clk_i, rst_ni)
  begin
    if rst_ni = '0' then
      fu_rd_ptr  <= (others => (others => '0'));
      fu_wr_ptr  <= (others => (others => '0'));
      fu_gnt     <= (others => (others => '0'));
    elsif rising_edge(clk_i) then
      fu_gnt <= fu_gnt_wire;
      for h in accl_range loop
        for i in 0 to 4 loop  -- Loop index 'i' is for the total number of different functional units (regardless what SIMD config is set)
          if fu_req(h)(i) = '1' then  -- if a reservation was made, to use a functional unit
            --to_integer(unsigned(fu_issue_buffer(i)(to_integer(unsigned(fu_wr_ptr(i)))))) <= h;  -- store the thread_ID in its corresponding buffer at the fu_wr_ptr position
            --fu_issue_buffer(to_integer(unsigned(fu_wr_ptr(i))))(i) <= std_logic_vector(unsigned(h));  -- store the thread_ID in its corresponding buffer at the fu_wr_ptr position
            fu_issue_buffer(i)(to_integer(unsigned(fu_wr_ptr(i))))  <= std_logic_vector(to_unsigned(h,TPS_CEIL));
            if unsigned(fu_wr_ptr(i)) = THREAD_POOL_SIZE - 2 then -- increment the pointer wr logic
              fu_wr_ptr(i) <= (others => '0');
            else
              fu_wr_ptr(i) <= std_logic_vector(unsigned(fu_wr_ptr(i)) + 1);
            end if;
          end if;
          case state_HDC(h) is
            when hdc_halt_hart =>
              if fu_gnt_en(h)(i) = '1' then
                if unsigned(fu_rd_ptr(i)) = THREAD_POOL_SIZE - 2 then  -- increment the read pointer
                  fu_rd_ptr(i) <= (others => '0');
                else
                  fu_rd_ptr(i) <= std_logic_vector(unsigned(fu_rd_ptr(i)) + 1);
                end if;
              end if;
            when others =>
             null;
          end case;
        end loop;
      end loop;
    end if;
  end process;

  FU_Issue_Buffer_comb : process(all)
  begin
    for h in accl_range loop
      fu_gnt_wire(h) <= (others => '0');
      fu_gnt_en(h)   <= (others => '0');

      ------------------- BUNDLING -----------------------
      if bundle_en_pending_wire(h) = '1' and busy_bundle_wire = '0' then
        fu_gnt_en(h)(0) <= '1';
      end if;
      ----------------------------------------------------

      ------------------- BINDING ------------------------
      if bind_en_pending_wire(h) = '1' and busy_bind_wire = '0' then
        fu_gnt_en(h)(2) <= '1';
      end if;
      ----------------------------------------------------

      -------------------- SIMILARITY --------------------
      if sim_en_pending_wire(h) = '1' and busy_sim_wire = '0' then
        fu_gnt_en(h)(5) <= '1';
      end if;
      ----------------------------------------------------

      -------------------- CLIPPING ----------------------
      if clip_en_pending_wire(h) = '1' and busy_clip_wire = '0' then
        fu_gnt_en(h)(6) <= '1';
      end if;
      ----------------------------------------------------

      -------------------- PERMUTATION -------------------
      if perm_en_pending_wire(h) = '1' and busy_perm_wire = '0' then
        fu_gnt_en(h)(7) <= '1';
      end if;
      ----------------------------------------------------

      -------------------- SEARCH -------------------------
      if as_en_pending_wire(h) = '1' and busy_sim_wire = '0' then
        fu_gnt_en(h)(8) <= '1';
      end if;
      -----------------------------------------------------
      
      case state_HDC(h) is
        when hdc_halt_hart =>
          for i in 0 to 4 loop 
            if fu_gnt_en(h)(i) = '1' then
              fu_gnt_wire(to_integer(unsigned(fu_issue_buffer(i)(to_integer(unsigned(fu_rd_ptr(i)))))))(i) <= '1'; -- give a grant to fu_gnt(h)(i), such that the 'h' index points to the thread in "fu_issue_buffer"
            end if;
          end loop;
        when others =>
          null;
      end case;
    end loop;
  end process;


  DSP_BUSY_FU_SYNC : process(clk_i, rst_ni)
  begin
    if rst_ni = '0' then
      
    elsif rising_edge(clk_i) then

      -------- BUNDLING ----------
      busy_bundle  <= busy_bundle_wire;
      ----------------------------

      -------- BINDING ------------
      busy_bind    <= busy_bind_wire;
      -----------------------------
      
      -------- SIMILARITY --------
      busy_sim    <= busy_sim_wire;
      ----------------------------

      -------- CLIPPING ----------
      busy_clip   <= busy_clip_wire;
      ----------------------------

      -------- PERMUTATION -------
      busy_perm   <= busy_perm_wire;
      ----------------------------

      -------- SEARCH -------------
      busy_as     <= busy_as_wire;
      -----------------------------

    end if;
  end process;

end generate FU_HANDLER_MT;

-------- BUNDLING ----------
busy_bundle_wire <= '1' when multithreaded_accl_en = 1 and bundle_en_wire   /= (accl_range => '0') else '0';
----------------------------

-------- BINDING -----------
busy_bind_wire <= '1' when multithreaded_accl_en = 1 and bind_en_wire   /= (accl_range => '0') else '0';
----------------------------

-------- SIMILARITY --------
busy_sim_wire <= '1' when multithreaded_accl_en = 1 and sim_en_wire   /= (accl_range => '0') else '0';
----------------------------

-------- CLIPPING ----------
busy_clip_wire <= '1' when multithreaded_accl_en = 1 and clip_en_wire /= (accl_range => '0') else '0';
----------------------------

------- PERMUTATION --------
busy_perm_wire <= '1' when multithreaded_accl_en = 1 and perm_en_wire /= (accl_range => '0') else '0';
----------------------------

-------- SEARCH ------------
busy_as_wire <= '1' when multithreaded_accl_en = 1 and as_en_wire /= (accl_range => '0') else '0';
----------------------------


  -----------------------------------------------------------------
  --  ███╗   ███╗ █████╗ ██████╗ ██████╗ ██╗███╗   ██╗ ██████╗   --
  --  ████╗ ████║██╔══██╗██╔══██╗██╔══██╗██║████╗  ██║██╔════╝   --
  --  ██╔████╔██║███████║██████╔╝██████╔╝██║██╔██╗ ██║██║  ███╗  --
  --  ██║╚██╔╝██║██╔══██║██╔═══╝ ██╔═══╝ ██║██║╚██╗██║██║   ██║  --
  --  ██║ ╚═╝ ██║██║  ██║██║     ██║     ██║██║ ╚████║╚██████╔╝  --
  --  ╚═╝     ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝     ╚═╝╚═╝  ╚═══╝ ╚═════╝   --
  -----------------------------------------------------------------

MULTICORE_OUT_MAPPER : if multithreaded_accl_en = 0 generate
MAPPER_replicated : for h in fu_range generate

  MAPPING_OUT_UNIT_comb : process(all)
  begin
      hdc_sc_data_write_wire_int(h)  <= (others => '0');
      hdc_sc_data_write_wire(h)      <= hdc_sc_data_write_wire_int(h);
      SIMD_RD_BYTES_wire(h)          <= SIMD*(Data_Width/8);

      if hdc_instr_req(h) = '1' or busy_hdc_internal_lat(h) = '1' then
        case state_HDC(h) is
          
          when hdc_init =>


          when hdc_exec =>

            --------------------------- BUNDLING ------------------------------
            if decoded_instruction_HDC_lat(h)(HVBUNDLE_bit_position)    = '1' then
              hdc_sc_data_write_wire_int(h) <= hdcu_out_bundle_results(h);
            end if;
            -------------------------------------------------------------------

            --------------------------- BINDING --------------------------------
            if (decoded_instruction_HDC_lat(h)(HVBIND_bit_position)    = '1' ) then
              hdc_sc_data_write_wire_int(h) <= hdcu_out_bind_results(h);
            end if;
            --------------------------------------------------------------------

            -------------------------- SIMILARITY -------------------------------
            if decoded_instruction_HDC_lat(h)(HVSIM_bit_position)      = '1' then
              hdc_sc_data_write_wire_int(h)(SIMILARITY_BITS -1 downto 0) <= hdcu_out_sim_results(h);              
            end if;
            ---------------------------------------------------------------------

            ------------------------ CLIPPING -----------------------------------
            if decoded_instruction_HDC_lat(h)(HVCLIP_bit_position  )   = '1' then
              for i in 0 to SIMD - 1 loop
                hdc_sc_data_write_wire_int(h)(SIMD_Width - Data_Width*i - 1 downto SIMD_Width - Data_Width*(i+1)) <= hdcu_out_clip_results(h)((Data_Width-1) + Data_Width*i downto Data_Width*i);
              end loop;
            end if;
            ----------------------------------------------------------------------

            ------------------------ PERMUTATION --------------------------------
            if decoded_instruction_HDC_lat(h)(HVPERM_bit_position)   = '1' then
              for i in 0 to SIMD - 1 loop
                hdc_sc_data_write_wire_int(h)(SIMD_Width - Data_Width*i - 1 downto SIMD_Width - Data_Width*(i+1)) <= hdcu_out_perm_results(h)((Data_Width-1) + Data_Width*i downto Data_Width*i);
              end loop;
              
            end if;
            ----------------------------------------------------------------------

            ------------------------ SEARCH -------------------------------------
            if decoded_instruction_HDC_lat(h)(HVSEARCH_bit_position) = '1' then
              hdc_sc_data_write_wire_int(h)(Data_Width - 1 downto 0) <= std_logic_vector(to_unsigned(temp_best_class_index(h), 32));
            end if;
            ----------------------------------------------------------------------

            if halt_hdc(h) = '0' and halt_hdc_lat(h) = '1' then
              hdc_sc_data_write_wire(h) <= hdc_sc_data_write_int(h);
            end if;
          when others =>
            null;
        end case;
      end if;
  end process;

end generate;
end generate;

MULTITHREAD_OUT_MAPPER : if multithreaded_accl_en = 1 generate
  MAPPING_OUT_UNIT_comb : process(all)
  begin
    for h in 0 to (ACCL_NUM - FU_NUM) loop
      hdc_sc_data_write_wire_int(h)  <= (others => '0');
      hdc_sc_data_write_wire(h)      <= hdc_sc_data_write_wire_int(h);
      SIMD_RD_BYTES_wire(h)          <= SIMD*(Data_Width/8);

      if hdc_instr_req(h) = '1' or busy_hdc_internal_lat(h) = '1' then
        
        case state_HDC(h) is
          
          when hdc_init =>

         
          when hdc_exec =>
            
            ------------------------- BUNDLING --------------------------------
            if decoded_instruction_HDC_lat(h)(HVBUNDLE_bit_position)   = '1' then
              hdc_sc_data_write_wire_int(h) <= hdcu_out_bundle_results(0);
            end if;
            -------------------------------------------------------------------

            ------------------------- BINDING --------------------------------
            if (decoded_instruction_HDC_lat(h)(HVBIND_bit_position)    = '1' ) then
              hdc_sc_data_write_wire_int(h) <= hdcu_out_bind_results(0);
            end if;
            -------------------------------------------------------------------
            
            ------------------------- SIMILARITY ------------------------------
            if decoded_instruction_HDC_lat(h)(HVSIM_bit_position)      = '1' then
              hdc_sc_data_write_wire_int(h)(SIMILARITY_BITS -1 downto 0) <= hdcu_out_sim_results(0);              
            end if;
            -------------------------------------------------------------------

            ------------------------- CLIPPING --------------------------------
            if decoded_instruction_HDC_lat(h)(HVCLIP_bit_position  )   = '1' then
              -- Rivolto l'ordine dei bit
              for i in 0 to SIMD - 1 loop
                hdc_sc_data_write_wire_int(h)(SIMD_Width - Data_Width*i - 1 downto SIMD_Width - Data_Width*(i+1)) <= hdcu_out_clip_results(0)((Data_Width-1) + Data_Width*i downto Data_Width*i);
              end loop;
            end if;
            -------------------------------------------------------------------

            ------------------------- PERMUTATION -----------------------------
            if decoded_instruction_HDC_lat(h)(HVPERM_bit_position)   = '1' then
              for i in 0 to SIMD - 1 loop
                hdc_sc_data_write_wire_int(h)(SIMD_Width - Data_Width*i - 1 downto SIMD_Width - Data_Width*(i+1)) <= hdcu_out_perm_results(h)((Data_Width-1) + Data_Width*i downto Data_Width*i);
              end loop;
            end if;
            -------------------------------------------------------------------

            ------------------------- SEARCH ----------------------------------
            if decoded_instruction_HDC_lat(h)(HVSEARCH_bit_position) = '1' then
              hdc_sc_data_write_wire_int(h)(Data_Width - 1 downto 0) <= std_logic_vector(to_unsigned(temp_best_class_index(0), 32));
            end if;
            -------------------------------------------------------------------

            if halt_hdc(h) = '0' and halt_hdc_lat(h) = '1' then
              hdc_sc_data_write_wire(h) <= hdc_sc_data_write_int(h);
            end if;

          when others =>
            null;

        end case;
      end if;
    end loop;
  end process;
end generate;


FU_replicated : for f in fu_range generate

  DSP_MAPPING_IN_UNIT_comb : process(all)
  
  variable h : integer;

  begin
    
    ------------------- BUNDLING ---------------------
    hdcu_in_bundle_operands(f)      <= (others => (others => '0'));
    -------------------------------------------------

    ------------------- BINDING --------------------
    hdcu_in_bind_operands(f)        <= (others => (others => '0'));
    --------------------------------------------------
    
    ------------------- SIMILARITY -------------------
    hdcu_in_sim_operands(f)         <= (others => (others => '0'));
    -------------------------------------------------

    ------------------- CLIPPING ---------------------
    hdcu_in_clip_operand_0(f)       <= (others => '0');
    -------------------------------------------------

    ------------------- PERMUTATION ------------------
    hdcu_in_perm_operand_0(f)       <= (others => '0');
    -------------------------------------------------

    for g in 0 to (ACCL_NUM - FU_NUM) loop

      if multithreaded_accl_en = 1 then
        h := g;  -- set the spm rd/wr ports equal to the "for-loop"
      elsif multithreaded_accl_en = 0 then
        h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
      end if;

      if hdc_instr_req(h) = '1' or busy_hdc_internal_lat(h) = '1' then
        case state_HDC(h) is
          
          when hdc_exec =>

            ---------------------------- BUNDLING --------------------------------------
            if decoded_instruction_HDC_lat(h)(HVBUNDLE_bit_position) = '1' then 
              hdcu_in_bundle_operands(f)(0) <= hdc_sc_data_read(h)(0);
              
              for i in 0 to SIMD - 1 loop
                for k in 0 to COUNTER_BITS - 1 loop
                  hdcu_in_bundle_operands(f)(1)(Data_Width*i + COUNTERS_NUMBER*(k+1) - 1 downto Data_Width*i + COUNTERS_NUMBER*k) <= hdc_sc_data_read(h)(1)(((Data_Width*(i+1)) - 1) - COUNTERS_NUMBER*k downto Data_Width*(i+1) - COUNTERS_NUMBER*(k+1));
                end loop;
              end loop;

            end if;
            -------------------------------------------------------------------------

            ----------------------------- BINDING -----------------------------------
            if decoded_instruction_HDC_lat(h)(HVBIND_bit_position)  = '1' then
              hdcu_in_bind_operands(f)(0) <= hdc_sc_data_read(h)(0);
              hdcu_in_bind_operands(f)(1) <= hdc_sc_data_read(h)(1);             
            end if;
            ------------------------------------------------------------------------

            ----------------------------- SIMILARITY -------------------------------
             if (decoded_instruction_HDC_lat(h)(HVSIM_bit_position)  = '1') or 
                (decoded_instruction_HDC_lat(h)(HVSEARCH_bit_position)  = '1') then
              
              hdcu_in_sim_operands(f)(0) <= hdc_sc_data_read(h)(0);
              
              if decoded_instruction_HDC_lat(h)(HVSEARCH_bit_position) = '1' and to_integer(unsigned(HVSIZE(h))) < SIMD_RD_BYTES_wire(h) then
                hdcu_in_sim_operands(f)(1)(to_integer(unsigned(HVSIZE(h)))*8 - 1 downto 0) <= hdc_sc_data_read(h)(1)(to_integer(unsigned(HVSIZE(h)))*8 - 1 downto 0); 
              else
                hdcu_in_sim_operands(f)(1) <= hdc_sc_data_read(h)(1);
              end if;

            end if;
            ------------------------------------------------------------------------

             --------------------------- CLIPPING ----------------------------------
             if decoded_instruction_HDC_lat(h)(HVCLIP_bit_position  ) = '1' then
              hdcu_in_clip_operand_0(f) <= hdc_sc_data_read(h)(0);
              hdcu_in_clip_operand_1(f) <= RS2_Data_IE_lat(h);
            end if;
            ------------------------------------------------------------------------
            
             --------------------------- PERMUTATION --------------------------------
             if decoded_instruction_HDC_lat(h)(HVPERM_bit_position) = '1' then
              for i in 0 to SIMD - 1 loop
                hdcu_in_perm_operand_0(f)(SIMD_Width-1 - Data_Width*i downto SIMD_Width - Data_Width*(i+1)) <= hdc_sc_data_read(h)(0)(Data_Width*(i+1) - 1 downto Data_Width*i);
              end loop;
              hdcu_in_perm_operand_1(f) <= RS2_Data_IE_lat(h);
            end if;
             ------------------------------------------------------------------------

          when others =>
            null;
        end case;
      end if;
    end loop;
  end process;

  ----------------------------------------------------------------------------------------------------------
  -- ██████╗ ██╗   ██╗███╗   ██╗██████╗ ██╗     ██╗███╗   ██╗ ██████╗     ██╗   ██╗███╗   ██╗██╗████████╗ --
  -- ██╔══██╗██║   ██║████╗  ██║██╔══██╗██║     ██║████╗  ██║██╔════╝     ██║   ██║████╗  ██║██║╚══██╔══╝ --
  -- ██████╔╝██║   ██║██╔██╗ ██║██║  ██║██║     ██║██╔██╗ ██║██║  ███╗    ██║   ██║██╔██╗ ██║██║   ██║    --
  -- ██╔══██╗██║   ██║██║╚██╗██║██║  ██║██║     ██║██║╚██╗██║██║   ██║    ██║   ██║██║╚██╗██║██║   ██║    --
  -- ██████╔╝╚██████╔╝██║ ╚████║██████╔╝███████╗██║██║ ╚████║╚██████╔╝    ╚██████╔╝██║ ╚████║██║   ██║    -- 
  -- ╚═════╝  ╚═════╝ ╚═╝  ╚═══╝╚═════╝ ╚══════╝╚═╝╚═╝  ╚═══╝ ╚═════╝      ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    --                                                                                                
  ----------------------------------------------------------------------------------------------------------

  fsm_HDCU_bundling : process(clk_i, rst_ni)
  variable h : integer;
  begin
    if rst_ni = '0' then
      
      counters                <= (others => (others => (others => (others => '0'))));
      bundle_offset           <= (others => (others=>0));
      bundle_processed_bytes  <= (others => 0);

    elsif rising_edge(clk_i) then
      
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then
          h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
        end if;

        -- Standard initializations for the HDCU bundling
        bundle_processed_bytes(h) <= 0;
        for i in 0 to SIMD - 1 loop
          bundle_offset (h)(i) <= 8*i;
        end loop;
        
        -- When we have the enable
        if bundle_en(h) = '1' and halt_hdc_lat(h) = '0' and (bundle_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
          for i in 0 to SIMD - 1 loop
            counters(h)(i)            <= counters_wire(h)(i);                  
            bundle_offset(h)(i)       <= bundle_offset(h)(i) + 8*SIMD;   -- By default, increment the offset by 8*SIMD
            bundle_processed_bytes(h) <= bundle_processed_bytes(h) + 1;  -- Increment the count

            -- When the completed a 32-bit word, we reset the offset
            if (bundle_processed_bytes(h) = Data_Width/COUNTERS_NUMBER - 1) then
              bundle_offset (h)(i)      <= 8*i;
              bundle_processed_bytes(h) <= 0;
            end if;
            
            for k in 0 to COUNTERS_NUMBER - 1 loop
              hdcu_out_bundle_results(f)(Data_Width*i + COUNTER_BITS*(k+1) - 1 downto Data_Width*i + COUNTER_BITS*k) <= counters_wire(h)(i)(k);
            end loop;

          end loop;
        end if;
      end loop;
    end if;
  end process;

  comb_HDCU_bundling: process(all)
  variable h : integer;
  begin
    
    counters_wire <= (others => (others => (others => (others => '0'))));
    
    for g in 0 to (ACCL_NUM - FU_NUM) loop
      
      if multithreaded_accl_en = 1 then
        h := g;  -- set the spm rd/wr ports equal to the "for-loop"
      elsif multithreaded_accl_en = 0 then
        h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
      end if;
      
      if bundle_en(h) = '1' and halt_hdc_lat(h) = '0' and (bundle_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
        for i in 0 to SIMD - 1 loop
          for k in 0 to COUNTERS_NUMBER - 1 loop
            counters_wire(f)(i)(k) <= hdcu_in_bundle_operands(f)(0)(Data_Width*i + COUNTER_BITS*(k+1) - 1 downto Data_Width*i + COUNTER_BITS*k);
            if hdcu_in_bundle_operands(f)(1)(bundle_offset(h)(i) + k) = '1' then
              counters_wire(f)(i)(k) <= std_logic_vector(unsigned(hdcu_in_bundle_operands(f)(0)(Data_Width*i + COUNTER_BITS*(k+1) - 1 downto Data_Width*i + COUNTER_BITS*k)) + 1);
            end if;
          end loop;
        end loop;
      end if;
    end loop;
  end process;
  
  --------------------------------------------------------------------------------------------
  -- ██████╗ ██╗███╗   ██╗██████╗ ██╗███╗   ██╗ ██████╗     ██╗   ██╗███╗   ██╗██╗████████╗ --
  -- ██╔══██╗██║████╗  ██║██╔══██╗██║████╗  ██║██╔════╝     ██║   ██║████╗  ██║██║╚══██╔══╝ --
  -- ██████╔╝██║██╔██╗ ██║██║  ██║██║██╔██╗ ██║██║  ███╗    ██║   ██║██╔██╗ ██║██║   ██║    --
  -- ██╔══██╗██║██║╚██╗██║██║  ██║██║██║╚██╗██║██║   ██║    ██║   ██║██║╚██╗██║██║   ██║    --
  -- ██████╔╝██║██║ ╚████║██████╔╝██║██║ ╚████║╚██████╔╝    ╚██████╔╝██║ ╚████║██║   ██║    --
  -- ╚═════╝ ╚═╝╚═╝  ╚═══╝╚═════╝ ╚═╝╚═╝  ╚═══╝ ╚═════╝      ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    --                                                                                                                                                            
  --------------------------------------------------------------------------------------------

  fsm_MUL_STAGE_1 : process(clk_i,rst_ni)
  variable h : integer;
  begin
    if rst_ni = '0' then
    elsif rising_edge(clk_i) then
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then
          h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
        end if;
        if halt_hdc_lat(h) = '0' then
          if bind_en(h) = '1' and (bind_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
          for i in 0 to SIMD-1 loop
              hdcu_out_bind_results(f)((Data_Width-1)+Data_Width*(i) downto Data_Width*(i))  <= std_logic_vector(unsigned(hdcu_in_bind_operands(f)(0)(31+32*(i)  downto 32*(i)) xor hdcu_in_bind_operands(f)(1)(31+32*(i)  downto 32*(i))));      
          end loop;
        end if;
        end if;
      end loop;
    end if;
  end process;
  
  ---------------------------------------------------------------------------------------------------------------
  -- ███████╗██╗███╗   ███╗██╗██╗      █████╗ ██████╗ ██╗████████╗██╗   ██╗    ██╗   ██╗███╗   ██╗██╗████████╗ --
  -- ██╔════╝██║████╗ ████║██║██║     ██╔══██╗██╔══██╗██║╚══██╔══╝╚██╗ ██╔╝    ██║   ██║████╗  ██║██║╚══██╔══╝ --
  -- ███████╗██║██╔████╔██║██║██║     ███████║██████╔╝██║   ██║    ╚████╔╝     ██║   ██║██╔██╗ ██║██║   ██║    --
  -- ╚════██║██║██║╚██╔╝██║██║██║     ██╔══██║██╔══██╗██║   ██║     ╚██╔╝      ██║   ██║██║╚██╗██║██║   ██║    --
  -- ███████║██║██║ ╚═╝ ██║██║███████╗██║  ██║██║  ██║██║   ██║      ██║       ╚██████╔╝██║ ╚████║██║   ██║    --
  -- ╚══════╝╚═╝╚═╝     ╚═╝╚═╝╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝   ╚═╝      ╚═╝        ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    --
  ---------------------------------------------------------------------------------------------------------------

  fsm_HDCU_sim : process(clk_i,rst_ni)
  variable h : integer;
  begin
    if rst_ni = '0' then    
      hdcu_out_sim_results <= (others => (others => '0'));
      sim_measure_reg      <= (others => (others => '0'));

    elsif rising_edge(clk_i) then
      
      hdcu_out_sim_results <= (others => (others => '0'));
      sim_measure_reg <= (others => (others => '0'));
      
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then -- Tutti gli acceleratori condividono la stessa FU
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then -- Ho una FU per acceleratore
          h := f;  -- set the spm rd/wr ports equal to the "for-generate",
        end if;


        if halt_hdc_lat(h) = '0' and sim_en(h) = '1' and (sim_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
          
          sim_measure_reg(f) <= sim_measure(f);
          if decoded_instruction_HDC_lat(h)(HVSEARCH_bit_position) = '1' then
            if to_integer(unsigned(AMSIZE_READ(h))) = 0 then
              sim_measure_reg <= (others => (others => '0'));
            end if;
          end if;

          hdcu_out_sim_results(f) <= hamming_distance_wire(h); 

        end if;
      end loop;
    end if;
  end process;

  HDCU_sim_comb : process(all)
  variable h : integer;
  variable accumulated_sim : std_logic_vector(SIMILARITY_BITS - 1 downto 0) := (others => '0');
  
  begin
    accumulated_sim       := (others => '0'); -- Reset accumulated_sim for each process activation
    hamming_distance_wire <= (others => (others => '0')); -- Reset hamming_distance_wire for each process activation
    sim_measure           <= (others => (others => '0')); -- Reset sim_measure for each process activation
    
    for g in 0 to (ACCL_NUM - FU_NUM) loop
      if multithreaded_accl_en = 1 then 
        h := g;  -- set the spm rd/wr ports equal to the "for-loop"
      elsif multithreaded_accl_en = 0 then 
        h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
      end if;
      
      if halt_hdc_lat(h) = '0' and sim_en(h) = '1' and (sim_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
          
          for i in 0 to SIMD-1 loop
            xor_out(f)((Data_Width-1) + Data_Width*i downto Data_Width*i)             <= std_logic_vector(unsigned(hdcu_in_sim_operands(f)(0)((Data_Width-1) + Data_Width*i downto Data_Width*i) xor hdcu_in_sim_operands(f)(1)((Data_Width-1) + Data_Width*i downto Data_Width*i)));      
            partial_sim_measure(f)((Data_Width-1) + Data_Width*i downto Data_Width*i) <= std_logic_vector(to_unsigned(popcount(xor_out(f)((Data_Width-1)+Data_Width*i downto Data_Width*i)), Data_Width));
            sim_measure(f)((Data_Width-1) + Data_Width*i downto Data_Width*i)         <= std_logic_vector(unsigned(sim_measure_reg(f)((Data_Width-1)+Data_Width*i downto Data_Width*i)) + unsigned(partial_sim_measure(f)((Data_Width-1) + Data_Width*(i) downto Data_Width*(i))));
            accumulated_sim := std_logic_vector(resize(unsigned(accumulated_sim) + unsigned(sim_measure(f)((Data_Width*(i+1)-1) downto Data_Width*i)), accumulated_sim'length));
          end loop;

          if decoded_instruction_HDC_lat(h)(HVSEARCH_bit_position) = '1' then
            if to_integer(unsigned(AMSIZE_READ(h))) = 0 then
              hamming_distance_wire(h) <= accumulated_sim;
            end if;
          else
            hamming_distance_wire(h) <= accumulated_sim;
          end if;
          
        end if;
    end loop;
  end process;

  --------------------------------------------------------------------------------------------------
  --  ██████╗██╗     ██╗██████╗ ██████╗ ██╗███╗   ██╗ ██████╗     ██╗   ██╗███╗   ██╗██╗████████╗ --
  -- ██╔════╝██║     ██║██╔══██╗██╔══██╗██║████╗  ██║██╔════╝     ██║   ██║████╗  ██║██║╚══██╔══╝ --
  -- ██║     ██║     ██║██████╔╝██████╔╝██║██╔██╗ ██║██║  ███╗    ██║   ██║██╔██╗ ██║██║   ██║    -- 
  -- ██║     ██║     ██║██╔═══╝ ██╔═══╝ ██║██║╚██╗██║██║   ██║    ██║   ██║██║╚██╗██║██║   ██║    -- 
  -- ╚██████╗███████╗██║██║     ██║     ██║██║ ╚████║╚██████╔╝    ╚██████╔╝██║ ╚████║██║   ██║    -- 
  --  ╚═════╝╚══════╝╚═╝╚═╝     ╚═╝     ╚═╝╚═╝  ╚═══╝ ╚═════╝      ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    --  
  --------------------------------------------------------------------------------------------------

  fsm_DSP_clip : process(clk_i,rst_ni)
  variable h : integer;
  begin
    if rst_ni = '0' then
      
      clip_processed_bytes <= (others => 0);

    elsif rising_edge(clk_i) then
  
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        
        if multithreaded_accl_en = 1 then
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then 
          h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
        end if;
        
        clip_processed_bytes(h) <= 0;

        if halt_hdc_lat(h) = '0' and clip_en(h) = '1' and (clip_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
          
          for i in 0 to SIMD-1 loop

            if (clip_processed_bytes(h) = 3) then 
              clip_processed_bytes(h) <= 0;
            else
              clip_processed_bytes(h) <= clip_processed_bytes(h) + 1;
            end if;

            for k in 0 to COUNTERS_NUMBER - 1 loop

              if to_integer(unsigned(hdcu_in_clip_operand_0(f)(Data_Width*i + COUNTER_BITS*(k+1) - 1 downto Data_Width*i + COUNTER_BITS*k))) <= to_integer(unsigned('0'& hdcu_in_clip_operand_1(f)(31 downto 1))) then
                hdcu_out_clip_results(f)(SIMD_Width - COUNTERS_NUMBER*SIMD*clip_processed_bytes(h) - (i+1)*COUNTERS_NUMBER + k) <= '0';
              else
                hdcu_out_clip_results(f)(SIMD_Width - COUNTERS_NUMBER*SIMD*clip_processed_bytes(h) - (i+1)*COUNTERS_NUMBER + k) <= '1';
              end if;

            end loop;

          end loop;

        end if;
      end loop;
    end if;
  end process;

  -------------------------------------------------------------------------------------------------------------------------------------
  -- ██████╗ ███████╗██████╗ ███╗   ███╗██╗   ██╗████████╗ █████╗ ████████╗██╗ ██████╗ ███╗   ██╗    ██╗   ██╗███╗   ██╗██╗████████╗ --
  -- ██╔══██╗██╔════╝██╔══██╗████╗ ████║██║   ██║╚══██╔══╝██╔══██╗╚══██╔══╝██║██╔═══██╗████╗  ██║    ██║   ██║████╗  ██║██║╚══██╔══╝ --
  -- ██████╔╝█████╗  ██████╔╝██╔████╔██║██║   ██║   ██║   ███████║   ██║   ██║██║   ██║██╔██╗ ██║    ██║   ██║██╔██╗ ██║██║   ██║    --
  -- ██╔═══╝ ██╔══╝  ██╔══██╗██║╚██╔╝██║██║   ██║   ██║   ██╔══██║   ██║   ██║██║   ██║██║╚██╗██║    ██║   ██║██║╚██╗██║██║   ██║    --
  -- ██║     ███████╗██║  ██║██║ ╚═╝ ██║╚██████╔╝   ██║   ██║  ██║   ██║   ██║╚██████╔╝██║ ╚████║    ╚██████╔╝██║ ╚████║██║   ██║    --
  -- ╚═╝     ╚══════╝╚═╝  ╚═╝╚═╝     ╚═╝ ╚═════╝    ╚═╝   ╚═╝  ╚═╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝     ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    --
  -------------------------------------------------------------------------------------------------------------------------------------                                                                                                                               
  
  -- This process can be removed and substituted by a variable in the sync process, no difference in hardware (actually separated just for debug reasons)
  fsm_HDC_permcomb : process(all)
  variable h : integer;
  begin
    for g in 0 to (ACCL_NUM - FU_NUM) loop
      if multithreaded_accl_en = 1 then 
        h := g;  -- set the spm rd/wr ports equal to the "for-loop"
      elsif multithreaded_accl_en = 0 then 
        h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
      end if;
      new_perm_offset(h) <= 0;
      if (to_integer(unsigned(HVSIZE(h))) < SIMD_RD_BYTES_wire(h)) then
        new_perm_offset(h) <= (SIMD_RD_BYTES_wire(h) - to_integer(unsigned(HVSIZE(h))))*8;
      elsif ((to_integer(unsigned(HVSIZE_READ_lat(h))) /= 0 and 
              to_integer(unsigned(HVSIZE_READ_lat(h))) <= SIMD_RD_BYTES_wire(h))) then
        new_perm_offset(h) <= (SIMD_RD_BYTES_wire(h) - to_integer(unsigned(HVSIZE_READ_lat(h))))*8;
      end if;
    end loop;
  end process;

  shift_amount(f) <= to_integer(unsigned(hdcu_in_perm_operand_1(f)));

  fsm_HDCU_perm : process(clk_i,rst_ni)
  variable h : integer;
  
  begin
    if rst_ni = '0' then
      buffer_reg               <= (others => (others => '0'));
      hdcu_out_perm_results(f) <= (others => '0');
    elsif rising_edge(clk_i) then
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then 
          h := f;  -- set the spm rd/wr ports equal to the "for-generate" 
        end if;
        if halt_hdc_lat(h) = '0' and perm_en(h) = '1' and (perm_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
          
          -- Default Assignment for the hdcu_out_perm_results and buffer_reg;
          buffer_reg(f)(Data_Width-1 downto Data_Width - shift_amount(f))            <= hdcu_in_perm_operand_0(f)(shift_amount(f)-1 downto 0);

          if perm_stage_2_en(h) = '1' then
            hdcu_out_perm_results(f)(SIMD_Width-1 downto SIMD_Width - shift_amount(f)) <= buffer_reg(f)(Data_Width-1 downto Data_Width - shift_amount(f));  
            hdcu_out_perm_results(f)(SIMD_Width-1 - shift_amount(f) downto 0)          <= hdcu_in_perm_operand_0(f)(SIMD_Width-1 downto shift_amount(f)); 
          end if;
          
          -- If the number of bytes to permute is less than the SIMD_RD_BYTES_wire, we update the assignment properly;
          if (to_integer(unsigned(HVSIZE_READ_lat(h))) /= 0 and 
              to_integer(unsigned(HVSIZE_READ_lat(h))) <= SIMD_RD_BYTES_wire(h)) or 
              to_integer(unsigned(HVSIZE(h))) < SIMD_RD_BYTES_wire(h) then

              hdcu_out_perm_results(f)(SIMD_Width-1 downto new_perm_offset(h)) <= buffer_reg(f)(Data_Width -1 downto Data_Width - shift_amount(f)) &
                                                                                  hdcu_in_perm_operand_0(f)(SIMD_Width-1 downto shift_amount(f) + new_perm_offset(h));  
              buffer_reg(f)(Data_Width-1 downto Data_Width - shift_amount(f))  <= hdcu_in_perm_operand_0(f)(shift_amount(f) + new_perm_offset(h)-1 downto new_perm_offset(h)); 
          end if;

          if (to_integer(unsigned(HVSIZE_READ_lat(h))) = 0) then
            buffer_reg(f) <= (others => '0');
          end if;
        end if;
      end loop;
    end if;
  end process;

 --------------------------------------------------------------------------------------------------------------------------
 --    █████╗ ███████╗███████╗       ███████╗███████╗ █████╗ ██████╗  ██████╗██╗  ██╗    ██╗   ██╗███╗   ██╗██╗████████╗ --
 --   ██╔══██╗██╔════╝██╔════╝       ██╔════╝██╔════╝██╔══██╗██╔══██╗██╔════╝██║  ██║    ██║   ██║████╗  ██║██║╚══██╔══╝ --
 --   ███████║███████╗███████╗       ███████╗█████╗  ███████║██████╔╝██║     ███████║    ██║   ██║██╔██╗ ██║██║   ██║    -- 
 --   ██╔══██║╚════██║╚════██║       ╚════██║██╔══╝  ██╔══██║██╔══██╗██║     ██╔══██║    ██║   ██║██║╚██╗██║██║   ██║    --
 --   ██║  ██║███████║███████║██╗    ███████║███████╗██║  ██║██║  ██║╚██████╗██║  ██║    ╚██████╔╝██║ ╚████║██║   ██║    --
 --   ╚═╝  ╚═╝╚══════╝╚══════╝╚═╝    ╚══════╝╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝╚═╝  ╚═╝     ╚═════╝ ╚═╝  ╚═══╝╚═╝   ╚═╝    --
 --------------------------------------------------------------------------------------------------------------------------
  
  fsm_HDCU_as : process(clk_i,rst_ni)          
  variable h : integer;

  begin
    if rst_ni = '0' then
      temp_best_sim_class  <= (others => 8192); -- Worst case scenario
      temp_best_class_index <= (others => 0);

    elsif rising_edge(clk_i) then
      for g in 0 to (ACCL_NUM - FU_NUM) loop
        if multithreaded_accl_en = 1 then 
          h := g;  -- set the spm rd/wr ports equal to the "for-loop"
        elsif multithreaded_accl_en = 0 then 
          h := f;  -- set the spm rd/wr ports equal to the "for-generate",
        end if;

        if halt_hdc_lat(h) = '0' and as_en(h) = '1' and (as_stage_1_en(h) = '1' or recover_state_wires(h) = '1') then
        
          if to_integer(unsigned(AMSIZE_READ(h))) = 0  and to_integer(unsigned(HVSIZE_READ_lat(h))) /= 0 then 
            if temp_best_sim_class(h) > to_integer(unsigned(hamming_distance_wire(h)))then
              temp_best_sim_class(h)   <= to_integer(unsigned(hamming_distance_wire(h)));
              temp_best_class_index(h) <= class_index(h);
            end if;
          end if;
        end if;
      end loop;
    end if;
  end process;

end generate FU_replicated;
end HDC;
--------------------------------- END of HDC architecture ------------------------------------

