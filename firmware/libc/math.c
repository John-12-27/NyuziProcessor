// 
// Copyright (C) 2014 Jeff Bush
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
// 
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
// Boston, MA  02110-1301, USA.
// 

#include "libc.h"

//
// Builtin math functions. 
//

unsigned int __udivsi3(unsigned int dividend, unsigned int divisor)
{
	if (dividend < divisor)
		return 0;

	int quotientBits = __builtin_clz(divisor) - __builtin_clz(dividend);
	divisor <<= quotientBits;
	unsigned int quotient = 0;
	do
	{
		quotient <<= 1;
		if (dividend >= divisor)
		{
			dividend -= divisor;
			quotient |= 1;
		}
		
		divisor >>= 1;
	}
	while (--quotientBits >= 0);

	return quotient;
}

int __divsi3(int value1, int value2)
{
	int sign1 = value1 >> 31;
	int sign2 = value2 >> 31;
	
	// Take absolute values
	unsigned int u_value1 = (value1 ^ sign1) - sign1;
	unsigned int u_value2 = (value2 ^ sign2) - sign2;

	// Compute result sign
	sign1 ^= sign2;

	// Perform division (will call __udivsi3), then convert sign back 
	return ((u_value1 / u_value2) ^ sign1) - sign1;
}

unsigned int __umodsi3(unsigned int value1, unsigned int value2)
{
	return value1 - __udivsi3(value1, value2) * value2;
}

int __modsi3(int value1, int value2)
{
	return value1 - __divsi3(value1, value2) * value2;
}

unsigned long long int __muldi3(unsigned long long int a, unsigned long long int b)
{
	unsigned int a_low = a;
	unsigned int b_low = b;
	unsigned int a_high = a >> 32;
	unsigned int b_high = b >> 32;

	unsigned long long int result = a_low * b_low;
	result += (unsigned long long int)(a_high * b_low + a_low * b_high) << 32;
	return result;
}

//
// Standard library math functions
//

int abs(int value)
{
	if (value < 0)
		return -value;
	
	return value;
}

double fmod(double val1, double val2)
{
	int whole = val1 / val2;
	return val1 - (whole * val2);
}

//
// Use taylor series to approximate sine
//   x**3/3! + x**5/5! - x**7/7! ...
//

const int kNumTerms = 7;

const double denominators[] = { 
	0.166666666666667f, 	// 1 / 3!
	0.008333333333333f,		// 1 / 5!
	0.000198412698413f,		// 1 / 7!
	0.000002755731922f,		// 1 / 9!
	2.50521084e-8f,			// 1 / 11!
	1.6059044e-10f,			// 1 / 13!
	7.6471637e-13f			// 1 / 15!
};

double sin(double angle)
{
	// More accurate if the angle is smaller. Constrain to 0-M_PI*2
	angle = fmod(angle, M_PI * 2.0f);

	double angleSquared = angle * angle;
	double numerator = angle;
	double result = angle;
	
	for (int i = 0; i < kNumTerms; i++)
	{
		numerator *= angleSquared;		
		double term = numerator * denominators[i];
		if (i & 1)
			result += term;
		else
			result -= term;
	}
	
	return result;
}

double cos(double angle)
{
	return sin(angle + M_PI * 0.5f);
}

double sqrt(double value)
{
	double guess = value;
	for (int iteration = 0; iteration < 10; iteration++)
		guess = ((value / guess) + guess) / 2.0f;

	return guess;
}
