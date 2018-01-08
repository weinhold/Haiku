/*
 * Copyright 2012, Alex Smith, alex@alex-smith.me.uk.
 * Copyright 2009-2018, Ingo Weinhold, ingo_weinhold@gmx.de.
 * Copyright 2011-2015, Rene Gollent, rene@gollent.com.
 * Distributed under the terms of the MIT License.
 */
#ifndef ARCHITECTURE_X86_64_H
#define ARCHITECTURE_X86_64_H


#include <Array.h>

#include "Architecture.h"
#include "Register.h"


class SourceLanguage;


class ArchitectureX8664 : public Architecture {
public:
								ArchitectureX8664(uint32 featureFlags = 0);
	virtual						~ArchitectureX8664();

	virtual	status_t			Init();

	virtual int32				StackGrowthDirection() const;

	virtual	int32				CountRegisters() const;
	virtual	const Register*		Registers() const;
	virtual status_t			InitRegisterRules(CfaContext& context) const;

	virtual	status_t			GetDwarfRegisterMaps(RegisterMap** _toDwarf,
									RegisterMap** _fromDwarf) const;

	virtual	uint32				CpuFeatures() const;

	virtual	status_t			CreateCpuState(CpuState*& _state) const;
	virtual	status_t			CreateCpuState(const void* cpuStateData,
									size_t size, CpuState*& _state) const;
	virtual	status_t			CreateStackFrame(TeamMemory* teamMemory,
									Image* image, FunctionDebugInfo* function,
									CpuState* cpuState, bool isTopFrame,
									StackFrame*& _previousFrame,
									CpuState*& _previousCpuState) const;
	virtual	void				UpdateStackFrameCpuState(TeamMemory* teamMemory,
									const StackFrame* frame,
									Image* previousImage,
									FunctionDebugInfo* previousFunction,
									CpuState* previousCpuState) const;

	virtual	status_t			ReadValueFromMemory(TeamMemory* teamMemory,
									target_addr_t address, uint32 valueType,
									BVariant& _value) const;
	virtual	status_t			ReadValueFromMemory(TeamMemory* teamMemory,
									target_addr_t addressSpace,
									target_addr_t address, uint32 valueType,
									BVariant& _value) const;

	virtual	status_t			DisassembleCode(FunctionDebugInfo* function,
									const void* buffer, size_t bufferSize,
									DisassembledCode*& _sourceCode) const;
	virtual	status_t			GetStatement(TeamMemory* teamMemory,
									FunctionDebugInfo* function,
									target_addr_t address,
									Statement*& _statement) const;
	virtual	status_t			GetInstructionInfo(TeamMemory* teamMemory,
									target_addr_t address,
									InstructionInfo& _info,
									CpuState* state) const;
	virtual	status_t			ResolvePICFunctionAddress(
									TeamMemory* teamMemory,
									target_addr_t instructionAddress,
									CpuState* state,
									target_addr_t& _targetAddress) const;

	virtual	status_t			GetWatchpointDebugCapabilities(
									int32& _maxRegisterCount,
									int32& _maxBytesPerRegister,
									uint8& _watchpointCapabilityFlags) const;

	virtual	status_t			GetReturnAddressLocation(
									StackFrame* frame, target_size_t valueSize,
									ValueLocation*& _location) const;

private:
			struct ToDwarfRegisterMap;
			struct FromDwarfRegisterMap;

private:
			void				_AddRegister(int32 index, const char* name,
									uint32 bitSize, uint32 valueType,
									register_type type, bool calleePreserved);
			void				_AddIntegerRegister(int32 index,
									const char* name, uint32 valueType,
									register_type type, bool calleePreserved);
			void				_AddFPRegister(int32 index,
									const char* name);
			void				_AddSIMDRegister(int32 index,
									const char* name, uint32 byteSize);
			bool				_HasFunctionPrologue(TeamMemory* teamMemory,
									FunctionDebugInfo* function) const;
private:
			Array<Register>		fRegisters;
			SourceLanguage*		fAssemblyLanguage;
			ToDwarfRegisterMap*	fToDwarfRegisterMap;
			FromDwarfRegisterMap* fFromDwarfRegisterMap;
};


#endif	// ARCHITECTURE_X86_64_H
