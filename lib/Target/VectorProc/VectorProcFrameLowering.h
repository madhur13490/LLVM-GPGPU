//===-- VectorProcFrameLowering.h - Define frame lowering for VectorProc --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//

#ifndef VECTORPROC_FRAMEINFO_H
#define VECTORPROC_FRAMEINFO_H

#include "VectorProc.h"
#include "VectorProcSubtarget.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
  class VectorProcSubtarget;

class VectorProcFrameLowering : public TargetFrameLowering {
  const VectorProcSubtarget &SubTarget;
public:
	explicit VectorProcFrameLowering(const VectorProcSubtarget &ST)
		: TargetFrameLowering(TargetFrameLowering::StackGrowsDown,
			64, 0, 64), SubTarget(ST) {}

	/// emitProlog/emitEpilog - These methods insert prolog and epilog code into
	/// the function.
	void emitPrologue(MachineFunction &MF) const;
	void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const;
	void eliminateCallFramePseudoInstr(MachineFunction &MF,
		MachineBasicBlock &MBB,
		MachineBasicBlock::iterator I) const;
	bool hasFP(const MachineFunction &MF) const;
	bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
		MachineBasicBlock::iterator MI,
		const std::vector<CalleeSavedInfo> &CSI,
		const TargetRegisterInfo *TRI) const;

	virtual bool targetHandlesStackFrameRounding() const {
		return true;
	}
};

} // End llvm namespace

#endif
