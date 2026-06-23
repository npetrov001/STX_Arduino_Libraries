#include <Arduino.h>

#include "STX_MinMaxAvgDiff_Simple.h"

void STX_MinMaxAvgDiff_Simple::Push(float v1, float v2)
{
    last_diff = v1 - v2;
    if (cnt == 0)
    {
        min_diff = last_diff;
        max_diff = last_diff;
        avg_diff = last_diff;
        cnt = 1;
    }
    else
    {
        if (last_diff < min_diff) min_diff = last_diff;
        if (last_diff > max_diff) max_diff = last_diff;
        // long-term stable average calc
        avg_diff = (avg_diff * cnt + last_diff) / (cnt + 1);
        // increment
        cnt++;
    }
}

// float diff with 1 decimal precision as a string - for printing floats
char* STX_MinMaxAvgDiff_Simple::last_diff_str()
{
    return dtostrf(last_diff, 1, 1, buf1);
}

// float diff with 1 decimal precision as a string - for printing floats
char* STX_MinMaxAvgDiff_Simple::max_diff_str()
{
    float v = fmaxf(fabsf(min_diff), fabsf(max_diff));
    return dtostrf(v, 1, 1, buf2);
}
