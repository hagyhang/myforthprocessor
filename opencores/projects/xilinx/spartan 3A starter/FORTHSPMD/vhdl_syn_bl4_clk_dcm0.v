-------------------------------------------------------------------------------
-- Copyright (c) 2005 Xilinx, Inc.
-- This design is confidential and proprietary of Xilinx, All Rights Reserved.
-------------------------------------------------------------------------------
--   ____  ____
--  /   /\/   /
-- /___/  \  /   Vendor		    : Xilinx
-- \   \   \/    Version	    : $Name: mig_v1_7_b7 $
--  \   \        Application	    : MIG
--  /   /        Filename	    : vhdl_syn_bl4_clk_dcm.vhd
-- /___/   /\    Date Last Modified : $Date: 2007/02/10 08:06:36 $
-- \   \  /  \   Date Created	    : Mon May 2 2005
--  \___\/\___\
--
-- Device      : Spartan-3/3A
-- Design Name : DDR2 SDRAM
-- Purpose     :     This module generates the system clock for controller block
--                    This also generates the recapture clock, clock for the
--                    Refresh counter and also for the data path
-------------------------------------------------------------------------------

library ieee;
library UNISIM;
use ieee.std_logic_1164.all;
use UNISIM.VCOMPONENTS.all;
entity vhdl_syn_bl4_clk_dcm is
  port(
    input_clk : in  std_logic;
    rst       : in  std_logic;
    clk       : out std_logic;
    clk90     : out std_logic;
    dcm_lock  : out std_logic
    );
end vhdl_syn_bl4_clk_dcm;

architecture arc of vhdl_syn_bl4_clk_dcm is

  signal clk0dcm   : std_logic;
  signal clk90dcm  : std_logic;
  signal clk0_buf  : std_logic;
  signal clk90_buf : std_logic;
  signal vcc       : std_logic;
  signal gnd       : std_logic;
  signal dcm1_lock : std_logic;

begin

  vcc   <= '1';
  gnd   <= '0';
  clk   <= clk0_buf;
  clk90 <= clk90_buf;

  DCM_INST1 : DCM
    generic map(
      DLL_FREQUENCY_MODE    => "LOW",
      DUTY_CYCLE_CORRECTION => true
      )

    port map (
      CLKIN    => input_clk,
      CLKFB    => clk0_buf,
      DSSEN    => gnd,
      PSINCDEC => gnd,
      PSEN     => gnd,
      PSCLK    => gnd,
      RST      => rst,
      CLK0     => clk0dcm,
      CLK90    => clk90dcm,
      CLK180   => open,
      CLK270   => open,
      CLK2X    => open,
      CLK2X180 => open,
      CLKDV    => open,
      CLKFX    => open,
      CLKFX180 => open,
      LOCKED   => dcm1_lock,
      PSDONE   => open,
      STATUS   => open
      );

  BUFG_CLK0 : BUFGMUX
    port map (
      O  => clk0_buf,
      I0 => clk0dcm,
      I1 => clk0dcm,
      S  => '0'
      );
  BUFG_CLK90 : BUFGMUX
    port map (
      O  => clk90_buf,
      I0 => clk90dcm,
      I1 => clk90dcm,
      S  => '0'
      );

  dcm_lock <= dcm1_lock;
end arc;
