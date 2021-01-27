/****************************************************************************
*  Copyright 2018 Gorgon Meducer (Email:embedded_zhuoran@hotmail.com)       *
*                                                                           *
*  Licensed under the Apache License, Version 2.0 (the "License");          *
*  you may not use this file except in compliance with the License.         *
*  You may obtain a copy of the License at                                  *
*                                                                           *
*     http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                           *
*  Unless required by applicable law or agreed to in writing, software      *
*  distributed under the License is distributed on an "AS IS" BASIS,        *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
*  See the License for the specific language governing permissions and      *
*  limitations under the License.                                           *
*                                                                           *
****************************************************************************/

/*============================ INCLUDES ======================================*/
#include ".\app_platform\app_platform.h"
#include <string.h>

/*============================ MACROS ========================================*/

#ifndef FRAME_BUFFER_SIZE
#   warning No defined FRAME_BUFFER_SIZE, default value 512 is used
#   define FRAME_BUFFER_SIZE                (512)
#endif

#ifndef DELAY_OBJ_POOL_SIZE
#   warning No defined DELAY_OBJ_POOL_SIZE, default value 4 is used
#   define DELAY_OBJ_POOL_SIZE              (4)
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
static volatile uint32_t    s_wMSTicks = 0;   
NO_INIT es_simple_frame_t   s_tFrame;
NO_INIT multiple_delay_t    s_tDelayService;
NO_INIT telegraph_engine_t  s_tTelegraphEngine;
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/


/*----------------------------------------------------------------------------
  SysTick / Timer0 IRQ Handler
 *----------------------------------------------------------------------------*/

void SysTick_Handler (void) 
{
    /*! 1ms timer event handler */
    s_wMSTicks++;
    
    if (!(s_wMSTicks % 1000)) {
        static volatile uint16_t wValue = 0;

        //printf("%s [%08x]\r\n", "Hello world!", wValue++);
        
        //STREAM_OUT.Stream.Flush();
    }
    
    MULTIPLE_DELAY.Dependent.TimerTickService(&s_tDelayService);
    
    /*! call application platform 1ms event handler */
    app_platform_1ms_event_handler();
}



static void system_init(void)
{
    //! initialise multiple delay service
    do {
        NO_INIT static multiple_delay_item_t s_tDelayObjPool[DELAY_OBJ_POOL_SIZE];

        MULTIPLE_DELAY_CFG (    &s_tDelayService,
                                (uint8_t *)s_tDelayObjPool,
                                sizeof(s_tDelayObjPool)
                                
                            );
    } while(false);
    
    SysTick_Config(SystemCoreClock  / 1000);  //!< Generate interrupt every 1 ms 
}


#if DEMO_MULTIPLE_DELAY == ENABLED

static void app_2000ms_delay_timeout_event_handler(multiple_delay_report_status_t tStatus, void *pObj)
{
    static volatile uint16_t wValue = 0;

    printf("%s [%08x]\r\n", "Hello world!", wValue++);
        
    STREAM_OUT.Stream.Flush();
    
    //! request again
    MULTIPLE_DELAY.RequestDelay(&s_tDelayService, 
                                2000,                                           //!< request delay 2000ms
                                MULTIPLE_DELAY_LOW_PRIORITY,                    //!< priority is low
                                NULL,                                           //!< no tag
                                &app_2000ms_delay_timeout_event_handler);       //!< timout event handler
}


static void app_3000ms_delay_timeout_event_handler(multiple_delay_report_status_t tStatus, void *pObj)
{
    static volatile uint16_t wValue = 0;

    printf("%s [%08x]\r\n", "Apple!", wValue++);
        
    STREAM_OUT.Stream.Flush();
    
    //! request again
    MULTIPLE_DELAY.RequestDelay(&s_tDelayService, 
                                3000,                                           //!< request delay 3000ms
                                MULTIPLE_DELAY_LOW_PRIORITY,                    //!< priority is low
                                NULL,                                           //!< no tag
                                &app_3000ms_delay_timeout_event_handler);       //!< timout event handler
}

static void app_1500ms_delay_timeout_event_handler(multiple_delay_report_status_t tStatus, void *pObj)
{
    static volatile uint16_t wValue = 0;

    printf("%s [%08x]\r\n", "Orange!", wValue++);
        
    STREAM_OUT.Stream.Flush();
    
    //! request again
    MULTIPLE_DELAY.RequestDelay(&s_tDelayService, 
                                1500,                                           //!< request delay 1500ms
                                MULTIPLE_DELAY_NORMAL_PRIORITY,                 //!< priority is normal
                                NULL,                                           //!< no tag
                                &app_1500ms_delay_timeout_event_handler);       //!< timout event handler
}

#endif

#if DEMO_FRAME_USE_BLOCK_MODE == ENABLED
    
static block_t * frame_parser(block_t *ptBlock, void *ptObj)
{
    #define DEMO_STRING         "Hello world!\r\n"
    do {
        if (NULL == ptBlock) {
            break;
        }
        uint8_t *pchBuffer = BLOCK.Buffer.Get(ptBlock);
        memcpy(&pchBuffer[1], DEMO_STRING, sizeof(DEMO_STRING));
        BLOCK.Size.Set(ptBlock, sizeof(DEMO_STRING)+1);
        
    } while(false);
    
    
    return ptBlock;
}
#else
static uint_fast16_t frame_parser(mem_block_t tMemory, uint_fast16_t hwSize)
{
    return hwSize;
}
#endif



static void app_init(void)
{
    //! initialise simple frame service
    do {
        NO_INIT static uint8_t s_chFrameBuffer[FRAME_BUFFER_SIZE];
        i_pipe_t s_tPipe = {
            .ReadByte = (STREAM_IN.Stream.ReadByte),
            .WriteByte = (STREAM_OUT.Stream.WriteByte),
            .Stream = {
                .Read = STREAM_IN.Stream.Read,
                .Write = STREAM_OUT.Stream.Write,
            },
        };
        
    #if DEMO_FRAME_USE_BLOCK_MODE == ENABLED
    
        NO_INIT static union {
            block_t tBlock;
            uint8_t chBuffer[FRAME_BUFFER_SIZE + sizeof(block_t)];
        } s_tBuffer;
        /*
typedef struct {
    void *          pBuffer;
    union {
        struct {
            uint32_t    BlockSize           : 24;
            uint32_t    IsNoWrite           : 1;
            uint32_t    IsNoRead            : 1;
            uint32_t    IsNoDirectAccess    : 1;
            uint32_t                        : 5;
        };
        uint32_t        wCapability;
    };
    uint_fast8_t    chAdapterID;
} block_cfg_t;
*/
        do {
            const block_cfg_t tCFG = {
                .pBuffer = NULL,
                .BlockSize = sizeof(s_tBuffer),
                //.IsNoWrite = false,
                //.IsNoRead = false,
                //.IsNoDirectAccess = false,
                //.chAdapterID = BLOCK_FREE_TO_ANY
            };
            BLOCK.Init(&s_tBuffer.tBlock, (block_cfg_t *)&tCFG);
        } while(0);
    #endif
        //! initialise simple frame service
        ES_SIMPLE_FRAME_CFG(    &s_tFrame, 
                                &s_tPipe,
                                
                            #if DEMO_FRAME_USE_BLOCK_MODE == ENABLED
                                &frame_parser,
                                .bStaticBufferMode = false,
                                .ptBlock = &s_tBuffer.tBlock,
                                .pTag = &s_tTelegraphEngine
                            #else
                                &frame_parser,
                                s_chFrameBuffer,
                                sizeof(s_chFrameBuffer)
                            #endif
                            );
    } while(false);
    
    
#if DEMO_MULTIPLE_DELAY == ENABLED
    MULTIPLE_DELAY.RequestDelay(&s_tDelayService, 
                                2000,                                           //!< request delay 2000ms
                                MULTIPLE_DELAY_LOW_PRIORITY,                    //!< priority is low
                                NULL,                                           //!< no tag
                                &app_2000ms_delay_timeout_event_handler);       //!< timout event handler
                                
    //! request again
    MULTIPLE_DELAY.RequestDelay(&s_tDelayService, 
                                3000,                                           //!< request delay 3000ns
                                MULTIPLE_DELAY_LOW_PRIORITY,                    //!< priority is low
                                NULL,                                           //!< no tag
                                &app_3000ms_delay_timeout_event_handler);       //!< timout event handler
                                
    //! request again
    MULTIPLE_DELAY.RequestDelay(&s_tDelayService, 
                                1500,                             st delay 1500ms
                                MULTIPLE_DELAY_NORMAL_PRIORITY,                 //!< priority is normal
                                NULL,                                           //!< no tag
                                &app_1500ms_delay_timeout_event_handler);       //!< timout event handler
#endif

}

/*----------------------------------------------------------------------------
  Main function
 *----------------------------------------------------------------------------*/

int main (void) 
{
    system_init();
    app_init();
    
    printf("for any string you entered, it will be echo back immediatly.\r\n");
    
    while (true) {
    
    #if false
        if (fsm_rt_cpl == ES_SIMPLE_FRAME.Task(&s_tFrame)) {
            STREAM_OUT.Stream.Flush();
        }
    #else
        uint8_t chByte;
        if (STREAM_IN.Stream.ReadByte(&chByte)) {
            STREAM_OUT.Stream.WriteByte(chByte );
            //printf("0x%02x ", chByte);
        } else {
            STREAM_OUT.Stream.Flush();
        }
    #endif
    #if DEMO_MULTIPLE_DELAY == ENABLED
        MULTIPLE_DELAY.Task(&s_tDelayService);
    #endif
    
        extern void example_menu_task(void);
        example_menu_task();
    
    }
}




