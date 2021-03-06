//===-- VectorProcInstrInfo.cpp - VectorProc Instruction Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the VectorProc implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "VectorProcInstrInfo.h"
#include "VectorProc.h"
#include "VectorProcMachineFunctionInfo.h"
#include "VectorProcSubtarget.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_CTOR
#include "VectorProcGenInstrInfo.inc"

using namespace llvm;

VectorProcInstrInfo::VectorProcInstrInfo(VectorProcSubtarget &ST)
  : VectorProcGenInstrInfo(VectorProc::ADJCALLSTACKDOWN, VectorProc::ADJCALLSTACKUP),
    RI(ST, *this), Subtarget(ST) {
}

/// isLoadFromStackSlot - If the specified machine instruction is a direct
/// load from a stack slot, return the virtual or physical register number of
/// the destination along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than loading from the stack slot.
unsigned VectorProcInstrInfo::isLoadFromStackSlot(const MachineInstr *MI,
                                             int &FrameIndex) const {
	if (MI->getOpcode() == VectorProc::LWi || MI->getOpcode() == VectorProc::LWf
		|| MI->getOpcode() == VectorProc::BLOCK_LOADI
		|| MI->getOpcode() == VectorProc::BLOCK_LOADF) {
		if (MI->getOperand(1).isFI() && MI->getOperand(2).isImm() &&
			MI->getOperand(2).getImm() == 0) {
			FrameIndex = MI->getOperand(1).getIndex();
			return MI->getOperand(0).getReg();
		}
	}

	return 0;
}

/// isStoreToStackSlot - If the specified machine instruction is a direct
/// store to a stack slot, return the virtual or physical register number of
/// the source reg along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than storing to the stack slot.
unsigned VectorProcInstrInfo::isStoreToStackSlot(const MachineInstr *MI,
	int &FrameIndex) const {
	if (MI->getOpcode() == VectorProc::SWi || MI->getOpcode() == VectorProc::SWf
		|| MI->getOpcode() == VectorProc::BLOCK_STOREI
		|| MI->getOpcode() == VectorProc::BLOCK_STOREF) {
		if (MI->getOperand(0).isFI() && MI->getOperand(1).isImm() &&
			MI->getOperand(1).getImm() == 0) {
			FrameIndex = MI->getOperand(0).getIndex();
			return MI->getOperand(2).getReg();
		}
	}

	return 0;
}

MachineInstr *
VectorProcInstrInfo::emitFrameIndexDebugValue(MachineFunction &MF,
                                         int FrameIx,
                                         uint64_t Offset,
                                         const MDNode *MDPtr,
                                         DebugLoc dl) const {
	MachineInstrBuilder MIB = BuildMI(MF, dl, get(VectorProc::DBG_VALUE))
		.addFrameIndex(FrameIx).addImm(0).addImm(Offset).addMetadata(MDPtr);

	return &*MIB;
}


bool VectorProcInstrInfo::AnalyzeBranch(MachineBasicBlock &MBB,
                                   MachineBasicBlock *&TBB,
                                   MachineBasicBlock *&FBB,
                                   SmallVectorImpl<MachineOperand> &Cond,
                                   bool AllowModify) const
{
	return true;	// Indicate we can't analyze branch
}

unsigned
VectorProcInstrInfo::InsertBranch(MachineBasicBlock &MBB,MachineBasicBlock *TBB,
                             MachineBasicBlock *FBB,
                             const SmallVectorImpl<MachineOperand> &Cond,
                             DebugLoc DL) const {
	return 0;
}

unsigned VectorProcInstrInfo::RemoveBranch(MachineBasicBlock &MBB) const
{
	return 0;
}

void VectorProcInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator I, DebugLoc DL,
                                 unsigned DestReg, unsigned SrcReg,
                                 bool KillSrc) const 
{
	BuildMI(MBB, I, DL, get(VectorProc::MOVEREG), DestReg).addReg(SrcReg, 
		getKillRegState(KillSrc));
}

MachineMemOperand *
VectorProcInstrInfo::GetMemOperand(MachineBasicBlock &MBB, int FI,
	unsigned Flag) const 
{
	MachineFunction &MF = *MBB.getParent();
	MachineFrameInfo &MFI = *MF.getFrameInfo();
	unsigned Align = MFI.getObjectAlignment(FI);

	return MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(FI), Flag,
		MFI.getObjectSize(FI), Align);
}

void VectorProcInstrInfo::
storeRegToStack(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                unsigned SrcReg, bool isKill, int FI,
                const TargetRegisterClass *RC, const TargetRegisterInfo *TRI,
                int64_t Offset) const {
	DebugLoc DL;
	if (I != MBB.end()) 
  		DL = I->getDebugLoc();
 
	MachineMemOperand *MMO = GetMemOperand(MBB, FI, MachineMemOperand::MOStore);
	unsigned Opc = 0;

	if (VectorProc::ScalarRegRegClass.hasSubClassEq(RC))
		Opc = VectorProc::SWi;
	else if (VectorProc::VectorRegRegClass.hasSubClassEq(RC))
		Opc = VectorProc::BLOCK_STOREI;
	else
		llvm_unreachable("unknown register class in storeRegToStack");

	BuildMI(MBB, I, DL, get(Opc)).addFrameIndex(FI).addImm(Offset)
		.addMemOperand(MMO).addReg(SrcReg, getKillRegState(isKill));
}

void VectorProcInstrInfo::
loadRegFromStack(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                 unsigned DestReg, int FI, const TargetRegisterClass *RC,
                 const TargetRegisterInfo *TRI, int64_t Offset) const {
	DebugLoc DL;
	if (I != MBB.end()) 
		DL = I->getDebugLoc();

	MachineMemOperand *MMO = GetMemOperand(MBB, FI, MachineMemOperand::MOLoad);
	unsigned Opc = 0;

	if (VectorProc::ScalarRegRegClass.hasSubClassEq(RC))
		Opc = VectorProc::LWi;
	else if (VectorProc::VectorRegRegClass.hasSubClassEq(RC))
		Opc = VectorProc::BLOCK_LOADI;
	else
		llvm_unreachable("unknown register class in storeRegToStack");

	BuildMI(MBB, I, DL, get(Opc), DestReg).addFrameIndex(FI).addImm(Offset)
		.addMemOperand(MMO);
}

unsigned VectorProcInstrInfo::getGlobalBaseReg(MachineFunction *MF) const
{
	VectorProcMachineFunctionInfo *VectorProcFI = MF->getInfo<VectorProcMachineFunctionInfo>();
	unsigned GlobalBaseReg = VectorProcFI->getGlobalBaseReg();
	if (GlobalBaseReg != 0)
		return GlobalBaseReg;

	// Insert the set of GlobalBaseReg into the first MBB of the function
	MachineBasicBlock &FirstMBB = MF->front();
	MachineBasicBlock::iterator MBBI = FirstMBB.begin();
	MachineRegisterInfo &RegInfo = MF->getRegInfo();

	GlobalBaseReg = RegInfo.createVirtualRegister(&VectorProc::ScalarRegRegClass);

	return GlobalBaseReg;
}
