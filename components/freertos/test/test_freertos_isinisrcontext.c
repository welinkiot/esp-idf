/*
 See if xPortInIsrContext works
*/

#include <esp_types.h>
#include <stdio.h>
#include "rom/ets_sys.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/xtensa_api.h"
#include "unity.h"
#include "esp_intr_alloc.h"
#include "xtensa/hal.h"

static volatile int in_int_context, int_handled;


static void testint(void *arg) {
    xthal_set_ccompare(1, xthal_get_ccount()+8000000000);
    ets_printf("INT!\n");
    if (xPortInIsrContext()) in_int_context++;
    int_handled++;
}


static void testthread(void *arg) {
    intr_handle_t handle;
    in_int_context=0;
    int_handled=0;
    TEST_ASSERT(!xPortInIsrContext());
    xthal_set_ccompare(2, xthal_get_ccount()+8000000);
    esp_intr_alloc(ETS_INTERNAL_TIMER1_INTR_SOURCE, 0, &testint, NULL, &handle);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    TEST_ASSERT(int_handled);
    TEST_ASSERT(in_int_context);
    esp_intr_free(handle);
    vTaskDelete(NULL);
}


TEST_CASE("xPortInIsrContext test", "[freertos]")
{
    xTaskCreatePinnedToCore(testthread, "tst" , 4096, NULL, 3, NULL, 0);
    vTaskDelay(150 / portTICK_PERIOD_MS);
    xTaskCreatePinnedToCore(testthread, "tst" , 4096, NULL, 3, NULL, 1);
    vTaskDelay(150 / portTICK_PERIOD_MS);
}

