#if !defined(FIXEDPNT_H)
#define FIXEDPNT_H

class TFixedPoint
{
public:
	TFixedPoint() : Value(0)
	{}
	TFixedPoint(double f) : Value(f)
	{}
	TFixedPoint(int s) : Value(s)
	{}
	TFixedPoint(int n, int d) : Value(double(n) * 1.0 / d)
	{}

	// Unary negation operator
	//
	TFixedPoint         operator -() {return -1 * Value;}

	// Postfix increment/decrement operators
	//
	void           operator ++(int) {Value += 1.0;}
	void           operator --(int) {Value -= 1.0;}

	// Bitwise logical operators
	//
	TFixedPoint    operator <<(unsigned n)
	{
		double v = Value;
		for (unsigned i = 0; i < n; i++)
		{
			v *= 2.0;
		}
		return v;
	}

	TFixedPoint    operator >>(unsigned n)
	{
		double v = Value;
		for (unsigned i = 0; i < n; i++)
		{
			v /= 2.0;
		}
		return v;
	}


	// Assignment operators
	//
	TFixedPoint& operator <<=(unsigned n);
	TFixedPoint& operator >>=(unsigned n);

	TFixedPoint& operator +=(const TFixedPoint& f) {Value += f.Value; return *this;}
	TFixedPoint& operator -=(const TFixedPoint& f) {Value -= f.Value; return *this;}
	TFixedPoint& operator *=(int s) 			   {Value *= s; return *this;}
	TFixedPoint& operator *=(const TFixedPoint& f) {Value = f.Value; return *this;}
	TFixedPoint& operator /=(int s) 			   {Value /= s; return *this;}
	TFixedPoint& operator /=(const TFixedPoint& f) {Value /= f.Value; return *this;}

	// Binary arithmetic operators
	//
	friend TFixedPoint operator +(const TFixedPoint& l, const TFixedPoint& r)
		{return l.Value + r.Value;}
	friend TFixedPoint operator +(int l, const TFixedPoint& r)
		{return TFixedPoint(l) += r.Value;}
	friend TFixedPoint operator +(const TFixedPoint& l, int r)
		{return r + l;}

	friend TFixedPoint operator -(const TFixedPoint& l, const TFixedPoint& r)
		{return l.Value - r.Value;}
	friend TFixedPoint operator -(int l, const TFixedPoint& r)
		{return TFixedPoint(l) -= r.Value;}
	friend TFixedPoint operator -(const TFixedPoint& l, int r)
		{return l - TFixedPoint(r);}

	friend TFixedPoint operator *(const TFixedPoint& l, const TFixedPoint& r)
		{return l.Value * r.Value;}
	friend TFixedPoint operator *(int l, const TFixedPoint& r)
		{return l * r.Value;}
	friend TFixedPoint operator *(const TFixedPoint& l, int r)
		{return l.Value * r;}

	friend TFixedPoint operator /(const TFixedPoint& l, const TFixedPoint& r)
		{return l.Value /r.Value;}
	friend TFixedPoint operator /(int l, const TFixedPoint& r)
		{return l / r.Value;}
	friend TFixedPoint operator /(const TFixedPoint& l, int r)
		{return l.Value / r;}

	// Equality operators
	//
/*
	friend bool   operator ==(const TFixedPoint& l,
							  const TFixedPoint& r) {return l.Value == r.Value;}
	friend bool   operator !=(const TFixedPoint& l,
							  const TFixedPoint& r) {return l.Value != r.Value;}
*/
	// Conversion operator to int
	//
	operator int()		{ return int(Value); }

private:
	TFixedPoint(long v) {Value = v;}
	double Value;
};

inline TFixedPoint&
TFixedPoint::operator <<=(unsigned n)
{
	TFixedPoint a = Value;
	a = a << n;
	Value = a;
	return *this;
}

inline TFixedPoint&
TFixedPoint::operator >>=(unsigned n)
{
	TFixedPoint a = Value;
	a = a >> n;
	Value = a;
	return *this;
}


#endif // FIXEDPNT_H
