#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <list>
#include <utility>
#include <map>
#include "Target.hpp"

using namespace std;

//per ogni esecuzione stampare:
//indirizzo + tempo iniezione + tipo di fault

//TODO: add an address-object map to identify the injected object
//this map needs to be re-filled every time RTOS changes
//gdb_script can be used to extract the addresses of global variables
//after running gdb_script use getaddr.py to get the c++ map
//map length = 296

map<long, string> rtosObjects = {{0x4312a0, "pc54ByteString"}, {0x431298, "pc55ByteString"}, {0x4318e0, "xDelayedTaskList1"}, {0x431920, "xDelayedTaskList2"}, {0x431a80, "xActiveTimerList1"}, {0x431ac0, "xActiveTimerList2"}, {0x4312d8, "TraceQueueClassTable"}, {0x433260, "uiInEventGroupSetBitsFromISR"}, {0x4312e6, "CurrentFilterGroup"}, {0x4312e4, "CurrentFilterMask"}, {0x433e20, "RecorderData"}, {0x433728, "RecorderDataPtr"}, {0x4332a1, "handle_of_last_logged_task"}, {0x45f7dc, "init_hwtc_count"}, {0x433280, "isPendingContextSwitch"}, {0x4332a0, "nISRactive"}, {0x4332c0, "objectHandleStacks"}, {0x433290, "recorder_busy"}, {0x433294, "timestampFrequency"}, {0x433298, "traceErrorMessage"}, {0x433288, "trace_disable_timestamp"}, {0x43328c, "uiTraceSystemState"}, {0x433284, "uiTraceTickCount"}, {0x4332a8, "vTraceStopHookPtr"}, {0x43373c, "heapMemUsage"}, {0x433730, "isrstack"}, {0x433738, "last_timestamp"}, {0x4312e0, "readyEventsEnabled"}, {0x427ab8, "xAllowableMargin"}, {0x427ab0, "xHalfMaxBlockTime"}, {0x427aa8, "xMaxBlockTime"}, {0x431248, "pcBlockingTaskName"}, {0x431240, "pcControllingTaskName"}, {0x432028, "xBlockingCycles"}, {0x432020, "xControllingCycles"}, {0x432030, "xErrorOccurred"}, {0x432218, "sBlockingConsumerCount"}, {0x43221e, "sBlockingProducerCount"}, {0x432338, "ulISRCycles"}, {0x432330, "ulTestMasterCycles"}, {0x432334, "ulTestSlaveCycles"}, {0x432340, "xEventGroup"}, {0x432348, "xISREventGroup"}, {0x432398, "ulGuardedVariable"}, {0x432390, "ulLoopCounter"}, {0x4323b8, "xBlockWasAborted"}, {0x432388, "xErrorDetected"}, {0x4323a0, "xHighPriorityMutexTask"}, {0x4323a8, "xMediumPriorityMutexTask"}, {0x4323b0, "xSecondMediumPriorityMutexTask"}, {0x427e78, "xInterruptGivePeriod"}, {0x4323e4, "ulCountingSemaphoreLoops"}, {0x4323e0, "ulMasterLoops"}, {0x4323f8, "xISRCountingSemaphore"}, {0x4323f0, "xISRMutex"}, {0x432400, "xMasterSlaveMutex"}, {0x432410, "xOkToGiveCountingSemaphore"}, {0x432408, "xOkToGiveMutex"}, {0x4323e8, "xSlaveHandle"}, {0x431258, "xDemoStatus"}, {0x432448, "ulCycleCounters"}, {0x432440, "xControlMessageBuffer"}, {0x432430, "xCoreBMessageBuffers"}, {0x433020, "ucBufferStorage"}, {0x433060, "ulEchoLoopCounters"}, {0x433068, "ulNonBlockingRxCounter"}, {0x433010, "ulSenderLoopCounters"}, {0x43257c, "xBlockingStackSize"}, {0x432460, "xStaticMessageBuffers"}, {0x432598, "xPollingConsumerCount"}, {0x4325a0, "xPollingProducerCount"}, {0x433df0, "xHighPriorityTask"}, {0x433e00, "xHighestPriorityTask"}, {0x433df8, "xMediumPriorityTask"}, {0x4325c8, "xISRQueue"}, {0x431268, "xISRTestStatus"}, {0x433e10, "xQueueSetReceivingTask"}, {0x433e08, "xQueueSetSendingTask"}, {0x432610, "ulCycleCounter"}, {0x431278, "ulISRTxValue"}, {0x4325f8, "ulQueueUsedCounter"}, {0x432620, "uxNextRand"}, {0x432608, "xQueueSet"}, {0x431270, "xQueueSetTasksStatus"}, {0x4325e0, "xQueues"}, {0x432618, "xSetupComplete"}, {0x431708, "xQueue"}, {0x431280, "xQueueSetPollStatus"}, {0x432c20, "ulNextRand"}, {0x4327c0, "uxCreatorTaskStackBuffer"}, {0x432c28, "uxCycleCounter"}, {0x432700, "xCreatorTaskTCBBuffer"}, {0x431290, "pcDataSentFromInterrupt"}, {0x43306c, "ulInterruptTriggerCounter"}, {0x431288, "xErrorStatus"}, {0x433070, "xInterruptStreamBuffer"}, {0x432f80, "xStaticStreamBuffers"}, {0x4312b8, "pcStringToReceive"}, {0x4312b0, "pcStringToSend"}, {0x4330b0, "ulCycleCount"}, {0x4330a8, "xStreamBuffer"}, {0x4330c8, "ulNotifyCycleCount"}, {0x4330d8, "ulTimerNotificationsReceived"}, {0x4330dc, "ulTimerNotificationsSent"}, {0x4330d0, "xTaskToNotify"}, {0x431710, "xTimer"}, {0x4331f0, "ucAutoReloadTimerCounters"}, {0x433220, "ucISRAutoReloadTimerCounter"}, {0x433230, "ucISROneShotTimerCounter"}, {0x433210, "ucOneShotTimerCounter"}, {0x433140, "xAutoReloadTimers"}, {0x433238, "xBasePeriod"}, {0x433218, "xISRAutoReloadTimer"}, {0x433228, "xISROneShotTimer"}, {0x433208, "xOneShotTimer"}, {0x4312c8, "xTestStatus"}, {0x432240, "xPrimaryCycles"}, {0x432258, "xRunIndicator"}, {0x432238, "xSecondary"}, {0x432248, "xSecondaryCycles"}, {0x432230, "xTestQueue"}, {0x4322a0, "xParameters"}, {0x433de0, "xCreatedTask"}, {0x4322e0, "usCreationCount"}, {0x427c20, "uxMaxNumberOfExtraTasksRunning"}, {0x4322e8, "uxTasksRunningAtStart"}, {0x433de8, "xSuspendedTestQueue"}, {0x432308, "ulCounter"}, {0x432320, "ulExpectedValue"}, {0x43230c, "usCheckVariable"}, {0x4322f8, "xContinuousIncrementHandle"}, {0x432300, "xLimitedIncrementHandle"}, {0x432318, "xSuspendedQueueReceiveError"}, {0x432310, "xSuspendedQueueSendError"}, {0x432380, "usTaskCheck"}, {0x4323d0, "xTaskCheck"}, {0x4326a8, "uxBlockingCycles"}, {0x4326a0, "uxControllingCycles"}, {0x4326b0, "uxPollingCycles"}, {0x432698, "xBlockingIsSuspended"}, {0x4326c0, "xBlockingTaskHandle"}, {0x432690, "xControllingIsSuspended"}, {0x4326b8, "xControllingTaskHandle"}, {0x432680, "xMutex"}, {0x4326e0, "sCheckVariables"}, {0x4326e8, "sNextCheckVariable"}, {0x433c88, "xStdioMutex"}, {0x433be0, "xStdioMutexBuffer"}, {0x433760, "uxTimerTaskStack"}, {0x431200, "xTraceRunning"}, {0x431700, "xSemaphore"}, {0x433bc0, "fR"}, {0x433bc8, "fW"}, {0x431208, "string1"}, {0x431210, "string2"}, {0x431218, "string3"}, {0x431230, "pcStatusMessage"}, {0x431770, "xMutexToDelete"}, {0x431798, "ulStartTimeNs"}, {0x432000, "hMainThread"}, {0x431e60, "hSigSetupThread"}, {0x432018, "prvStartTimeNs"}, {0x432008, "uxCriticalNesting"}, {0x431f00, "xAllSignals"}, {0x431e80, "xResumeSignals"}, {0x432010, "xSchedulerEnd"}, {0x431f80, "xSchedulerOriginalSignalMask"}, {0x433ca0, "xQueueRegistry"}, {0x4317a0, "pxCurrentTCB"}, {0x431238, "uxTopUsedPriority"}, {0x431950, "pxOverflowDelayedTaskList"}, {0x4317c0, "pxReadyTasksLists"}, {0x431a60, "ulTaskSwitchedInTime"}, {0x431a64, "ulTotalRunTime"}, {0x431a08, "uxCurrentNumberOfTasks"}, {0x4319c8, "uxDeletedTasksWaitingCleanUp"}, {0x431a58, "uxSchedulerSuspended"}, {0x431a40, "uxTaskNumber"}, {0x431a18, "uxTopReadyPriority"}, {0x431a50, "xIdleTaskHandle"}, {0x431a48, "xNextTaskUnblockTime"}, {0x431a38, "xNumOfOverflows"}, {0x431a28, "xPendedTicks"}, {0x431960, "xPendingReadyList"}, {0x431a20, "xSchedulerRunning"}, {0x4319e0, "xSuspendedTaskList"}, {0x4319a0, "xTasksWaitingTermination"}, {0x431a10, "xTickCount"}, {0x431a30, "xYieldPending"}, {0x431ae8, "pxCurrentTimerList"}, {0x431af0, "pxOverflowTimerList"}, {0x431af8, "xTimerQueue"}, {0x431b00, "xTimerTaskHandle"}, {0x0000000000431000, "_GLOBAL_OFFSET_TABLE_"}, {0x00000000004311f0, "__data_start"}, {0x00000000004311f0, "data_start"}, {0x00000000004311f8, "__dso_handle"}, {0x0000000000431220, "ulLastParameter1.4291"}, {0x0000000000431228, "ulParameter1.4307"}, {0x0000000000431250, "usLastCreationCount.3319"}, {0x000000000043127c, "ulExpectedReceivedFromISR.3508"}, {0x00000000004312d0, "uxTick.3133"}, {0x00000000004312e8, "__TMC_END__"}, {0x00000000004312e8, "__bss_start"}, {0x00000000004312e8, "_edata"}, {0x0000000000431300, "stderr@@GLIBC_2.2.5"}, {0x0000000000431308, "completed"}, {0x0000000000431320, "xPrinted.4301"}, {0x0000000000431340, "xIdleTaskTCB.4315"}, {0x0000000000431400, "uxIdleTaskStack.4316"}, {0x0000000000431640, "xTimerTaskTCB.4322"}, {0x0000000000431720, "xStatus.3309"}, {0x0000000000431768, "pxStatusArray.3310"}, {0x0000000000431778, "ulLastParameter2.4292"}, {0x0000000000431780, "xTimer.4300"}, {0x0000000000431788, "ulParameter2.4308"}, {0x0000000000431790, "xPerformedOneShotTests.4316"}, {0x0000000000431b08, "xLastTime.3651"}, {0x0000000000431b20, "ucStaticTimerQueueStorage.3693"}, {0x0000000000431da0, "xStaticTimerQueue.3692"}, {0x0000000000432040, "xEventGroupBuffer.3169"}, {0x0000000000432080, "ucStorageBuffer.3178"}, {0x0000000000432089, "ucQueueStorage.3189"}, {0x00000000004320a0, "xQueueBuffer.3188"}, {0x0000000000432160, "xSemaphoreBuffer.3196"}, {0x0000000000432208, "xLastControllingCycleCount.3211"}, {0x0000000000432210, "xLastBlockingCycleCount.3212"}, {0x0000000000432224, "sLastBlockingConsumerCount.3461"}, {0x000000000043222a, "sLastBlockingProducerCount.3462"}, {0x0000000000432260, "xLastPrimaryCycleCount.2985"}, {0x0000000000432268, "xLastSecondaryCycleCount.2986"}, {0x00000000004322d0, "uxLastCount0.2948"}, {0x00000000004322d8, "uxLastCount1.2949"}, {0x00000000004322f0, "uxTasksRunningNow.3321"}, {0x0000000000432324, "ulValueToSend.3465"}, {0x0000000000432328, "usLastTaskCheck.3478"}, {0x000000000043232c, "ulLastExpectedValue.3479"}, {0x0000000000432360, "xCallCount.2965"}, {0x0000000000432368, "xISRTestError.2966"}, {0x0000000000432370, "ulPreviousSetBitCycles.2977"}, {0x0000000000432374, "ulPreviousWaitBitCycles.2976"}, {0x0000000000432378, "ulPreviousISRCycles.2978"}, {0x00000000004323c0, "uxLoopCount.3474"}, {0x00000000004323c8, "ulLastLoopCounter.3513"}, {0x00000000004323cc, "ulLastLoopCounter2.3514"}, {0x0000000000432418, "xLastGiveTime.3471"}, {0x0000000000432420, "ulLastMasterLoopCounter.3477"}, {0x0000000000432424, "ulLastCountingSemaphoreLoops.3478"}, {0x0000000000432450, "ulLastCycleCounters.3407"}, {0x0000000000432580, "ulLastEchoLoopCounters.3491"}, {0x0000000000432588, "ulLastNonBlockingRxCounter.3492"}, {0x0000000000432590, "ulLastSenderLoopCounters.3498"}, {0x00000000004325a8, "xPolledQueue.3428"}, {0x00000000004325bc, "ulLastLoopCounter.3464"}, {0x00000000004325d0, "ulCallCount.2935"}, {0x0000000000432628, "ulLastCycleCounter.3469"}, {0x0000000000432630, "ulLastQueueUsedCounter.3471"}, {0x000000000043263c, "ulLastISRTxValue.3470"}, {0x0000000000432640, "ulLoops.3487"}, {0x0000000000432648, "ePriorities.3488"}, {0x000000000043264c, "ulCallCount.3503"}, {0x0000000000432650, "ulExpectedReceivedFromTask.3507"}, {0x0000000000432658, "xQueueToWriteTo.3522"}, {0x0000000000432674, "ulCallCount.3440"}, {0x0000000000432678, "ulValueToSend.3441"}, {0x000000000043267c, "ulLastCycleCounter.3445"}, {0x00000000004326c8, "uxLastControllingCycles.2947"}, {0x00000000004326d0, "uxLastBlockingCycles.2948"}, {0x00000000004326d8, "uxLastPollingCycles.2949"}, {0x00000000004326f0, "sLastCheckVariables.3451"}, {0x0000000000432c40, "ucQueueStorageArea.3082"}, {0x0000000000432c80, "xStaticQueue.3081"}, {0x0000000000432d40, "uxStackBuffer.3117"}, {0x0000000000432f70, "uxLastCycleCounter.3187"}, {0x0000000000433078, "ucTempBuffer.3460"}, {0x0000000000433088, "xNextChar.3513"}, {0x0000000000433090, "ulLastEchoLoopCounters.3536"}, {0x0000000000433098, "ulLastNonBlockingRxCounter.3537"}, {0x000000000043309c, "ulLastInterruptTriggerCounter.3538"}, {0x00000000004330a0, "ulLastSenderLoopCounters.3543"}, {0x00000000004330b8, "xCallCount.3379"}, {0x00000000004330c0, "xNextByteToSend.3376"}, {0x00000000004330f0, "ulCallCount.2894"}, {0x00000000004330f8, "xCallCount.2910"}, {0x0000000000433100, "xAPIToUse.2911"}, {0x0000000000433108, "ulLastNotifyCycleCount.2923"}, {0x0000000000433240, "xLastCycleFrequency.3075"}, {0x0000000000433248, "xIterationsWithoutCounterIncrement.3074"}, {0x0000000000433250, "ulLastLoopCounter.3071"}, {0x0000000000433258, "uxCallCount.3142"}, {0x0000000000433740, "handle"}, {0x0000000000433744, "indexOfHandle.3008"}, {0x0000000000433748, "idx"}, {0x000000000043374c, "old_timestamp"}, {0x0000000000433750, "last_hwtc_count"}, {0x0000000000433754, "last_hwtc_rest"}};

class Injection{
public:
    Injection(const chrono::duration<long, std::ratio<1, 1000>> &elapsed, string faultType, Target obj
              ) : elapsed(elapsed), faultType(std::move(faultType)), object(std::move(obj)) {}

    Target object;
    chrono::duration<long, std::ratio<1, 1000>> elapsed{};
    string faultType;
};

class Logger{
private:
    list<Injection> inj;
    fstream logFile;
public:

    Logger() = default;

    void addInjection(long addr, chrono::duration<long, std::ratio<1, 1000>> elapsed, string faultType){
        Injection *i;
        Target *t;
        if(rtosObjects.find(addr) != rtosObjects.end()) //if it is a known address then retrieve the object from the map
            t = new Target(rtosObjects.find(addr)->second, addr, 0);
        else
            t = new Target("RTOSObject", addr, 0);

        i = new Injection(elapsed, std::move(faultType), std::move(*t));
        inj.push_back(*i);
    }
    void logOnfile(int pid){
        logFile.open("../logs/logFile_" + to_string(pid) + ".txt", ios::in | ios::out);
        for(const Injection& i : inj){
            string s_inj = "Address : " + to_string(i.object.getAddress()) + " --- Time : " + to_string(i.elapsed.count()) + " --- Fault type : " + i.faultType + " --- Injected Object : " + i.object.getName() + "\n";
            logFile.write(s_inj.c_str(), (int) s_inj.length());
        }
    }
    void printInj(){
        cout << endl;
        cout << endl;
        for(const Injection& i : inj){
            cout << "Address : 0x" << hex << i.object.getAddress() << " --- Time : " + to_string(i.elapsed.count()) + " --- Fault type : " + i.faultType << " --- Injected Object : " + i.object.getName() + "\n";
        }
    }

    virtual ~Logger() {
        if(logFile.is_open())
            logFile.close();
    }

};