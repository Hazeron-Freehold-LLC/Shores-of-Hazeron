//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aunoise.cpp,v 1.7 2022/02/19 18:38:14 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#include "aunoise.h"
//

// Tables of prime numbers are used by the Random function. All tables must contain the same number of values.
static int s_TinyPrime[] =
{
	13,
	17,
	19,
	23,
	29,
	31,
	37,
	41,
	43,
	47,
	53,
	59,
};

static int s_SmallPrime[] =
{
	15629,
	15641,
	15643,
	15647,
	15649,
	15661,
	15667,
	15671,
	15679,
	15683,
	15727,
	15731,
};

static int s_MediumPrime[] =
{
	754907,
	754921,
	754931,
	754937,
	754939,
	754967,
	754969,
	754973,
	754979,
	754981,
	754991,
	789221,
};

static int s_BigPrime[] =
{
	1372950023,
	1372950043,
	1372950049,
	1372950071,
	1372950077,
	1372950101,
	1372950133,
	1372950169,
	1372950191,
	1372950193,
	1372950221,
	1372950233,
	//1376312589,	// NOT A PRIME NUMBER
};

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuNoiseF::AuNoiseF()
{
}

AuNoiseF::~AuNoiseF()
{
}

// "Looks awful, like those cheap 'plasmas' that everyone uses to generate landscapes. It's a simple algorithm though, and I 
// suppose would be excusable if you were trying to do perlin noise in realtime."
float AuNoiseF::Linear_Interpolate(float a, float b, float x)
{
	return a*(1-x) + b*x;
}

// "This method gives a much smother curve than Linear Interpolation. It's clearly better and worth the effort if you can afford
// the very slight loss in speed."
float AuNoiseF::Cosine_Interpolate(float a, float b, float x)
{
	float ft = x * 3.141592653589793238462f;
	float f = (1.f - cosf(ft)) * .5f;
	return a*(1.f - f) + b*f;
}

float AuNoiseF::IntegerNoise(unsigned int randFunc, int x)
{
	int n = (x<<13) ^ x;
	return Random(n, s_SmallPrime[randFunc], s_MediumPrime[randFunc], s_BigPrime[randFunc]);
}

float AuNoiseF::IntegerNoise(unsigned int randFunc, int x, int y)
{
	int n = x + y * s_TinyPrime[randFunc];
	n = (n<<13) ^ n;
	return Random(n, s_SmallPrime[randFunc], s_MediumPrime[randFunc], s_BigPrime[randFunc]);
}

float AuNoiseF::Random(int seed, int smallPrime, int medPrime, int bigPrime)
{
	int nn = (seed * (seed * seed * smallPrime + medPrime) + bigPrime) & 0x7fffffff;
	return 1.f - (float)nn / 1073741824.f;
}

float AuNoiseF::SmoothNoise(unsigned int randFunc, int x)
{
	return IntegerNoise(randFunc, x)/2 + IntegerNoise(randFunc, x-1)/4 + IntegerNoise(randFunc, x+1)/4;
}

float AuNoiseF::SmoothNoise(unsigned int randFunc, int x, int y)
{
	float corners = (IntegerNoise(randFunc, x-1, y-1) +
			IntegerNoise(randFunc, x+1, y-1) +
			IntegerNoise(randFunc, x-1, y+1) +
			IntegerNoise(randFunc, x+1, y+1)) / 16;
	float sides = (IntegerNoise(randFunc, x-1, y) +
			IntegerNoise(randFunc, x+1, y) +
			IntegerNoise(randFunc, x, y-1) +
			IntegerNoise(randFunc, x, y+1)) / 8;
	float center = IntegerNoise(randFunc, x, y) / 4;
	return corners + sides + center;
}

float AuNoiseF::ExponentialCurve(float weight, float exponent)
{
	return powf(weight, exponent);
}

float AuNoiseF::CubicSCurve(float weight)
{
	return -2*powf(weight, 3) + 3*powf(weight, 2);
}

float AuNoiseF::QuinticSCurve(float weight)
{
	return 6*powf(weight,6) - 15*powf(weight,4) + 10*powf(weight,3);
}

float AuNoiseF::CubicCoherentNoise(unsigned int randFunc, float x)
{
	int integer_X = (int)floorf(x);
	float fractional_X = x - integer_X;

	float v1 = IntegerNoise(randFunc, integer_X);
	float v2 = IntegerNoise(randFunc, integer_X + 1);

	return Cosine_Interpolate(v1, v2, CubicSCurve(fractional_X));
}

float AuNoiseF::CubicCoherentNoise(unsigned int randFunc, float x, float y)
{
	int integer_X = (int)floorf(x);
	float fractional_X = x - integer_X;

	int integer_Y = (int)floorf(y);
	float fractional_Y = y - integer_Y;

	float v1 = IntegerNoise(randFunc, integer_X, integer_Y);
	float v2 = IntegerNoise(randFunc, integer_X + 1, integer_Y);
	float v3 = IntegerNoise(randFunc, integer_X, integer_Y + 1);
	float v4 = IntegerNoise(randFunc, integer_X + 1, integer_Y + 1);

	float i1 = Cosine_Interpolate(v1, v2, CubicSCurve(fractional_X));
	float i2 = Cosine_Interpolate(v3, v4, CubicSCurve(fractional_X));

	return Cosine_Interpolate(i1, i2, CubicSCurve(fractional_Y));
}

float AuNoiseF::QuinticCoherentNoise(unsigned int randFunc, float x)
{
	int integer_X = (int)floorf(x);
	float fractional_X = x - integer_X;

	float v1 = IntegerNoise(randFunc, integer_X);
	float v2 = IntegerNoise(randFunc, integer_X + 1);

	return Cosine_Interpolate(v1, v2, QuinticSCurve(fractional_X));
}

float AuNoiseF::QuinticCoherentNoise(unsigned int randFunc, float x, float y)
{
	int integer_X = (int)floorf(x);
	float fractional_X = x - integer_X;

	int integer_Y = (int)floorf(y);
	float fractional_Y = y - integer_Y;

	float v1 = IntegerNoise(randFunc, integer_X, integer_Y);
	float v2 = IntegerNoise(randFunc, integer_X + 1, integer_Y);
	float v3 = IntegerNoise(randFunc, integer_X, integer_Y + 1);
	float v4 = IntegerNoise(randFunc, integer_X + 1, integer_Y + 1);

	float i1 = Cosine_Interpolate(v1, v2, QuinticSCurve(fractional_X));
	float i2 = Cosine_Interpolate(v3, v4, QuinticSCurve(fractional_X));

	return Cosine_Interpolate(i1, i2, QuinticSCurve(fractional_Y));
}

float AuNoiseF::CoherentNoise(unsigned int randFunc, float x)
{
	int integer_X = (int)floorf(x);
	float fractional_X = x - integer_X;

	float v1 = IntegerNoise(randFunc, integer_X);
	float v2 = IntegerNoise(randFunc, integer_X + 1);

	return Cosine_Interpolate(v1, v2, fractional_X);
}

float AuNoiseF::CoherentNoise(unsigned int randFunc, float x, float y)
{
	int integer_X = (int)floorf(x);
	float fractional_X = x - integer_X;

	int integer_Y = (int)floorf(y);
	float fractional_Y = y - integer_Y;

	float v1 = IntegerNoise(randFunc, integer_X, integer_Y);
	float v2 = IntegerNoise(randFunc, integer_X + 1, integer_Y);
	float v3 = IntegerNoise(randFunc, integer_X, integer_Y + 1);
	float v4 = IntegerNoise(randFunc, integer_X + 1, integer_Y + 1);

	float i1 = Cosine_Interpolate(v1, v2, fractional_X);
	float i2 = Cosine_Interpolate(v3, v4, fractional_X);

	return Cosine_Interpolate(i1, i2, fractional_Y);
}

float AuNoiseF::SmoothCubicCoherentNoise(unsigned int randFunc, float x)
{
	int integer_X = (int)floorf(x);
	float fractional_X = x - integer_X;

	float v1 = SmoothNoise(randFunc, integer_X);
	float v2 = SmoothNoise(randFunc, integer_X + 1);

	return Cosine_Interpolate(v1, v2, CubicSCurve(fractional_X));
}

float AuNoiseF::SmoothCubicCoherentNoise(unsigned int randFunc, float x, float y)
{
	int integer_X = (int)floorf(x);
	float fractional_X = x - integer_X;

	int integer_Y = (int)floorf(y);
	float fractional_Y = y - integer_Y;

	float v1 = SmoothNoise(randFunc, integer_X, integer_Y);
	float v2 = SmoothNoise(randFunc, integer_X + 1, integer_Y);
	float v3 = SmoothNoise(randFunc, integer_X, integer_Y + 1);
	float v4 = SmoothNoise(randFunc, integer_X + 1, integer_Y + 1);

	float i1 = Cosine_Interpolate(v1, v2, CubicSCurve(fractional_X));
	float i2 = Cosine_Interpolate(v3, v4, CubicSCurve(fractional_X));

	return Cosine_Interpolate(i1, i2, CubicSCurve(fractional_Y));
}

float AuNoiseF::SmoothQuinticCoherentNoise(unsigned int randFunc, float x)
{
	int integer_X = (int)floorf(x);
	float fractional_X = x - integer_X;

	float v1 = SmoothNoise(randFunc, integer_X);
	float v2 = SmoothNoise(randFunc, integer_X + 1);

	return Cosine_Interpolate(v1, v2, QuinticSCurve(fractional_X));
}

float AuNoiseF::SmoothQuinticCoherentNoise(unsigned int randFunc, float x, float y)
{
	int integer_X = (int)floorf(x);
	float fractional_X = x - integer_X;

	int integer_Y = (int)floorf(y);
	float fractional_Y = y - integer_Y;

	float v1 = SmoothNoise(randFunc, integer_X, integer_Y);
	float v2 = SmoothNoise(randFunc, integer_X + 1, integer_Y);
	float v3 = SmoothNoise(randFunc, integer_X, integer_Y + 1);
	float v4 = SmoothNoise(randFunc, integer_X + 1, integer_Y + 1);

	float i1 = Cosine_Interpolate(v1, v2, QuinticSCurve(fractional_X));
	float i2 = Cosine_Interpolate(v3, v4, QuinticSCurve(fractional_X));

	return Cosine_Interpolate(i1, i2, QuinticSCurve(fractional_Y));
}

float AuNoiseF::SmoothCoherentNoise(unsigned int randFunc, float x)
{
	int integer_X = (int)floorf(x);
	float fractional_X = x - integer_X;

	float v1 = SmoothNoise(randFunc, integer_X);
	float v2 = SmoothNoise(randFunc, integer_X + 1);

	return Cosine_Interpolate(v1, v2, fractional_X);
}

float AuNoiseF::SmoothCoherentNoise(unsigned int randFunc, float x, float y)
{
	int integer_X = (int)floorf(x);
	float fractional_X = x - integer_X;

	int integer_Y = (int)floorf(y);
	float fractional_Y = y - integer_Y;

	float v1 = SmoothNoise(randFunc, integer_X, integer_Y);
	float v2 = SmoothNoise(randFunc, integer_X + 1, integer_Y);
	float v3 = SmoothNoise(randFunc, integer_X, integer_Y + 1);
	float v4 = SmoothNoise(randFunc, integer_X + 1, integer_Y + 1);

	float i1 = Cosine_Interpolate(v1, v2, fractional_X);
	float i2 = Cosine_Interpolate(v3, v4, fractional_X);

	return Cosine_Interpolate(i1, i2, fractional_Y);
}

float AuNoiseF::PerlinNoise(unsigned int number_of_octaves, float persistence, float x)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	float total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		float frequency = powf(2.f, octave);
		float amplitude = powf(persistence, octave);
		total += CoherentNoise(octave, x * frequency) * amplitude;
	}
	return total;
}

float AuNoiseF::PerlinNoise(unsigned int number_of_octaves, float persistence, float x, float y)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	float total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		float frequency = powf(2.f, octave);
		float amplitude = powf(persistence, octave);
		total += CoherentNoise(octave, x * frequency, y * frequency) * amplitude;
	}
	return total;
}

// Returns the maximum value returned by the PerlinNoise function for a given 'persistence'.
float AuNoiseF::PerlinNoiseRange(unsigned int number_of_octaves, float persistence)
{
	float total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		float amplitude = powf(persistence, octave);
		total += amplitude;
	}
	return total;
}

float AuNoiseF::CubicPerlinNoise(unsigned int number_of_octaves, float persistence, float x)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	float total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		float frequency = powf(2.f, octave);
		float amplitude = powf(persistence, octave);
		total += CubicCoherentNoise(octave, x * frequency) * amplitude;
	}
	return total;
}

float AuNoiseF::CubicPerlinNoise(unsigned int number_of_octaves, float persistence, float x, float y)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	float total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		float frequency = powf(2.f, octave);
		float amplitude = powf(persistence, octave);
		total += CubicCoherentNoise(octave, x * frequency, y * frequency) * amplitude;
	}
	return total;
}

float AuNoiseF::QuinticPerlinNoise(unsigned int number_of_octaves, float persistence, float x)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	float total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		float frequency = powf(2.f, octave);
		float amplitude = powf(persistence, octave);
		total += QuinticCoherentNoise(octave, x * frequency) * amplitude;
	}
	return total;
}

float AuNoiseF::QuinticPerlinNoise(unsigned int number_of_octaves, float persistence, float x, float y)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	float total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		float frequency = powf(2.f, octave);
		float amplitude = powf(persistence, octave);
		total += QuinticCoherentNoise(octave, x * frequency, y * frequency) * amplitude;
	}
	return total;
}

float AuNoiseF::SmoothPerlinNoise(unsigned int number_of_octaves, float persistence, float x)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	float total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		float frequency = powf(2.f, octave);
		float amplitude = powf(persistence, octave);
		total += SmoothCoherentNoise(octave, x * frequency) * amplitude;
	}
	return total;
}

float AuNoiseF::SmoothPerlinNoise(unsigned int number_of_octaves, float persistence, float x, float y)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	float total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		float frequency = powf(2.f, octave);
		float amplitude = powf(persistence, octave);
		total += SmoothCoherentNoise(octave, x * frequency, y * frequency) * amplitude;
	}
	return total;
}

float AuNoiseF::SmoothCubicPerlinNoise(unsigned int number_of_octaves, float persistence, float x)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	float total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		float frequency = powf(2.f, octave);
		float amplitude = powf(persistence, octave);
		total += SmoothCubicCoherentNoise(octave, x * frequency) * amplitude;
	}
	return total;
}

float AuNoiseF::SmoothCubicPerlinNoise(unsigned int number_of_octaves, float persistence, float x, float y)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	float total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		float frequency = powf(2.f, octave);
		float amplitude = powf(persistence, octave);
		total += SmoothCubicCoherentNoise(octave, x * frequency, y * frequency) * amplitude;
	}
	return total;
}

float AuNoiseF::SmoothQuinticPerlinNoise(unsigned int number_of_octaves, float persistence, float x)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	float total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		float frequency = powf(2.f, octave);
		float amplitude = powf(persistence, octave);
		total += SmoothQuinticCoherentNoise(octave, x * frequency) * amplitude;
	}
	return total;
}

float AuNoiseF::SmoothQuinticPerlinNoise(unsigned int number_of_octaves, float persistence, float x, float y)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	float total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		float frequency = powf(2.f, octave);
		float amplitude = powf(persistence, octave);
		total += SmoothQuinticCoherentNoise(octave, x * frequency, y * frequency) * amplitude;
	}
	return total;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

AuNoise::AuNoise()
{
}

AuNoise::~AuNoise()
{
}

// "Looks awful, like those cheap 'plasmas' that everyone uses to generate landscapes. It's a simple algorithm though, and I 
// suppose would be excusable if you were trying to do perlin noise in realtime."
double AuNoise::Linear_Interpolate(double a, double b, double x)
{
	return a*(1-x) + b*x;
}

// "This method gives a much smother curve than Linear Interpolation. It's clearly better and worth the effort if you can afford
// the very slight loss in speed."
double AuNoise::Cosine_Interpolate(double a, double b, double x)
{
	double ft = x * 3.141592653589793238462;
	double f = (1. - cos(ft)) * .5;
	return a*(1. - f) + b*f;
}

double AuNoise::IntegerNoise(unsigned int randFunc, int x)
{
	int n = (x<<13) ^ x;
	return Random(n, s_SmallPrime[randFunc], s_MediumPrime[randFunc], s_BigPrime[randFunc]);
}

double AuNoise::IntegerNoise(unsigned int randFunc, int x, int y)
{
	int n = x + y * s_TinyPrime[randFunc];
	n = (n<<13) ^ n;
	return Random(n, s_SmallPrime[randFunc], s_MediumPrime[randFunc], s_BigPrime[randFunc]);
}

double AuNoise::Random(int seed, int smallPrime, int medPrime, int bigPrime)
{
	int nn = (seed * (seed * seed * smallPrime + medPrime) + bigPrime) & 0x7fffffff;
	return 1. - (double)nn / 1073741824.;
}

double AuNoise::SmoothNoise(unsigned int randFunc, int x)
{
	return IntegerNoise(randFunc, x)/2 + IntegerNoise(randFunc, x-1)/4 + IntegerNoise(randFunc, x+1)/4;
}

double AuNoise::SmoothNoise(unsigned int randFunc, int x, int y)
{
	double corners = (IntegerNoise(randFunc, x-1, y-1) +
			IntegerNoise(randFunc, x+1, y-1) +
			IntegerNoise(randFunc, x-1, y+1) +
			IntegerNoise(randFunc, x+1, y+1)) / 16;
	double sides = (IntegerNoise(randFunc, x-1, y) +
			IntegerNoise(randFunc, x+1, y) +
			IntegerNoise(randFunc, x, y-1) +
			IntegerNoise(randFunc, x, y+1)) / 8;
	double center = IntegerNoise(randFunc, x, y) / 4;
	return corners + sides + center;
}

double AuNoise::ExponentialCurve(double weight, double exponent)
{
	return pow(weight, exponent);
}

double AuNoise::CubicSCurve(double weight)
{
	return -2*pow(weight, 3) + 3*pow(weight, 2);
}

double AuNoise::QuinticSCurve(double weight)
{
	return 6*pow(weight,6) - 15*pow(weight,4) + 10*pow(weight,3);
}

double AuNoise::CubicCoherentNoise(unsigned int randFunc, double x)
{
	int integer_X = (int)floor(x);
	double fractional_X = x - integer_X;

	double v1 = IntegerNoise(randFunc, integer_X);
	double v2 = IntegerNoise(randFunc, integer_X + 1);

	return Cosine_Interpolate(v1, v2, CubicSCurve(fractional_X));
}

double AuNoise::CubicCoherentNoise(unsigned int randFunc, double x, double y)
{
	int integer_X = (int)floor(x);
	double fractional_X = x - integer_X;

	int integer_Y = (int)floor(y);
	double fractional_Y = y - integer_Y;

	double v1 = IntegerNoise(randFunc, integer_X, integer_Y);
	double v2 = IntegerNoise(randFunc, integer_X + 1, integer_Y);
	double v3 = IntegerNoise(randFunc, integer_X, integer_Y + 1);
	double v4 = IntegerNoise(randFunc, integer_X + 1, integer_Y + 1);

	double i1 = Cosine_Interpolate(v1, v2, CubicSCurve(fractional_X));
	double i2 = Cosine_Interpolate(v3, v4, CubicSCurve(fractional_X));

	return Cosine_Interpolate(i1, i2, CubicSCurve(fractional_Y));
}

double AuNoise::QuinticCoherentNoise(unsigned int randFunc, double x)
{
	int integer_X = (int)floor(x);
	double fractional_X = x - integer_X;

	double v1 = IntegerNoise(randFunc, integer_X);
	double v2 = IntegerNoise(randFunc, integer_X + 1);

	return Cosine_Interpolate(v1, v2, QuinticSCurve(fractional_X));
}

double AuNoise::QuinticCoherentNoise(unsigned int randFunc, double x, double y)
{
	int integer_X = (int)floor(x);
	double fractional_X = x - integer_X;

	int integer_Y = (int)floor(y);
	double fractional_Y = y - integer_Y;

	double v1 = IntegerNoise(randFunc, integer_X, integer_Y);
	double v2 = IntegerNoise(randFunc, integer_X + 1, integer_Y);
	double v3 = IntegerNoise(randFunc, integer_X, integer_Y + 1);
	double v4 = IntegerNoise(randFunc, integer_X + 1, integer_Y + 1);

	double i1 = Cosine_Interpolate(v1, v2, QuinticSCurve(fractional_X));
	double i2 = Cosine_Interpolate(v3, v4, QuinticSCurve(fractional_X));

	return Cosine_Interpolate(i1, i2, QuinticSCurve(fractional_Y));
}

double AuNoise::CoherentNoise(unsigned int randFunc, double x)
{
	int integer_X = (int)floor(x);
	double fractional_X = x - integer_X;

	double v1 = IntegerNoise(randFunc, integer_X);
	double v2 = IntegerNoise(randFunc, integer_X + 1);

	return Cosine_Interpolate(v1, v2, fractional_X);
}

double AuNoise::CoherentNoise(unsigned int randFunc, double x, double y)
{
	int integer_X = (int)floor(x);
	double fractional_X = x - integer_X;

	int integer_Y = (int)floor(y);
	double fractional_Y = y - integer_Y;

	double v1 = IntegerNoise(randFunc, integer_X, integer_Y);
	double v2 = IntegerNoise(randFunc, integer_X + 1, integer_Y);
	double v3 = IntegerNoise(randFunc, integer_X, integer_Y + 1);
	double v4 = IntegerNoise(randFunc, integer_X + 1, integer_Y + 1);

	double i1 = Cosine_Interpolate(v1, v2, fractional_X);
	double i2 = Cosine_Interpolate(v3, v4, fractional_X);

	return Cosine_Interpolate(i1, i2, fractional_Y);
}

double AuNoise::SmoothCubicCoherentNoise(unsigned int randFunc, double x)
{
	int integer_X = (int)floor(x);
	double fractional_X = x - integer_X;

	double v1 = SmoothNoise(randFunc, integer_X);
	double v2 = SmoothNoise(randFunc, integer_X + 1);

	return Cosine_Interpolate(v1, v2, CubicSCurve(fractional_X));
}

double AuNoise::SmoothCubicCoherentNoise(unsigned int randFunc, double x, double y)
{
	int integer_X = (int)floor(x);
	double fractional_X = x - integer_X;

	int integer_Y = (int)floor(y);
	double fractional_Y = y - integer_Y;

	double v1 = SmoothNoise(randFunc, integer_X, integer_Y);
	double v2 = SmoothNoise(randFunc, integer_X + 1, integer_Y);
	double v3 = SmoothNoise(randFunc, integer_X, integer_Y + 1);
	double v4 = SmoothNoise(randFunc, integer_X + 1, integer_Y + 1);

	double i1 = Cosine_Interpolate(v1, v2, CubicSCurve(fractional_X));
	double i2 = Cosine_Interpolate(v3, v4, CubicSCurve(fractional_X));

	return Cosine_Interpolate(i1, i2, CubicSCurve(fractional_Y));
}

double AuNoise::SmoothQuinticCoherentNoise(unsigned int randFunc, double x)
{
	int integer_X = (int)floor(x);
	double fractional_X = x - integer_X;

	double v1 = SmoothNoise(randFunc, integer_X);
	double v2 = SmoothNoise(randFunc, integer_X + 1);

	return Cosine_Interpolate(v1, v2, QuinticSCurve(fractional_X));
}

double AuNoise::SmoothQuinticCoherentNoise(unsigned int randFunc, double x, double y)
{
	int integer_X = (int)floor(x);
	double fractional_X = x - integer_X;

	int integer_Y = (int)floor(y);
	double fractional_Y = y - integer_Y;

	double v1 = SmoothNoise(randFunc, integer_X, integer_Y);
	double v2 = SmoothNoise(randFunc, integer_X + 1, integer_Y);
	double v3 = SmoothNoise(randFunc, integer_X, integer_Y + 1);
	double v4 = SmoothNoise(randFunc, integer_X + 1, integer_Y + 1);

	double i1 = Cosine_Interpolate(v1, v2, QuinticSCurve(fractional_X));
	double i2 = Cosine_Interpolate(v3, v4, QuinticSCurve(fractional_X));

	return Cosine_Interpolate(i1, i2, QuinticSCurve(fractional_Y));
}

double AuNoise::SmoothCoherentNoise(unsigned int randFunc, double x)
{
	int integer_X = (int)floor(x);
	double fractional_X = x - integer_X;

	double v1 = SmoothNoise(randFunc, integer_X);
	double v2 = SmoothNoise(randFunc, integer_X + 1);

	return Cosine_Interpolate(v1, v2, fractional_X);
}

double AuNoise::SmoothCoherentNoise(unsigned int randFunc, double x, double y)
{
	int integer_X = (int)floor(x);
	double fractional_X = x - integer_X;

	int integer_Y = (int)floor(y);
	double fractional_Y = y - integer_Y;

	double v1 = SmoothNoise(randFunc, integer_X, integer_Y);
	double v2 = SmoothNoise(randFunc, integer_X + 1, integer_Y);
	double v3 = SmoothNoise(randFunc, integer_X, integer_Y + 1);
	double v4 = SmoothNoise(randFunc, integer_X + 1, integer_Y + 1);

	double i1 = Cosine_Interpolate(v1, v2, fractional_X);
	double i2 = Cosine_Interpolate(v3, v4, fractional_X);

	return Cosine_Interpolate(i1, i2, fractional_Y);
}

double AuNoise::PerlinNoise(unsigned int number_of_octaves, double persistence, double x)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	double total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		double frequency = pow(2., octave);
		double amplitude = pow(persistence, octave);
		total += CoherentNoise(octave, x * frequency) * amplitude;
	}
	return total;
}

double AuNoise::PerlinNoise(unsigned int number_of_octaves, double persistence, double x, double y)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	double total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		double frequency = pow(2., octave);
		double amplitude = pow(persistence, octave);
		total += CoherentNoise(octave, x * frequency, y * frequency) * amplitude;
	}
	return total;
}

// Returns the maximum value returned by the PerlinNoise function for a given 'persistence'.
double AuNoise::PerlinNoiseRange(unsigned int number_of_octaves, double persistence)
{
	double total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		double amplitude = pow(persistence, octave);
		total += amplitude;
	}
	return total;
}

double AuNoise::CubicPerlinNoise(unsigned int number_of_octaves, double persistence, double x)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	double total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		double frequency = pow(2., octave);
		double amplitude = pow(persistence, octave);
		total += CubicCoherentNoise(octave, x * frequency) * amplitude;
	}
	return total;
}

double AuNoise::CubicPerlinNoise(unsigned int number_of_octaves, double persistence, double x, double y)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	double total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		double frequency = pow(2., octave);
		double amplitude = pow(persistence, octave);
		total += CubicCoherentNoise(octave, x * frequency, y * frequency) * amplitude;
	}
	return total;
}

double AuNoise::QuinticPerlinNoise(unsigned int number_of_octaves, double persistence, double x)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	double total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		double frequency = pow(2., octave);
		double amplitude = pow(persistence, octave);
		total += QuinticCoherentNoise(octave, x * frequency) * amplitude;
	}
	return total;
}

double AuNoise::QuinticPerlinNoise(unsigned int number_of_octaves, double persistence, double x, double y)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	double total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		double frequency = pow(2., octave);
		double amplitude = pow(persistence, octave);
		total += QuinticCoherentNoise(octave, x * frequency, y * frequency) * amplitude;
	}
	return total;
}

double AuNoise::SmoothPerlinNoise(unsigned int number_of_octaves, double persistence, double x)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	double total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		double frequency = pow(2., octave);
		double amplitude = pow(persistence, octave);
		total += SmoothCoherentNoise(octave, x * frequency) * amplitude;
	}
	return total;
}

double AuNoise::SmoothPerlinNoise(unsigned int number_of_octaves, double persistence, double x, double y)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	double total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		double frequency = pow(2., octave);
		double amplitude = pow(persistence, octave);
		total += SmoothCoherentNoise(octave, x * frequency, y * frequency) * amplitude;
	}
	return total;
}

double AuNoise::SmoothCubicPerlinNoise(unsigned int number_of_octaves, double persistence, double x)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	double total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		double frequency = pow(2., octave);
		double amplitude = pow(persistence, octave);
		total += SmoothCubicCoherentNoise(octave, x * frequency) * amplitude;
	}
	return total;
}

double AuNoise::SmoothCubicPerlinNoise(unsigned int number_of_octaves, double persistence, double x, double y)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	double total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		double frequency = pow(2., octave);
		double amplitude = pow(persistence, octave);
		total += SmoothCubicCoherentNoise(octave, x * frequency, y * frequency) * amplitude;
	}
	return total;
}

double AuNoise::SmoothQuinticPerlinNoise(unsigned int number_of_octaves, double persistence, double x)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	double total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		double frequency = pow(2., octave);
		double amplitude = pow(persistence, octave);
		total += SmoothQuinticCoherentNoise(octave, x * frequency) * amplitude;
	}
	return total;
}

double AuNoise::SmoothQuinticPerlinNoise(unsigned int number_of_octaves, double persistence, double x, double y)
{
	Q_ASSERT(number_of_octaves > 0);
	Q_ASSERT(number_of_octaves <= sizeof(s_SmallPrime)/sizeof(int));
	double total = 0;
	for(int octave=0; octave<(int)number_of_octaves; octave++)
	{
		double frequency = pow(2., octave);
		double amplitude = pow(persistence, octave);
		total += SmoothQuinticCoherentNoise(octave, x * frequency, y * frequency) * amplitude;
	}
	return total;
}

//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
