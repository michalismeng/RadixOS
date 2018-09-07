#ifndef RTC_H_07092018
#define RTC_H_07092018

#include <types.h>
#include <time.h>

#define RTC_PORT_INDEX 0x70
#define RTC_PORT_DATA 0x71

typedef enum {
  RTC_REG_SEC = 0,
  RTC_REG_MIN = 2,
  RTC_REG_HOUR = 4,
  RTC_REG_WEEKDAY = 6,      // <-- may be unreliable       
  RTC_REG_DAY = 7,
  RTC_REG_MONTH = 8,
  RTC_REG_YEAR = 9,
  RTC_REG_CENTURY = 0x32,   // <-- century register may not exist and even if it does it is unreliable 
 
  RTC_STATUS_A = 10,
  RTC_STATUS_B = 11
} rtc_register_t;

typedef enum {
  RTC_FORMAT_24 = 2,
  RTC_FORMAT_BCD = 4,

  RTC_PM_BIT = 0x80       // If 12 hour format is used, then last bit controls am-pm.
} rtc_format_t;

// get the time from the CMOS RTC
void rtc_read_time(time_t* time);

#endif