/*
 * @(#)InstructionDecoder.java	1.2 03/01/23 11:16:20
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.asm.sparc;

import sun.jvm.hotspot.asm.*;

// basic instruction decoder class
abstract class InstructionDecoder implements /* imports */ SPARCOpcodes , RTLDataTypes, RTLOperations {
    // some general utility functions - for format 2, 3 & 3A instructions

    static int extractSignedIntFromNBits(int value, int num_bits) {
        return (value << (32 - num_bits)) >> (32 - num_bits);
    }

    // "rs1"
    static int getSourceRegister1(int instruction) {
        return (instruction & RS1_MASK) >>> RS1_START_BIT;
    }

    // "rs2"
    static int getSourceRegister2(int instruction) {
        return (instruction & RS2_MASK);
    }

    // "rd"
    static int getDestinationRegister(int instruction) {
        return (instruction & RD_MASK) >>> RD_START_BIT;
    }

    static int getConditionCode(int instruction) {
        return (instruction & CONDITION_CODE_MASK) >>> CONDITION_CODE_START_BIT;
    }

    // "i" bit - used to indicate whether second component in an indirect
    // address is immediate value or a register. (format 3 & 3A).

    static boolean isIBitSet(int instruction) {
        return (instruction & I_MASK) != 0;
    }

    static ImmediateOrRegister getOperand2(int instruction) {
        boolean iBit = isIBitSet(instruction);
        ImmediateOrRegister operand2 = null;
        if (iBit) {
           operand2 = new Immediate(new Short((short)extractSignedIntFromNBits(instruction, 13)));
        } else {
           operand2 = SPARCRegisters.getRegister(getSourceRegister2(instruction));
        }
        return operand2;
    }

    // "opf" - floating point operation code.
    static int getOpf(int instruction) {
        return (instruction & OPF_MASK) >>> OPF_START_BIT;
    }

    abstract Instruction decode(int instruction, SPARCInstructionFactory factory);
}

