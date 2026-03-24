#include "setrtc.h"

void RTC_SetTimeDate(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    sTime.Hours = 12;
    sTime.Minutes = 0;
    sTime.Seconds = 0;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }

    sDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
    sDate.Month = RTC_MONTH_MARCH;
    sDate.Date = 18;
    sDate.Year = 26;   // 表示 2026

    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
}