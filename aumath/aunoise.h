//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aunoise.h,v 1.10 2022/02/19 18:38:02 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuNoise_H
#define AuNoise_H

#include "aumath.h"

/*
From: http://freespace.virgin.net/hugo.elias/models/m_perlin.htm
Found similar information at: http://libnoise.sourceforge.net/noisegen/index.html
My final result is a blend of the two. The hugo.elias examples appeared to be based on the libnoise examples at SourceForge but the
hugo.elias examples contained numerous errors.

-----------
Persistence 
-----------

When you're adding together these noise functions, you may wonder exactly what amplitude and frequency to use for each one. The one
dimensional example above used twice the frequency and half the amplitude for each successive noise function added. This is quite
common. So common in fact, that many people don't even consider using anything else. However, you can create Perlin Noise functions
with different characteristics by using other frequencies and amplitudes at each step. For example, to create smooth rolling hills,
you could use Perlin noise function with large amplitudes for the low frequencies , and very small amplitudes for the higher
frequencies. Or you could make a flat, but very rocky plane choosing low amplitudes for low frequencies.

To make it simpler, and to avoid repeating the words Amplitude and Frequency all the time, a single number is used to specify the
amplitude of each frequency. This value is known as Persistence. There is some ambiguity as to its exact meaning. The term was
originally coined by Mandelbrot, one of the people behind the discovery of fractals. He defined noise with a lot of high
frequency as having a low persistence. My friend Matt also came up with the concept of persistence, but defined it the other way
round. To be honest, I prefer Matt's definition. Sorry Mandelbrot. So our definition of persistence is this:

	frequency = 2^i
	amplitude = persistence^i

Where i is the ith noise function being added. To illustrate the effect of persistence on the output of the Perlin Noise, take a
look at the diagrams below. They show the component noise functions that are added, the effect of the persistence value, and the
resultant Perlin noise function.

-------
Octaves
-------

Each successive noise function you add is known as an octave. The reason for this is that each noise function is twice the
frequency of the previous one. In music, octaves also have this property. 

Exactly how many octaves you add together is entirely up to you. You may add as many or as few as you want. However, let me give
you some suggestions. If you are using the perlin noise function to render an image to the screen, there will come a point when an
octave has too high a frequency to be displayable. There simply may not be enough pixels on the screen to reproduce all the little
details of a very high frequency noise function. Some implementations of Perlin Noise automatically add up as many noise functions
they can until the limits of the screen (or other medium) are reached. 

It is also wise to stop adding noise functions when their amplitude becomes too small to reproduce. Exactly when that happens
depends on the level of persistence, the overall amplitude of the Perlin function and the bit resolution of your screen (or
whatever).
*/

class AUMATH_DLL AuNoiseF
{
public:
	AuNoiseF();
	~AuNoiseF();

public:
	// 1D coherent noise using IntegerNoise and Cosine_Interpolate interpolation.
	// 'randFunc' may be in the range 0..11.
	// Returns a value from -1 to 1.
	static float CoherentNoise(unsigned int randFunc, float x);

	// 2D coherent noise using IntegerNoise and Cosine_Interpolation.
	// 'randFunc' may be in the range 0..11.
	// Returns a value from -1 to 1.
	static float CoherentNoise(unsigned int randFunc, float x, float y);

	// Cosine curve interpolation function.
	static float Cosine_Interpolate(float a, float b, float x);

	// 1D coherent noise using IntegerNoise and CubicSCurve interpolation.
	// 'randFunc' may be in the range 0..11.
	static float CubicCoherentNoise(unsigned int randFunc, float x);

	// 2D coherent noise using IntegerNoise and CubicSCurve interpolation.
	// 'randFunc' may be in the range 0..11.
	static float CubicCoherentNoise(unsigned int randFunc, float x, float y);

	// 1D Perlin noise using CubicCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static float CubicPerlinNoise(unsigned int number_of_octaves, float persistence, float x);

	// 2D Perlin noise using CubicCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static float CubicPerlinNoise(unsigned int number_of_octaves, float persistence, float x, float y);

	// Cubic S-curve interpolation function.
	static float CubicSCurve(float weight);

	// Exponential curve interpolation function.
	static float ExponentialCurve(float weight, float exponent=2.f);

	// 1D noise.
	// 'randFunc' may be in the range 0..11.
	// Returns a value from -1 to 1.
	static float IntegerNoise(unsigned int randFunc, int x);

	// 2D noise.
	// 'randFunc' may be in the range 0..11.
	// Returns a value from -1 to 1.
	static float IntegerNoise(unsigned int randFunc, int x, int y);

	// Linear interpolation function.
	static float Linear_Interpolate(float a, float b, float x);

	// 1D perlin noise using CoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static float PerlinNoise(unsigned int number_of_octaves, float persistence, float x);

	// 2D perlin noise using CoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static float PerlinNoise(unsigned int number_of_octaves, float persistence, float x, float y);

	// Returns the maximum value returned by the PerlinNoise function for a given 'number_of_octaves' and 'persistence'.
	// 'number_of_octaves' may be in the range 1..12.
	static float PerlinNoiseRange(unsigned int number_of_octaves, float persistence);

	// 1D coherent noise using IntegerNoise and QuinticSCurve interpolation.
	// 'randFunc' may be in the range 0..11.
	static float QuinticCoherentNoise(unsigned int randFunc, float x);

	// 2D coherent noise using IntegerNoise and QuinticSCurve interpolation.
	// 'randFunc' may be in the range 0..11.
	static float QuinticCoherentNoise(unsigned int randFunc, float x, float y);

	// 1D Perlin noise using QuinticCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static float QuinticPerlinNoise(unsigned int number_of_octaves, float persistence, float x);

	// 2D Perlin noise using QuinticCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static float QuinticPerlinNoise(unsigned int number_of_octaves, float persistence, float x, float y);

	// Quintic S-curve interpolation function.
	static float QuinticSCurve(float weight);

	// Returns a pseudo-random number from -1 to 1 that corresponds to the 'seed' value.
	// 'smallPrime', 'mediumPrime', 'bigPrime' are prime numbers.
	static float Random(int seed, int smallPrime, int mediumPrime, int bigPrime);

	// 1D coherent noise using SmoothNoise and Cosine_Interpolate interpolation.
	// 'randFunc' may be in the range 0..11.
	static float SmoothCoherentNoise(unsigned int randFunc, float x);

	// 2D coherent noise using SmoothNoise and Cosine_Interpolation.
	// 'randFunc' may be in the range 0..11.
	static float SmoothCoherentNoise(unsigned int randFunc, float x, float y);

	// 1D coherent noise using SmoothNoise and CubicSCurve interpolation.
	// 'randFunc' may be in the range 0..11.
	static float SmoothCubicCoherentNoise(unsigned int randFunc, float x);

	// 2D coherent noise using SmoothNoise and CubicSCurve interpolation.
	// 'randFunc' may be in the range 0..11.
	static float SmoothCubicCoherentNoise(unsigned int randFunc, float x, float y);

	// 1D Perlin noise using SmoothCubicCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static float SmoothCubicPerlinNoise(unsigned int number_of_octaves, float persistence, float x);

	// 2D Perlin noise using SmoothCubicCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static float SmoothCubicPerlinNoise(unsigned int number_of_octaves, float persistence, float x, float y);

	// 1D noise with some neighboring noise averaged in for smoothness.
	// 'randFunc' may be in the range 0..11.
	static float SmoothNoise(unsigned int randFunc, int x);

	// 2D noise with some neighboring noise averaged in for smoothness.
	// 'randFunc' may be in the range 0..11.
	static float SmoothNoise(unsigned int randFunc, int x, int y);

	// 1D perlin noise using SmoothCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static float SmoothPerlinNoise(unsigned int number_of_octaves, float persistence, float x);

	// 2D perlin noise using SmoothCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static float SmoothPerlinNoise(unsigned int number_of_octaves, float persistence, float x, float y);

	// 1D coherent noise using SmoothNoise and QuinticSCurve interpolation.
	// 'randFunc' may be in the range 0..11.
	static float SmoothQuinticCoherentNoise(unsigned int randFunc, float x);

	// 2D coherent noise using SmoothNoise and QuinticSCurve interpolation.
	// 'randFunc' may be in the range 0..11.
	static float SmoothQuinticCoherentNoise(unsigned int randFunc, float x, float y);

	// 1D Perlin noise using SmoothQuinticCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static float SmoothQuinticPerlinNoise(unsigned int number_of_octaves, float persistence, float x);

	// 2D Perlin noise using SmoothQuinticCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static float SmoothQuinticPerlinNoise(unsigned int number_of_octaves, float persistence, float x, float y);
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

class AUMATH_DLL AuNoise
{
public:
	AuNoise();
	~AuNoise();

public:
	// 1D coherent noise using IntegerNoise and Cosine_Interpolate interpolation.
	// 'randFunc' may be in the range 0..11.
	// Returns a value from -1 to 1.
	static double CoherentNoise(unsigned int randFunc, double x);

	// 2D coherent noise using IntegerNoise and Cosine_Interpolation.
	// 'randFunc' may be in the range 0..11.
	// Returns a value from -1 to 1.
	static double CoherentNoise(unsigned int randFunc, double x, double y);

	// Cosine curve interpolation function.
	static double Cosine_Interpolate(double a, double b, double x);

	// 1D coherent noise using IntegerNoise and CubicSCurve interpolation.
	// 'randFunc' may be in the range 0..11.
	static double CubicCoherentNoise(unsigned int randFunc, double x);

	// 2D coherent noise using IntegerNoise and CubicSCurve interpolation.
	// 'randFunc' may be in the range 0..11.
	static double CubicCoherentNoise(unsigned int randFunc, double x, double y);

	// 1D Perlin noise using CubicCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static double CubicPerlinNoise(unsigned int number_of_octaves, double persistence, double x);

	// 2D Perlin noise using CubicCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static double CubicPerlinNoise(unsigned int number_of_octaves, double persistence, double x, double y);

	// Cubic S-curve interpolation function.
	static double CubicSCurve(double weight);

	// Exponential curve interpolation function.
	static double ExponentialCurve(double weight, double exponent=2.);

	// 1D noise.
	// 'randFunc' may be in the range 0..11.
	// Returns a value from -1 to 1.
	static double IntegerNoise(unsigned int randFunc, int x);

	// 2D noise.
	// 'randFunc' may be in the range 0..11.
	// Returns a value from -1 to 1.
	static double IntegerNoise(unsigned int randFunc, int x, int y);

	// Linear interpolation function.
	static double Linear_Interpolate(double a, double b, double x);

	// 1D perlin noise using CoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static double PerlinNoise(unsigned int number_of_octaves, double persistence, double x);

	// 2D perlin noise using CoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static double PerlinNoise(unsigned int number_of_octaves, double persistence, double x, double y);

	// Returns the maximum value returned by the PerlinNoise function for a given 'number_of_octaves' and 'persistence'.
	// 'number_of_octaves' may be in the range 1..12.
	static double PerlinNoiseRange(unsigned int number_of_octaves, double persistence);

	// 1D coherent noise using IntegerNoise and QuinticSCurve interpolation.
	// 'randFunc' may be in the range 0..11.
	static double QuinticCoherentNoise(unsigned int randFunc, double x);

	// 2D coherent noise using IntegerNoise and QuinticSCurve interpolation.
	// 'randFunc' may be in the range 0..11.
	static double QuinticCoherentNoise(unsigned int randFunc, double x, double y);

	// 1D Perlin noise using QuinticCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static double QuinticPerlinNoise(unsigned int number_of_octaves, double persistence, double x);

	// 2D Perlin noise using QuinticCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static double QuinticPerlinNoise(unsigned int number_of_octaves, double persistence, double x, double y);

	// Quintic S-curve interpolation function.
	static double QuinticSCurve(double weight);

	// Returns a pseudo-random number from -1 to 1 that corresponds to the 'seed' value.
	// 'smallPrime', 'mediumPrime', 'bigPrime' are prime numbers.
	static double Random(int seed, int smallPrime, int mediumPrime, int bigPrime);

	// 1D coherent noise using SmoothNoise and Cosine_Interpolate interpolation.
	// 'randFunc' may be in the range 0..11.
	static double SmoothCoherentNoise(unsigned int randFunc, double x);

	// 2D coherent noise using SmoothNoise and Cosine_Interpolation.
	// 'randFunc' may be in the range 0..11.
	static double SmoothCoherentNoise(unsigned int randFunc, double x, double y);

	// 1D coherent noise using SmoothNoise and CubicSCurve interpolation.
	// 'randFunc' may be in the range 0..11.
	static double SmoothCubicCoherentNoise(unsigned int randFunc, double x);

	// 2D coherent noise using SmoothNoise and CubicSCurve interpolation.
	// 'randFunc' may be in the range 0..11.
	static double SmoothCubicCoherentNoise(unsigned int randFunc, double x, double y);

	// 1D Perlin noise using SmoothCubicCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static double SmoothCubicPerlinNoise(unsigned int number_of_octaves, double persistence, double x);

	// 2D Perlin noise using SmoothCubicCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static double SmoothCubicPerlinNoise(unsigned int number_of_octaves, double persistence, double x, double y);

	// 1D noise with some neighboring noise averaged in for smoothness.
	// 'randFunc' may be in the range 0..11.
	static double SmoothNoise(unsigned int randFunc, int x);

	// 2D noise with some neighboring noise averaged in for smoothness.
	// 'randFunc' may be in the range 0..11.
	static double SmoothNoise(unsigned int randFunc, int x, int y);

	// 1D perlin noise using SmoothCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static double SmoothPerlinNoise(unsigned int number_of_octaves, double persistence, double x);

	// 2D perlin noise using SmoothCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static double SmoothPerlinNoise(unsigned int number_of_octaves, double persistence, double x, double y);

	// 1D coherent noise using SmoothNoise and QuinticSCurve interpolation.
	// 'randFunc' may be in the range 0..11.
	static double SmoothQuinticCoherentNoise(unsigned int randFunc, double x);

	// 2D coherent noise using SmoothNoise and QuinticSCurve interpolation.
	// 'randFunc' may be in the range 0..11.
	static double SmoothQuinticCoherentNoise(unsigned int randFunc, double x, double y);

	// 1D Perlin noise using SmoothQuinticCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static double SmoothQuinticPerlinNoise(unsigned int number_of_octaves, double persistence, double x);

	// 2D Perlin noise using SmoothQuinticCoherentNoise.
	// 'number_of_octaves' may be in the range 1..12.
	// Returns a value from -PerlinNoiseRange to +PerlinNoiseRange.
	static double SmoothQuinticPerlinNoise(unsigned int number_of_octaves, double persistence, double x, double y);
};

#endif // AuNoise_H
