// Copyright dotBunny Inc. All Rights Reserved.
// See the LICENSE file at the repository root for more information.

#pragma once
#include "NTestEnvironment.h"

class NEXUSCORE_API FNTestLatentCommands
{
public:
	static FNTestEnvironment Environment;
};



DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FNTestLatentCommand_FrameWait, int32, TotalFramesToWait);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FNTestLatentCommand_CreateWorld, FAutomationTestBase*, Test);
DEFINE_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_CleanupWorld);

DEFINE_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_PrePerformanceTest);
DEFINE_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_PostPerformanceTest);

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FNTestLatentCommand, TFunction<void()>, TestFunctionality);
DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FNTestLatentCommand_WorldTest, TFunction<void(UWorld*)>, TestFunctionality, const bool, bDisableGarbageCollection);

#define N_TESTS_LATENT_COMMANDS_PRE_PERFORMANCE_WAIT 100
#define N_TESTS_LATENT_COMMANDS_CREATE_WORLD_WAIT 10


#define N_TESTS_PERF_START_LATENT_TEST \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_PrePerformanceTest); \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_FrameWait(N_TESTS_LATENT_COMMANDS_PRE_PERFORMANCE_WAIT));

#define N_TESTS_PERF_START_LATENT_TEST_WORLD \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_PrePerformanceTest); \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_FrameWait(N_TESTS_LATENT_COMMANDS_PRE_PERFORMANCE_WAIT)); \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_CreateWorld(this)); \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_FrameWait(N_TESTS_LATENT_COMMANDS_CREATE_WORLD_WAIT));

#define N_TESTS_PERF_FINISH_LATENT_TEST \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_PostPerformanceTest); 

#define N_TESTS_PERF_FINISH_LATENT_TEST_WORLD \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_CleanupWorld); \
	ADD_LATENT_AUTOMATION_COMMAND(FNTestLatentCommand_PostPerformanceTest); 
