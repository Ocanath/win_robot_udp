/*
 * sin-math.h
 *
 *  Created on: Oct 29, 2018
 *      Author: Ocanath
 */

#ifndef SIN_MATH_H_
#define SIN_MATH_H_
#include <stdio.h>
#include <stdint.h>


#define ONE_BY_THREE_FACTORIAL 	0.16666666666f
#define ONE_BY_FIVE_FACTORIAL 	0.00833333333f
#define HALF_PI 				1.57079632679f
#define PI						3.14159265359f
#define THREE_BY_TWO_PI     	4.71238898038f
#define TWO_PI              	6.28318530718f
#define ONE_BY_TWO_PI 			0.1591549f
#define ONE_BY_PI 0.318310f

#define ONE_BY_THREE_PI			1.0471975512f

#define DEG_TO_RAD				0.0174532925f
#define RAD_TO_DEG				57.2957795f

/*
Helper structure for the sin_fade_stitch subroutine which allows generation of arbitrarily changing 
frequency sinusoids that get stitched together. 
*/
typedef struct f_stitch_t
{
    float start_ts;
    float t_offset;
    float f_prev;
}f_stitch_t;    //helper variables for sinusoid frequency stitching

float abs_f(float v);
float fmod_2pi(float in);
float unwrap(float theta,float * prev_theta);
float atan2_approx(float sinVal, float cosVal);
float cos_fast(float theta);
float sin_fast(float theta);
float sin_fade_stitch(uint32_t tbase, float f, float phase, f_stitch_t * h);
float wrap_2pi(float v);

#endif /* SIN_MATH_H_ */
