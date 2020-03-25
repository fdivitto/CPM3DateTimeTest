// v1.0 Feb 2020 - Fabrizio Di Vittorio


#include <stdio.h>
#include <cpm.h>


// no MSDOS check!
#pragma output noprotectmsdos


struct SCBPB_t {
  uint8_t offset;
  uint8_t set;
  union {
    uint8_t value_b;
    uint16_t value_w;
  };
};


// 719468 = number of days between 0000-03-01 and 1970-01-01 (unix base)
// 722389 = number of days between 0000-03-01 and 1978-01-01 (CP/M base)
//const int DATEBASE = 719468;
const int32_t DATEBASE = 722389;



// Returns number of days since civil DATEBASE.  Negative values indicate days prior to DATEBASE.
// Preconditions:  y-m-d represents a date in the civil (Gregorian) calendar
//                 m is in [1, 12]
//                 d is in [1, last_day_of_month(y, m)]
// ref: http://howardhinnant.github.io/date_algorithms.html
/*
int32_t days_from_civil(int32_t y, uint32_t m, uint32_t d)
{
  y -= m <= 2;
  const int32_t  era = (y >= 0 ? y : y - 399) / 400;
  const uint32_t yoe = (uint32_t)(y - era * 400);
  const uint32_t doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const uint32_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + (int32_t)(doe) - DATEBASE;
}
//*/


// Returns year/month/day triple in civil calendar
// Preconditions:  z is number of days since DATEBASE
// ref: http://howardhinnant.github.io/date_algorithms.html
void civil_from_days(int32_t z, int * ret_y, int * ret_m, int * ret_d)
{
  z += DATEBASE;
  const int32_t  era = (z >= 0 ? z : z - 146096) / 146097;
  const uint32_t doe = (uint32_t)(z - era * 146097);
  const uint32_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  const int32_t  y   = (int32_t)(yoe) + era * 400;
  const uint32_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  const uint32_t mp  = (5 * doy + 2) / 153;
  *ret_d             = doy - (153 * mp + 2) / 5 + 1;
  *ret_m             = mp + (mp < 10 ? 3 : -9);
  *ret_y             = y + (*ret_m <= 2);
}


void showDateTime(uint16_t ddate, uint8_t hour, uint8_t minutes, uint8_t seconds)
{
  int year, month, day;
  civil_from_days(ddate, &year, &month, &day);
  printf("Date = %d-%02d-%02d (stamp=%d)\r\n", year, month, day, ddate);
  hour = (hour & 0x0F) + ((hour >> 4) * 10);  // BCD to int
  minutes = (minutes & 0x0F) + ((minutes >> 4) * 10); // BCD to int
  seconds = (seconds & 0x0F) + ((seconds >> 4) * 10); // BCD to int
  printf("Time = %02d:%02d:%02d\r\n", hour, minutes, seconds);
}


// read date/time from SCB (using bdos get/set system control block)
void test1()
{
  struct SCBPB_t SCBPB;
  SCBPB.set = 0; // always read
  // Date in days in binary since 1 Jan 1978
  SCBPB.offset = 0x58;
  uint8_t ddate_low = bdos(49, &SCBPB) & 0xFF;
  SCBPB.offset = 0x59;
  uint8_t ddate_hi = bdos(49, &SCBPB) & 0xFF;
  uint16_t ddate = ddate_low | (ddate_hi << 8);
  // hour BCD
  SCBPB.offset = 0x5A;
  uint8_t hour = bdos(49, &SCBPB) & 0xFF;
  // minutes BCD
  SCBPB.offset = 0x5B;
  uint8_t minutes = bdos(49, &SCBPB) & 0xFF;
  // seconds BCD
  SCBPB.offset = 0x5C;
  uint8_t seconds = bdos(49, &SCBPB) & 0xFF;

  showDateTime(ddate, hour, minutes, seconds);
}


struct Datetime {
  uint16_t date;
  uint8_t  hour;
  uint8_t  minutes;
};


// read date/time using bdos func 105
void test2()
{
  struct Datetime DAT;
  int seconds = bdos(105, &DAT);
  showDateTime(DAT.date, DAT.hour, DAT.minutes, seconds);
}


int main()
{
  printf("\r\n** CLOCK Test **\r\n");
  printf("** v1.0 Feb 2020 - Fabrizio Di Vittorio\r\n");
  while (1) {
    printf("\r\n\nSelect test:\r\n");
    printf("1      -> Read date/time from SCB\r\n");
    printf("2      -> Read date/time from func 105\r\n");
    printf("Others -> Exit\r\n");
    printf("Please select > ");
    int c = toupper(getchar());
    printf("\r\n\n");
    switch (c) {
      case '1':
        test1();
        break;
      case '2':
        test2();
        break;
      default:
        printf("Bye!\r\n");
        return 0;
    }
  }
}
