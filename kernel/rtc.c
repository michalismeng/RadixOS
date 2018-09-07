#include <rtc.h>
#include <system.h>

static uint32_t to_binary(uint32_t bcd)
{
    return ((bcd & 0xF0) >> 1) + ((bcd & 0xF0) >> 3) + (bcd & 0xf);
}

static uint8_t rtc_read_cmos(uint16_t reg)
{
    outportb(RTC_PORT_INDEX, reg);
    return inportb(RTC_PORT_DATA);
} 

static uint8_t rtc_read_prog_flag()
{
    outportb(RTC_PORT_INDEX, RTC_STATUS_A);
    return (inportb(RTC_PORT_DATA) & 0x80);
}

#define rtc_is_bcd(s) (s & RTC_FORMAT_BCD)
#define rtc_is_24(s)  (s & RTC_FORMAT_24)
#define rtc_get_hour(h) (h & (~RTC_PM_BIT))
#define rtc_get_mode(h) (h & RTC_PM_BIT)


void rtc_read_time(time_t* time)
{
    uint8_t second[2];
    uint8_t minute[2];
    uint8_t hour[2];
    uint8_t day[2];
    uint8_t month[2];
    uint8_t year[2];
    uint32_t century[2];

    while(rtc_read_prog_flag());        // wait to finish update

    // perform a first read of the cmos variables
    second[0] = rtc_read_cmos(RTC_REG_SEC);
    minute[0] = rtc_read_cmos(RTC_REG_MIN);
    hour[0] = rtc_read_cmos(RTC_REG_HOUR);
    day[0] = rtc_read_cmos(RTC_REG_DAY);
    month[0] = rtc_read_cmos(RTC_REG_MONTH);
    year[0] = rtc_read_cmos(RTC_REG_YEAR);
    century[0] = rtc_read_cmos(RTC_REG_CENTURY);

    do {
        // read cmos variables until they are the same twice in a row
        // this is required to avoid inconsistent values during RTC updates
        second[1] = second[0];
        minute[1] = minute[0];
        hour[1] = hour[0];
        day[1] = day[0];
        month[1] = month[0];
        year[1] = year[0];

        while(rtc_read_prog_flag());

        second[0] = rtc_read_cmos(RTC_REG_SEC);
        minute[0] = rtc_read_cmos(RTC_REG_MIN);
        hour[0] = rtc_read_cmos(RTC_REG_HOUR);
        day[0] = rtc_read_cmos(RTC_REG_DAY);
        month[0] = rtc_read_cmos(RTC_REG_MONTH);
        year[0] = rtc_read_cmos(RTC_REG_YEAR);

    } while(second[0] != second[1] || minute[0] != minute[1] || hour[0] != hour[1] || day[0] != day[1] || month[0] != month[1] || year[0] != year[1]);

    uint8_t register_b = rtc_read_cmos(RTC_STATUS_B);

    // converto to binary if not already in that format
    if(!rtc_is_bcd(register_b))
    {
        second[0] = to_binary(second[0]);
        minute[0] = to_binary(minute[0]);
        hour[0] = to_binary(rtc_get_hour(hour[0])) | rtc_get_mode(hour[0]);
        day[0] = to_binary(day[0]);
        month[0] = to_binary(month[0]);
        year[0] = to_binary(year[0]);
    }

    // convert to 24 hour if necessary
    if(!rtc_is_24(register_b) && rtc_get_mode(hour[0]))
        hour[0] = (rtc_get_hour(hour[0]) + 12) % 24;

    century[0] = 2000;      // assume century is 2000...

    // pass the results back to the caller
    time->sec = second[0];
    time->min = minute[0];
    time->hour = hour[0];
    time->day = day[0];
    time->month = month[0];
    time->year = year[0] + century[0];

    time->weekday = time_get_weekday(year[0], month[0], day[0]);
}