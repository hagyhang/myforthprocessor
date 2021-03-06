-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : %module_name.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/06 08:09:03 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     : This module has the instantiations infrastructure_top and
--							 main modules.
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use UNISIM.VCOMPONENTS.all;

entity vhdl_syn_bl40 is
  port (
    cntrl0_DDR2_DQ                       : inout  std_logic_vector(15 downto 0);
    cntrl0_DDR2_A                        : out  std_logic_vector(12 downto 0);
    cntrl0_DDR2_BA                       : out  std_logic_vector(1 downto 0);
    cntrl0_DDR2_CK                       : out std_logic;
    cntrl0_DDR2_CK_N                     : out std_logic;
    cntrl0_DDR2_CKE                      : out std_logic;
    cntrl0_DDR2_CS_N                     : out std_logic;
    cntrl0_DDR2_RAS_N                    : out std_logic;
    cntrl0_DDR2_CAS_N                    : out std_logic;
    cntrl0_DDR2_WE_N                     : out std_logic;
    cntrl0_DDR2_ODT                      : out std_logic;
    cntrl0_DDR2_DM                       : out  std_logic_vector(1 downto 0);
    cntrl0_rst_dqs_div_in                : in std_logic;
    cntrl0_rst_dqs_div_out               : out std_logic;
    SYS_CLK                              : in std_logic;
    cntrl0_led_error_output1             : out std_logic;
    cntrl0_data_valid_out                : out std_logic;
    reset_in_n                           : in std_logic;
    cntrl0_DDR2_DQS                      : inout  std_logic_vector(1 downto 0);
    cntrl0_DDR2_DQS_N                    : inout  std_logic_vector(1 downto 0)
    );
end vhdl_syn_bl40;

architecture arc_mem_interface_top of vhdl_syn_bl40 is

component vhdl_syn_bl4_main_0  port (
   DDR2_DQ                        : inout  std_logic_vector(15 downto 0);
   DDR2_A                         : out  std_logic_vector(12 downto 0);
   DDR2_BA                        : out  std_logic_vector(1 downto 0);
   DDR2_CK                        : out std_logic;
   DDR2_CK_N                      : out std_logic;
   DDR2_CKE                       : out std_logic;
   DDR2_CS_N                      : out std_logic;
   DDR2_RAS_N                     : out std_logic;
   DDR2_CAS_N                     : out std_logic;
   DDR2_WE_N                      : out std_logic;
   DDR2_ODT                       : out std_logic;
   DDR2_DM                        : out  std_logic_vector(1 downto 0);
   rst_dqs_div_in                 : in std_logic;
   rst_dqs_div_out                : out std_logic;
   led_error_output1              : out std_logic;
   data_valid_out                 : out std_logic;
   DDR2_DQS                       : inout  std_logic_vector(1 downto 0);
   DDR2_DQS_N                     : inout  std_logic_vector(1 downto 0);
   clk_int                        : in std_logic;   
   clk90_int                      : in std_logic;   
   wait_200us                     : in std_logic;   
   sys_rst                        : in std_logic;   
   sys_rst90                      : in std_logic;   
   sys_rst180                     : in std_logic;   
   delay_sel_val                  : in std_logic_vector(4 downto 0)

   );
end component;

  component vhdl_syn_bl4_infrastructure_top
    port (
      SYS_CLK                        : in std_logic;
      reset_in_n                     : in std_logic;
      wait_200us                     : out std_logic;
      delay_sel_val1_val             : out std_logic_vector(4 downto 0);
      sys_rst_val                    : out std_logic;
      sys_rst90_val                  : out std_logic;
      clk_int_val                    : out std_logic;
      clk90_int_val                  : out std_logic;
      sys_rst180_val                 : out std_logic
      );
  end component;

  signal sys_rst      : std_logic;
  signal wait_200us   : std_logic;
  signal sys_rst90    : std_logic;
  signal sys_rst180   : std_logic;
  signal clk_0        : std_logic;
  signal clk90_0      : std_logic;
  signal delay_sel    : std_logic_vector(4 downto 0);

begin

main_00 :    vhdl_syn_bl4_main_0 port map (
   DDR2_DQ                        => cntrl0_DDR2_DQ,
   DDR2_A                         => cntrl0_DDR2_A,
   DDR2_BA                        => cntrl0_DDR2_BA,
   DDR2_CK                        => cntrl0_DDR2_CK,
   DDR2_CK_N                      => cntrl0_DDR2_CK_N,
   DDR2_CKE                       => cntrl0_DDR2_CKE,
   DDR2_CS_N                      => cntrl0_DDR2_CS_N,
   DDR2_RAS_N                     => cntrl0_DDR2_RAS_N,
   DDR2_CAS_N                     => cntrl0_DDR2_CAS_N,
   DDR2_WE_N                      => cntrl0_DDR2_WE_N,
   DDR2_ODT                       => cntrl0_DDR2_ODT,
   DDR2_DM                        => cntrl0_DDR2_DM,
   rst_dqs_div_in                 => cntrl0_rst_dqs_div_in,
   rst_dqs_div_out                => cntrl0_rst_dqs_div_out,
   led_error_output1              => cntrl0_led_error_output1,
   data_valid_out                 => cntrl0_data_valid_out,
   DDR2_DQS                       => cntrl0_DDR2_DQS,
   DDR2_DQS_N                     => cntrl0_DDR2_DQS_N,
   wait_200us                      => wait_200us,
   delay_sel_val                   => delay_sel,
   clk_int                         => clk_0,
   clk90_int                       => clk90_0,
   sys_rst                         => sys_rst,
   sys_rst90                       => sys_rst90,
   sys_rst180                      => sys_rst180
   );


  infrastructure_top0 : vhdl_syn_bl4_infrastructure_top
    port map (
      wait_200us                     => wait_200us,
      delay_sel_val1_val             => delay_sel,
      clk_int_val                    => clk_0,
      clk90_int_val                  => clk90_0,
      sys_rst_val                    => sys_rst,
      sys_rst90_val                  => sys_rst90,
      sys_rst180_val                 => sys_rst180,
      SYS_CLK                        => SYS_CLK,
      reset_in_n                     => reset_in_n
      );


end arc_mem_interface_top;
