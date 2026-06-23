#pragma once

/*************************************************
STX_MinMaxAvgDiff_Simple
Track min max avg and last diff between two values.
**************************************************/

class STX_MinMaxAvgDiff_Simple
{
public:
    void Push(float v1, float v2);
    char* last_diff_str();
    char* max_diff_str();

public:
    long   cnt = 0;
    float  last_diff = 0;
    float  min_diff = 0;
    float  max_diff = 0;
    double avg_diff = 0;

protected:
    // buffers to print floats
    char buf1[16];
    char buf2[16];
};
