
#ifndef _KERNEL_RTC_H
#define _KERNEL_RTC_H

typedef struct {
    uint8_t h; /* hour */
    uint8_t m; /* minute */
    uint8_t s; /* sec */
} Time;

typedef struct {
    uint8_t d;  /* day */
    uint8_t m;  /* month */
    uint16_t y; /* year */
    uint8_t c;  /* century */
} Date;

typedef struct {
    Date date;
    Time time;
} DateTime;

/* rtc_get_data: get data (seconds, minutes, ...) from the specified register of
 * the Real Time Clock */
uint8_t rtc_get_data(enum rtc_regs reg);

/* rtc_get_time: get a filled "Time" struct with the current hour, minute, and
 * second from the RTC */
Time rtc_get_time(void);

/* rtc_get_date: get a filled "Date" struct with the current day, month, and
 * year (century empty for now) from the RTC */
Date rtc_get_date(void);

/* rtc_get_datetime: get a filled "DateTime" struct with the current Date from
 * rtc_get_date and the current Time from rtc_get_time */
DateTime rtc_get_datetime(void);

#endif /* _KERNEL_RTC_H */

