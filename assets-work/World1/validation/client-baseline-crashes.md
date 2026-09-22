# Baseline client crashes — 2026-09-22

All observed before changing any World1/Object1 game files. OpenMU container was running. The client reached server selection and later Lorencia (test0Dw, including coordinates 188,119 and 134,119), but repeatedly crashed while preparing the reference captures. No required 1920×1080 screenshot was completed. No engine or CMake fixes were attempted.

The user authorized continuing offline with client verification pending. These are baseline failures, not evidence about replacement asset compatibility.

## Main-2026-09-22-133749.ips

{'codes': '0x0000000000000001, 0x0000000000000001', 'rawCodes': [1, 1], 'type': 'EXC_BAD_ACCESS', 'signal': 'SIGSEGV', 'subtype': 'KERN_INVALID_ADDRESS at 0x0000000000000001'}

```text
objc_msgSend
-[NSXPCEncoder _replaceObject:]
-[NSXPCEncoder encodeObject:forKey:]
-[WMXPCWindowTransactionAction encodeWithCoder:]
-[NSXPCEncoder _encodeObject:]
-[NSXPCEncoder _encodeArrayOfObjects:forKey:]
-[NSArray(NSArray) encodeWithCoder:]
-[NSXPCEncoder _encodeObject:]
-[NSXPCEncoder _encodeObject:]
_NSXPCSerializationAddInvocationWithOnlyObjectArgumentsArray
```

## Main-2026-09-22-133816.ips

{'codes': '0x0000000000000101, 0x0000000000000009', 'rawCodes': [257, 9], 'type': 'EXC_BAD_ACCESS', 'signal': 'SIGBUS', 'subtype': 'EXC_ARM_DA_ALIGN at 0x0000000000000009'}

```text
CFRelease
__CFBasicHashDrain
_CFRelease
-[NSKeyedUnarchiver dealloc]
+[NSKeyedUnarchiver unarchivedObjectOfClasses:fromData:error:]
+[NSKeyedUnarchiver unarchivedObjectOfClass:fromData:error:]
AudioComponentVector::createWithSerializedData(NSData*)
AudioComponentMgr_RegistrarService::mergeServerRegistrations(NSData*, bool, NSData*, NSArray*)
-[AudioComponentRegistrarClient registrationsChanged:includesExtensions:fsHash:]
__invoking___
```

## Main-2026-09-22-133851.ips

{'codes': '0x0000000000000001, 0x0000000000000229', 'rawCodes': [1, 553], 'type': 'EXC_BAD_ACCESS', 'signal': 'SIGSEGV', 'subtype': 'KERN_INVALID_ADDRESS at 0x0000000000000229'}

```text
AGX::FramebufferDriverConfig<AGX::HAL200::Encoders, AGX::HAL200::Classes, AGX::HAL200::ObjClasses>::FramebufferDriverConfig(MTLRenderPassDescriptor const*, unsigned int, eAGXLoadStorePatchMode)
AGX::FramebufferDriverConfig<AGX::HAL200::Encoders, AGX::HAL200::Classes, AGX::HAL200::ObjClasses>::FramebufferDriverConfig(MTLRenderPassDescriptor const*, unsigned int, eAGXLoadStorePatchMode)
-[AGXG16GFamilyRenderContext initWithCommandBuffer:descriptor:subEncoderIndex:framebuffer:]
-[AGXG16GFamilyCommandBuffer renderCommandEncoderWithDescriptor:]
METAL_BeginRenderPass
SDL_BeginGPURenderPass_REAL
mu::MuRendererSDLGpu::EndFrame()
MainLoop()
WinMain(HINSTANCE__*, HINSTANCE__*, char*, int)
main
```

## Main-2026-09-22-134152.ips

{'codes': '0x0000000000000000, 0x0000000000000000', 'rawCodes': [0, 0], 'type': 'EXC_CRASH', 'signal': 'SIGABRT'}

```text
__pthread_kill
pthread_kill
abort
__assert_rtn
MTLReportFailure.cold.1
MTLReportFailure
-[AGXG16GFamilyCommandBuffer blitCommandEncoderCommon:]
METAL_BeginCopyPass
SDL_BeginGPUCopyPass_REAL
mu::MuRendererSDLGpu::EndFrame()
```

## Main-2026-09-22-134335.ips

{'codes': '0x0000000000000000, 0x0000000000000000', 'rawCodes': [0, 0], 'type': 'EXC_CRASH', 'signal': 'SIGABRT'}

```text
__pthread_kill
pthread_kill
abort
__assert_rtn
IOGPUMetalCommandBufferStorageGrowKernelCommandBuffer.cold.6
IOGPUMetalCommandBufferStorageGrowKernelCommandBuffer
AGX::ContextCommon<AGX::HAL200::Encoders, AGX::HAL200::Classes, AGX::HAL200::ObjClasses>::newCommand(unsigned long)
AGX::BlitDispatchContext<AGX::HAL200::Encoders, AGX::HAL200::Classes, AGX::HAL200::ObjClasses>::beginComputePass(AGXStreamHardwareCommandRec**)
AGX::BlitDispatchContext<AGX::HAL200::Encoders, AGX::HAL200::Classes, AGX::HAL200::ObjClasses>::blitCDMBuffer(IOGPUMetalResource const*, unsigned long long, IOGPUMetalResource const*, unsigned long long, unsigned long, void const*, unsigned long)
-[AGXG16GFamilyBlitContext copyFromBuffer:sourceOffset:toBuffer:destinationOffset:size:]
```

## Main-2026-09-22-134341.ips

{'codes': '0x0000000000000001, 0x0000000000000001', 'rawCodes': [1, 1], 'type': 'EXC_BAD_ACCESS', 'signal': 'SIGSEGV', 'subtype': 'KERN_INVALID_ADDRESS at 0x0000000000000001'}

```text
objc_release
-[WMWindowPropertySnapshot copyWithZone:]
-[WMXPCWindowTransactionAction initLifecycleActionWithType:properties:fences:]
-[WMClientWindowManager prepareWindowTransaction:]
__54-[NSWMWindowCoordinator performTransactionUsingBlock:]_block_invoke.8
NSCGSTransactionRunPreCommitActionsForOrder_
NSCGSTransactionRunPreCommitActions_
__39+[_NSCGSTransaction currentTransaction]_block_invoke.26
CA::Transaction::run_commit_handlers(CATransactionPhase)
CA::Context::commit_transaction(CA::Transaction*, double, double*)
```

Terminal also reported:

```text
-[AGXG16GFamilyCommandBuffer blitCommandEncoderCommon:]:833:
failed assertion `A command encoder is already encoding to this command buffer'
```

Next client review: obtain original screenshots from the untouched baseline, then install exports and repeat matching locations, camera and resolution. Verify seams, material readability, unchanged alpha strips and the tavern still life; inspect new World1/Object1 load errors separately from the known missing Sound/Music/Object74 files.
