//===-- VectorProcISelLowering.h - VectorProc DAG Lowering Interface ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that VectorProc uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef VECTORPROC_ISELLOWERING_H
#define VECTORPROC_ISELLOWERING_H

#include "VectorProc.h"
#include "llvm/Target/TargetLowering.h"

namespace llvm {
	class VectorProcSubtarget;

	namespace VectorProcISD {
		enum {
			FIRST_NUMBER = ISD::BUILTIN_OP_END,
			LOAD_LITERAL,
			CALL,        // A call instruction.
			RET_FLAG,    // Return with a flag operand.
			SPLAT,			// Copy scalar register into all lanes of a vector
			SEL_COND_RESULT,
			RECIPROCAL_EST
		};
	}

	class VectorProcTargetLowering : public TargetLowering {
		const VectorProcSubtarget *Subtarget;
	public:
		VectorProcTargetLowering(TargetMachine &TM);
		virtual SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const;
		virtual MachineBasicBlock *EmitInstrWithCustomInserter(MachineInstr *MI,
			MachineBasicBlock *MBB) const;
		virtual const char *getTargetNodeName(unsigned Opcode) const;
		ConstraintType getConstraintType(const std::string &Constraint) const;
		std::pair<unsigned, const TargetRegisterClass*>
			getRegForInlineAsmConstraint(const std::string &Constraint, EVT VT) const;
		virtual bool isOffsetFoldingLegal(const GlobalAddressSDNode *GA) const;
		SDValue LowerBUILD_VECTOR(SDValue Op, SelectionDAG &DAG) const;
		SDValue LowerVECTOR_SHUFFLE(SDValue Op, SelectionDAG &DAG) const;
		SDValue LowerINSERT_VECTOR_ELT(SDValue Op, SelectionDAG &DAG) const;
		SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
		SDValue LowerConstantPool(SDValue Op, SelectionDAG &DAG) const;
		SDValue LowerConstant(SDValue Op, SelectionDAG &DAG) const;
		SDValue LowerFDIV(SDValue Op, SelectionDAG &DAG) const;
		EVT getSetCCResultType(EVT VT) const;
		virtual SDValue LowerReturn(SDValue Chain,
			CallingConv::ID CallConv, bool isVarArg,
			const SmallVectorImpl<ISD::OutputArg> &Outs,
			const SmallVectorImpl<SDValue> &OutVals,
			DebugLoc dl, SelectionDAG &DAG) const;
		SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
		virtual SDValue LowerFormalArguments(SDValue Chain,
			CallingConv::ID CallConv,
			bool isVarArg,
			const SmallVectorImpl<ISD::InputArg> &Ins,
			DebugLoc dl, SelectionDAG &DAG,
			SmallVectorImpl<SDValue> &InVals) const;
		virtual SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
			SmallVectorImpl<SDValue> &InVals) const;
		unsigned getSRetArgSize(SelectionDAG &DAG, SDValue Callee) const;
	};
} // end namespace llvm

#endif    // VECTORPROC_ISELLOWERING_H
