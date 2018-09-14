#include <time.h>

uint8_t time_get_weekday(uint16_t year, uint8_t month, uint8_t day)
{
    // use Schwerdtfeger's method to calculate gregorian week day
    static uint8_t e[] = {/* Dummy entry for month=0 */-1, 0,3,2,5,0,3,5,1,4,6,2,4};
    static uint8_t f_greogrian[] = {0,5,3,1};

    uint32_t c, g, f;

    if(month >= 3)
    {
        c = year / 100 * 100;
        g = year - 100 * c;
    }
    else
    {
        c = (year - 1) / 100;
        g = year - 1 - 100 * c;
    }

    uint8_t w = (day + e[month] + f_greogrian[c % 4] + g + g / 4) % 7;
    return w;
}

void time_print(ktime_t* time)
{
    printf("%s, %u-%u-%u %u:%u:%u", weekday_to_str(time->weekday), 
                                    time->day, 
                                    time->month, 
                                    time->year,
                                    time->hour, 
                                    time->min, 
                                    time->sec);
}