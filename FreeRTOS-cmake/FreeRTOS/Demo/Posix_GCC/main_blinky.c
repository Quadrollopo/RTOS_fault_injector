/*
 * FreeRTOS V202104.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/******************************************************************************
 * NOTE 1: The FreeRTOS demo threads will not be running continuously, so
 * do not expect to get real time behaviour from the FreeRTOS Linux port, or
 * this demo application.  Also, the timing information in the FreeRTOS+Trace
 * logs have no meaningful units.  See the documentation page for the Linux
 * port for further information:
 * https://freertos.org/FreeRTOS-simulator-for-Linux.html
 *
 * NOTE 2:  This project provides two demo applications.  A simple blinky style
 * project, and a more comprehensive test and demo application.  The
 * mainCREATE_SIMPLE_BLINKY_DEMO_ONLY setting in main.c is used to select
 * between the two.  See the notes on using mainCREATE_SIMPLE_BLINKY_DEMO_ONLY
 * in main.c.  This file implements the simply blinky version.  Console output
 * is used in place of the normal LED toggling.
 *
 * NOTE 3:  This file only contains the source code that is specific to the
 * basic demo.  Generic functions, such FreeRTOS hook functions, are defined
 * in main.c.
 ******************************************************************************
 *
 * main_blinky() creates one queue, one software timer, and two tasks.  It then
 * starts the scheduler.
 *
 * The Queue Send Task:
 * The queue send task is implemented by the prvQueueSendTask() function in
 * this file.  It uses vTaskDelayUntil() to create a periodic task that sends
 * the value 100 to the queue every 200 milliseconds (please read the notes
 * above regarding the accuracy of timing under Linux).
 *
 * The Queue Send Software Timer:
 * The timer is an auto-reload timer with a period of two seconds.  The timer's
 * callback function writes the value 200 to the queue.  The callback function
 * is implemented by prvQueueSendTimerCallback() within this file.
 *
 * The Queue Receive Task:
 * The queue receive task is implemented by the prvQueueReceiveTask() function
 * in this file.  prvQueueReceiveTask() waits for data to arrive on the queue.
 * When data is received, the task checks the value of the data, then outputs a
 * message to indicate if the data came from the queue send task or the queue
 * send software timer.
 *
 * Expected Behaviour:
 * - The queue send task writes to the queue every 200ms, so every 200ms the
 *   queue receive task will output a message indicating that data was received
 *   on the queue from the queue send task.
 * - The queue send software timer has a period of two seconds, and is reset
 *   each time a key is pressed.  So if two seconds expire without a key being
 *   pressed then the queue receive task will output a message indicating that
 *   data was received on the queue from the queue send software timer.
 *
 * NOTE:  Console input and output relies on Linux system calls, which can
 * interfere with the execution of the FreeRTOS Linux port. This demo only
 * uses Linux system call occasionally. Heavier use of Linux system calls
 * may crash the port.
 */

#include <stdio.h>
#include <pthread.h>
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include <unistd.h>
/* Local includes. */
#include "console.h"
#include <sys/types.h>
#include <stdlib.h>


/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	mainQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

/* The rate at which data is sent to the queue.  The times are converted from
milliseconds to ticks using the pdMS_TO_TICKS() macro. */
#define mainTASK_SEND_FREQUENCY_MS			pdMS_TO_TICKS( 1000UL )
#define mainTIMER_SEND_FREQUENCY_MS			pdMS_TO_TICKS( 3100UL )

/* The number of items the queue can hold at once. */
#define mainQUEUE_LENGTH					( 2 )

/* The values sent to the queue receive task from the queue send task and the
queue send software timer respectively. */
#define mainVALUE_SENT_FROM_TASK			( 100UL )
#define mainVALUE_SENT_FROM_TIMER			( 200UL )

#define PARALLEL 0
/*-----------------------------------------------------------*/

/*
 * The tasks as described in the comments at the top of this file.
 */
static void prvQueueReceiveTask( void *pvParameters );
static void prvQueueSendTask( void *pvParameters );
static void mySender( void *pvParameters );
static void myReceiver( void *pvParameters);

/*
 * The callback function executed when the software timer expires.
 */
static void prvQueueSendTimerCallback( TimerHandle_t xTimerHandle );

/*-----------------------------------------------------------*/

/* The queue used by both tasks. */
static QueueHandle_t xQueue = (void *) 0;
static StaticQueue_t xStaticQueue;
uint8_t ucQueueStorageArea[ mainQUEUE_LENGTH * 50 ];

/* Structure that will hold the TCB of the task being created. */
StaticTask_t xTaskBuffer1;
StaticTask_t xTaskBuffer2;

/* Buffer that the task being created will use as its stack.  Note this is
an array of StackType_t variables.  The size of StackType_t is dependent on
the RTOS port. */
StackType_t xStack1[ 200 ];
StackType_t xStack2[ 200 ];

/* An array of StaticTimer_t structures, which are used to store
 the state of each created timer. */
StaticTimer_t xTimerBuffer;

/* A software timer that is started from the tick hook. */
static TimerHandle_t xTimer = NULL;
static TaskHandle_t task1 = NULL;
static TaskHandle_t task2 = NULL;

//TickType_t xBlockTime = mainTASK_SEND_FREQUENCY_MS; //this is the variable that needs to be modified in order to get a delay


/*-----------------------------------------------------------*/

/* File to read and file to write */
FILE *fR, *fW;

/*** SEE THE COMMENTS AT THE TOP OF THIS FILE ***/
void main_blinky( void )
{
const TickType_t xTimerPeriod = mainTIMER_SEND_FREQUENCY_MS;

	/* Create the queue. */
	xQueue = xQueueCreateStatic( mainQUEUE_LENGTH, sizeof( char )*50, ucQueueStorageArea, &xStaticQueue);
    char falso[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int pid = getpid();
    snprintf(falso, 32, "../files/Falso_Dante_%d.txt", pid);

    console_print(falso);
    if( xQueue != NULL )
	{
        fR = fopen("../Vero_Dante.txt", "r");
        fW = fopen(falso, "w");
        if(fR==NULL || fW==NULL) {
            vQueueDelete(xQueue);
            printf("Non ho trovato i file\n");
            if(fR != NULL)
                printf("Non ho trovato Vero_Dante.txt");
                fclose(fR);
        }
        else {
            /* Start the two tasks as described in the comments at the top of this
            file. */
            task1 = xTaskCreateStatic( /*myReceiver,*/ prvQueueReceiveTask,            /* The function that implements the task. */
                                         "RX",                            /* The text name assigned to the task - for debug only as it is not used by the kernel. */
                                         configMINIMAL_STACK_SIZE,        /* The size of the stack to allocate to the task. */
                                         NULL,                            /* The parameter passed to the task - not used in this simple case. */
                                         mainQUEUE_RECEIVE_TASK_PRIORITY,/* The priority assigned to the task. */
                                         xStack1, &xTaskBuffer1);                            /* The task handle is not required, so NULL is passed. */

            task2 = xTaskCreateStatic(prvQueueSendTask, "TX", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_SEND_TASK_PRIORITY, xStack2, &xTaskBuffer2);
            //xTaskCreate( mySender, /*prvQueueSendTask,*/ "Sender", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );

            /* Create the software timer, but don't start it yet. */
            xTimer = xTimerCreateStatic(
                    "Timer",                /* The text name assigned to the software timer - for debug only as it is not used by the kernel. */
                    xTimerPeriod,        /* The period of the software timer in ticks. */
                    pdTRUE,                /* xAutoReload is set to pdTRUE. */
                    NULL,                /* The timer's ID is not used. */
                    prvQueueSendTimerCallback, &xTimerBuffer);/* The function executed when the timer expires. */

            if (xTimer != NULL) {
                xTimerStart(xTimer, 0);
            }

            /* Start the tasks and timer running. */
            vTaskStartScheduler();
            fclose(fR);
            fclose(fW);

        }
	}

	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was insufficient FreeRTOS heap memory available for the idle and/or
	timer tasks	to be created.  See the memory management section on the
	FreeRTOS web site for more details. */
}
/*-----------------------------------------------------------*/

/*static void mySender(void *params) {
  (void*)params;
  int i = 0;
  while (1) {
    printf("Sender task inserting %d\n", i);
    xQueueSend(xQueue, &i, 1000);
    i++;
    vTaskDelay(1000);
  }
}

static void myReceiver(void* params) {
  (void*)params;
  int i;
  while (1) {
    xQueueReceive(xQueue, &i, 2000);
    printf("Receiver task reading %d\n", i);
    vTaskDelay(1000);
  }
}*/


static void prvQueueSendTask( void *pvParameters )
{
TickType_t xNextWakeTime;
const TickType_t xBlockTime = mainTASK_SEND_FREQUENCY_MS; //this is the variable that needs to be modified in order to get a delay
const char* ulValueToSend = "Task";
const char* name = pcTaskGetName(NULL);
const char* msg[2] = {ulValueToSend, name};
char line[50] = {0};


	/* Prevent the compiler warning about the unused parameter. */
	( void ) pvParameters;

    char *res;
	/* Initialise xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
        res = fgets(line, 50, fR);
        if(res == NULL) vTaskEndScheduler(); //file ended
        msg[1] = line;
        console_print("Row read from file Vero_Dante\n");
		/* Place this task in the blocked state until it is time to run again.
		The block time is specified in ticks, pdMS_TO_TICKS() was used to
		convert a time specified in milliseconds into a time specified in ticks.
		While in the Blocked state this task will not consume any CPU time. */
		xTaskDelayUntil( &xNextWakeTime, xBlockTime );
		/* Send to the queue - causing the queue receive task to unblock and
		write to the console.  0 is used as the block time so the send operation
		will not block - it shouldn't need to block as the queue should always
		have at least one space at this point in the code. */
		xQueueSend( xQueue, msg, 100U );
	}
}
/*-----------------------------------------------------------*/

static void prvQueueSendTimerCallback( TimerHandle_t xTimerHandle )
{
const char* ulValueToSend[2] = {"Timer", "Message from Timer!\n"};

	/* This is the software timer callback function.  The software timer has a
	period of two seconds and is reset each time a key is pressed.  This
	callback function will execute if the timer expires, which will only happen
	if a key is not pressed for two seconds. */

	/* Avoid compiler warnings resulting from the unused parameter. */
	( void ) xTimerHandle;

	/* Send to the queue - causing the queue receive task to unblock and
	write out a message.  This function is called from the timer/daemon task, so
	must not block.  Hence the block time is set to 0. */
	xQueueSend( xQueue, ulValueToSend, 0U );
}
/*-----------------------------------------------------------*/

static void prvQueueReceiveTask( void *pvParameters )
{

    char string1[8] = "string1";
    char string2[8] = "string2";
    char string3[8] = "string3";
    char* ulReceivedValue[2];
    int N = 0;
	/* Prevent the compiler warning about the unused parameter. */
	( void ) pvParameters;

    for( ;; )
	{

		/* Wait until something arrives in the queue - this task will block
		indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
		FreeRTOSConfig.h.  It will not use any CPU time while it is in the
		Blocked state. */
		xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );
		/* To get here something must have been received from the queue, but
		is it an expected value?  Normally calling printf() from a task is not
		a good idea.  Here there is lots of stack space and only one task is
		using console IO so it is ok.  However, note the comments at the top of
		this file about the risks of making Linux system calls (such as
		console output) from a FreeRTOS task. */
		if( !strcmp(ulReceivedValue[0], "Task") )
		{
            fprintf(fW, "%s", ulReceivedValue[1]);
#ifndef PARALLEL
			console_print( "Message write on file Falso_Dante\n");
#endif
            //console_print(string1);
            //console_print(string2);
            //console_print(string3);
            //console_print("\n");
		}
		else if( !strcmp(ulReceivedValue[0], "Timer") )
		{
            fprintf(fW, "%s", ulReceivedValue[1]);
#ifndef PARALLEL
			console_print( "Message received from software timer\n" );
#endif
            N++;
            if(N == 3) {
                vTaskEndScheduler();
            }
		}
		else
		{
			console_print( "Unexpected message\n" );
		}
	}
}
/*-----------------------------------------------------------*/
